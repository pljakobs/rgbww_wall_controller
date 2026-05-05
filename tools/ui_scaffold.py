#!/usr/bin/env python3
"""UI scaffold generator for rgbww_wall_panel.

Creates LVGL screen classes and optionally wires them into ScreenFactory,
AppNavigator, and MainScreen burger menu.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass
from pathlib import Path


VALID_FLOWS = {"none", "close", "ok-cancel", "close-ok-cancel", "wizard"}
DEFAULT_MENU_REGISTRY = "tools/ui_menus.json"


@dataclass
class ScreenSpec:
    base_name: str
    class_name: str
    method_stem: str
    file_stem: str
    menu_label: str
    flow: str


@dataclass
class MenuSpec:
    name: str
    host_screen: str


class PatchError(RuntimeError):
    pass


def to_pascal_case(name: str) -> str:
    parts = re.split(r"[^A-Za-z0-9]+", name.strip())
    parts = [p for p in parts if p]
    if not parts:
        raise ValueError("Name must contain alphanumeric characters")
    return "".join(p[0].upper() + p[1:] for p in parts)


def normalize_spec(raw_name: str, flow: str, menu_label: str | None) -> ScreenSpec:
    pascal = to_pascal_case(raw_name)
    if pascal.endswith("Screen"):
        base = pascal[:-6]
        class_name = pascal
    else:
        base = pascal
        class_name = f"{pascal}Screen"

    if not base:
        raise ValueError("Screen name cannot be only 'Screen'")

    file_stem = f"{base}Screen"
    method_stem = f"{base}Screen"
    label = menu_label if menu_label else base
    if flow not in VALID_FLOWS:
        raise ValueError(f"Invalid flow '{flow}'")

    return ScreenSpec(
        base_name=base,
        class_name=class_name,
        method_stem=method_stem,
        file_stem=file_stem,
        menu_label=label,
        flow=flow,
    )


def write_file(path: Path, content: str, force: bool) -> None:
    if path.exists() and not force:
        raise PatchError(f"File already exists: {path}")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def load_menu_registry(path: Path) -> dict[str, dict[str, str]]:
    if not path.exists():
        return {}
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as err:
        raise PatchError(f"Invalid menu registry JSON in {path}: {err}") from err
    if not isinstance(payload, dict):
        raise PatchError(f"Invalid menu registry format in {path}: expected object")
    return payload


def save_menu_registry(path: Path, registry: dict[str, dict[str, str]], dry_run: bool) -> None:
    text = json.dumps(registry, indent=2, sort_keys=True) + "\n"
    if dry_run:
        print(f"[dry-run] would update {path}")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def ensure_menu_entry(
    registry: dict[str, dict[str, str]],
    menu_name: str,
    host_screen: str,
    create_if_missing: bool,
) -> MenuSpec:
    entry = registry.get(menu_name)
    if entry is None:
        if not create_if_missing:
            raise PatchError(
                f"Menu '{menu_name}' is not in registry. Use --create-menu-if-missing or run create-menu first."
            )
        entry = {"host_screen": host_screen}
        registry[menu_name] = entry
        return MenuSpec(name=menu_name, host_screen=host_screen)

    found_screen = entry.get("host_screen")
    if not isinstance(found_screen, str) or not found_screen:
        raise PatchError(f"Menu '{menu_name}' has invalid host_screen in registry")
    return MenuSpec(name=menu_name, host_screen=found_screen)


def ensure_contains_once(content: str, needle: str, snippet: str) -> str:
    if needle in content:
        return content
    return content + snippet


def insert_after(content: str, anchor: str, snippet: str) -> str:
    idx = content.find(anchor)
    if idx < 0:
        raise PatchError(f"Anchor not found: {anchor}")
    insert_pos = idx + len(anchor)
    return content[:insert_pos] + snippet + content[insert_pos:]


def insert_before(content: str, anchor: str, snippet: str) -> str:
    idx = content.find(anchor)
    if idx < 0:
        raise PatchError(f"Anchor not found: {anchor}")
    return content[:idx] + snippet + content[idx:]


def insert_after_regex(content: str, pattern: str, snippet: str) -> str:
    match = re.search(pattern, content, flags=re.MULTILINE)
    if not match:
        raise PatchError(f"Anchor regex not found: {pattern}")
    pos = match.end()
    return content[:pos] + snippet + content[pos:]


def generate_header(spec: ScreenSpec) -> str:
    has_close = spec.flow in {"close", "close-ok-cancel", "wizard"}
    has_ok_cancel = spec.flow in {"ok-cancel", "close-ok-cancel", "wizard"}

    close_methods = ""
    close_member = ""
    if has_close:
        close_methods = "\n    void setOnCloseRequested(std::function<void()> callback);\n"
        close_member = "\n    std::function<void()> onCloseRequested_;\n"

    ok_cancel_methods = ""
    ok_cancel_members = ""
    ok_cancel_handlers = ""
    if has_ok_cancel:
        ok_cancel_methods = (
            "\n    void setOnOkRequested(std::function<void()> callback);\n"
            "    void setOnCancelRequested(std::function<void()> callback);\n"
        )
        ok_cancel_handlers = (
            "\n    static void onOkButtonEvent(lv_event_t* event);\n"
            "    static void onCancelButtonEvent(lv_event_t* event);\n"
        )
        ok_cancel_members = (
            "\n    std::function<void()> onOkRequested_;\n"
            "    std::function<void()> onCancelRequested_;\n"
            "    lv_obj_t* footer_ = nullptr;\n"
        )

    return f"""#pragma once

#include <functional>
#include <memory>

#include <lvgl.h>

#include "ui/core/Screen.h"
#include "ui/core/UiTheme.h"
#include "ui/screens/DecoratedScreen.h"

namespace lightinator::ui::screens {{

class {spec.class_name} : public core::Screen {{
public:
    explicit {spec.class_name}(const core::UiTheme& theme);

    void mount(lv_obj_t* parent) override;{close_methods}{ok_cancel_methods}
private:{ok_cancel_handlers}
    core::UiTheme theme_;{close_member}{ok_cancel_members}
    std::unique_ptr<DecoratedScreen> decorated_;
}};

}} // namespace lightinator::ui::screens
"""


def generate_cpp(spec: ScreenSpec) -> str:
    has_close = spec.flow in {"close", "close-ok-cancel", "wizard"}
    has_ok_cancel = spec.flow in {"ok-cancel", "close-ok-cancel", "wizard"}

    show_close = "true" if has_close else "false"

    close_setter = ""
    close_hook = ""
    if has_close:
        close_setter = f"""
void {spec.class_name}::setOnCloseRequested(std::function<void()> callback)
{{
    onCloseRequested_ = std::move(callback);
}}
"""
        close_hook = """
    decorated_->setOnCloseRequested([this]() {
        if (onCloseRequested_) {
            onCloseRequested_();
        }
    });
"""

    ok_cancel_setters = ""
    ok_cancel_ui = ""
    ok_cancel_handlers = ""
    if has_ok_cancel:
        ok_cancel_setters = f"""
void {spec.class_name}::setOnOkRequested(std::function<void()> callback)
{{
    onOkRequested_ = std::move(callback);
}}

void {spec.class_name}::setOnCancelRequested(std::function<void()> callback)
{{
    onCancelRequested_ = std::move(callback);
}}
"""
        ok_cancel_ui = """
    footer_ = lv_obj_create(body);
    lv_obj_set_width(footer_, lv_pct(100));
    lv_obj_set_height(footer_, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(footer_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(footer_, 0, 0);
    lv_obj_set_style_pad_all(footer_, 0, 0);
    lv_obj_set_style_pad_column(footer_, 10, 0);
    lv_obj_set_flex_flow(footer_, LV_FLEX_FLOW_ROW);

    lv_obj_t* okButton = lv_btn_create(footer_);
    lv_obj_set_size(okButton, 120, 44);
    lv_obj_set_style_bg_color(okButton, theme_.colors.buttonBg, 0);
    lv_obj_set_style_text_color(okButton, theme_.colors.buttonFg, 0);
    lv_obj_set_style_border_width(okButton, 0, 0);
    lv_obj_add_event_cb(okButton, onOkButtonEvent, LV_EVENT_CLICKED, this);
    lv_obj_t* okLabel = lv_label_create(okButton);
    lv_label_set_text_static(okLabel, "OK");
    lv_obj_set_style_text_font(okLabel, theme_.fonts.contentSubheader, 0);
    lv_obj_center(okLabel);

    lv_obj_t* cancelButton = lv_btn_create(footer_);
    lv_obj_set_size(cancelButton, 120, 44);
    lv_obj_set_style_bg_color(cancelButton, theme_.colors.buttonBg, 0);
    lv_obj_set_style_text_color(cancelButton, theme_.colors.buttonFg, 0);
    lv_obj_set_style_border_width(cancelButton, 0, 0);
    lv_obj_add_event_cb(cancelButton, onCancelButtonEvent, LV_EVENT_CLICKED, this);
    lv_obj_t* cancelLabel = lv_label_create(cancelButton);
    lv_label_set_text_static(cancelLabel, "Cancel");
    lv_obj_set_style_text_font(cancelLabel, theme_.fonts.contentSubheader, 0);
    lv_obj_center(cancelLabel);
"""
        ok_cancel_handlers = f"""
void {spec.class_name}::onOkButtonEvent(lv_event_t* event)
{{
    auto* self = static_cast<{spec.class_name}*>(lv_event_get_user_data(event));
    if (self == nullptr || !self->onOkRequested_) {{
        return;
    }}
    self->onOkRequested_();
}}

void {spec.class_name}::onCancelButtonEvent(lv_event_t* event)
{{
    auto* self = static_cast<{spec.class_name}*>(lv_event_get_user_data(event));
    if (self == nullptr) {{
        return;
    }}
    if (self->onCancelRequested_) {{
        self->onCancelRequested_();
        return;
    }}
    if (self->onCloseRequested_) {{
        self->onCloseRequested_();
    }}
}}
"""

    return f"""#include "ui/screens/{spec.file_stem}.h"

namespace lightinator::ui::screens {{

{spec.class_name}::{spec.class_name}(const core::UiTheme& theme)
    : theme_(theme)
{{
}}
{close_setter}{ok_cancel_setters}
void {spec.class_name}::mount(lv_obj_t* parent)
{{
    HeaderOptions header = {{}};
    header.text = "{spec.base_name}";
    header.showClose = {show_close};
    header.height = theme_.headerHeight;
    header.color = theme_.colors.headerBg;
    header.font = theme_.fonts.header;

    decorated_ = std::make_unique<DecoratedScreen>(header);{close_hook}
    decorated_->mount(parent);
    setRoot(decorated_->root());

    lv_obj_t* body = decorated_->bodySlot();
    lv_obj_set_style_bg_color(body, theme_.colors.contentBg, 0);
    lv_obj_set_style_bg_opa(body, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(body, 12, 0);
    lv_obj_set_style_pad_row(body, 10, 0);
    lv_obj_set_flex_flow(body, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(body, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t* title = lv_label_create(body);
    lv_label_set_text_static(title, "TODO: implement screen content");
    lv_obj_set_style_text_font(title, theme_.fonts.contentSubheader, 0);
    lv_obj_set_style_text_color(title, theme_.colors.contentFg, 0);
{ok_cancel_ui}}}
{ok_cancel_handlers}
}} // namespace lightinator::ui::screens
"""


def apply_or_print(path: Path, content: str, dry_run: bool) -> None:
    if dry_run:
        print(f"[dry-run] would update {path}")
        return
    path.write_text(content, encoding="utf-8")


def patch_screen_factory(root: Path, spec: ScreenSpec, dry_run: bool) -> None:
    h_path = root / "src/ui/ScreenFactory.h"
    cpp_path = root / "src/ui/ScreenFactory.cpp"

    h = h_path.read_text(encoding="utf-8")
    cpp = cpp_path.read_text(encoding="utf-8")

    include_line = f'#include "ui/screens/{spec.file_stem}.h"\n'
    if include_line not in h:
        anchor = '#include "ui/screens/WifiConfigScreen.h"\n'
        h = insert_after(h, anchor, include_line)

    decl = (
        f"\n    std::unique_ptr<screens::{spec.class_name}> create{spec.method_stem}(\n"
        "        std::function<void()> onClose);\n"
    )
    if f"create{spec.method_stem}(" not in h:
        anchor = "    std::unique_ptr<screens::ThemePreviewScreen> createThemePreviewScreen(\n        std::function<void()> onClose);\n"
        h = insert_after(h, anchor, decl)

    impl = f"""
std::unique_ptr<screens::{spec.class_name}> ScreenFactory::create{spec.method_stem}(
    std::function<void()> onClose)
{{
    auto screen = std::make_unique<screens::{spec.class_name}>(theme_);
    screen->setOnCloseRequested(std::move(onClose));
    return screen;
}}
"""
    if f"ScreenFactory::create{spec.method_stem}(" not in cpp:
        insert_anchor = "std::unique_ptr<screens::ThemePreviewScreen> ScreenFactory::createThemePreviewScreen(\n"
        idx = cpp.find(insert_anchor)
        if idx < 0:
            raise PatchError("Could not find ThemePreviewScreen factory implementation")
        cpp = cpp[:idx] + impl + cpp[idx:]

    apply_or_print(h_path, h, dry_run)
    apply_or_print(cpp_path, cpp, dry_run)


def patch_app_navigator(root: Path, spec: ScreenSpec, dry_run: bool, wire_main_menu: bool) -> None:
    h_path = root / "src/ui/AppNavigator.h"
    cpp_path = root / "src/ui/AppNavigator.cpp"

    h = h_path.read_text(encoding="utf-8")
    cpp = cpp_path.read_text(encoding="utf-8")

    include_line = f'#include "ui/screens/{spec.file_stem}.h"\n'
    if include_line not in h:
        anchor = '#include "ui/screens/WifiConfigScreen.h"\n'
        h = insert_after(h, anchor, include_line)

    method_decl = f"    void show{spec.method_stem}();\n"
    if method_decl not in h:
        h = insert_after(h, "    void showThemePreviewScreen();\n", method_decl)

    member_decl = f"    std::unique_ptr<screens::{spec.class_name}>   {spec.base_name[0].lower() + spec.base_name[1:]}Screen_;\n"
    if member_decl not in h:
        h = insert_after(
            h,
            "    std::unique_ptr<screens::ThemePreviewScreen>   themePreviewScreen_;\n",
            member_decl,
        )

    var_name = f"{spec.base_name[0].lower() + spec.base_name[1:]}Screen_"

    show_method = f"""
void AppNavigator::show{spec.method_stem}()
{{
    clearRoot();
    mainScreen_.reset();
    colorPickerScreen_.reset();
    wifiConfigScreen_.reset();
    networkInfoScreen_.reset();
    themePreviewScreen_.reset();
    {var_name}.reset();
    {var_name} = factory_.create{spec.method_stem}(
        [this]() {{ showMainScreen(); }});
    {var_name}->mount(root_);
}}
"""

    if f"void AppNavigator::show{spec.method_stem}()" not in cpp:
        cpp = insert_before(cpp, "screens::WifiConfigScreen* AppNavigator::wifiConfigScreen()", show_method)

    reset_line = f"    {var_name}.reset();\n"
    for func in [
        "void AppNavigator::showMainScreen()",
        "void AppNavigator::showColorPickerScreen()",
        "void AppNavigator::showNetworkInfoScreen()",
        "void AppNavigator::showWifiConfigScreen()",
        "void AppNavigator::showThemePreviewScreen()",
    ]:
        start = cpp.find(func)
        if start < 0:
            raise PatchError(f"Function not found in AppNavigator.cpp: {func}")
        block_end = cpp.find("}\n", start)
        if block_end < 0:
            raise PatchError(f"Cannot find end of function block: {func}")
        block = cpp[start:block_end]
        if reset_line not in block:
            if "themePreviewScreen_.reset();\n" in block:
                new_block = block.replace("themePreviewScreen_.reset();\n", "themePreviewScreen_.reset();\n" + reset_line)
                cpp = cpp[:start] + new_block + cpp[block_end:]
            elif "networkInfoScreen_.reset();\n" in block:
                new_block = block.replace("networkInfoScreen_.reset();\n", "networkInfoScreen_.reset();\n" + reset_line)
                cpp = cpp[:start] + new_block + cpp[block_end:]

    if wire_main_menu:
        setter_call = f"    mainScreen_->setOnOpen{spec.base_name}Requested([this]() {{ show{spec.method_stem}(); }});\n"
        if setter_call not in cpp:
            anchor = "    mainScreen_->mount(root_);\n"
            cpp = insert_before(cpp, anchor, setter_call)

    apply_or_print(h_path, h, dry_run)
    apply_or_print(cpp_path, cpp, dry_run)


def patch_main_screen(root: Path, spec: ScreenSpec, dry_run: bool, menu_name: str) -> None:
    h_path = root / "src/ui/screens/MainScreen.h"
    cpp_path = root / "src/ui/screens/MainScreen.cpp"

    h = h_path.read_text(encoding="utf-8")
    cpp = cpp_path.read_text(encoding="utf-8")

    setter_decl = f"    void setOnOpen{spec.base_name}Requested(std::function<void()> callback);\n"
    if setter_decl not in h:
        h = insert_after(h, "    void setOnOpenThemePreviewRequested(std::function<void()> callback);\n", setter_decl)

    handler_decl = f"    static void onMenu{spec.base_name}Event(lv_event_t* event);\n"
    if handler_decl not in h:
        h = insert_after(h, "    static void onMenuNetworkInfoEvent(lv_event_t* event);\n", handler_decl)

    callback_member = f"    std::function<void()> onOpen{spec.base_name}Requested_;\n"
    if callback_member not in h:
        h = insert_after(h, "    std::function<void()> onOpenThemePreviewRequested_;\n", callback_member)

    setter_impl = f"""
void MainScreen::setOnOpen{spec.base_name}Requested(std::function<void()> callback)
{{
    onOpen{spec.base_name}Requested_ = std::move(callback);
}}
"""
    if f"void MainScreen::setOnOpen{spec.base_name}Requested(" not in cpp:
        cpp = insert_after(cpp, "void MainScreen::setOnOpenThemePreviewRequested(std::function<void()> callback)\n{\n    onOpenThemePreviewRequested_ = std::move(callback);\n}\n", setter_impl)

    handler_impl = f"""
void MainScreen::onMenu{spec.base_name}Event(lv_event_t* event)
{{
    auto* self = static_cast<MainScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {{
        return;
    }}
    self->hideBurgerMenu();
    if (self->onOpen{spec.base_name}Requested_) {{
        self->onOpen{spec.base_name}Requested_();
    }}
}}
"""
    if f"void MainScreen::onMenu{spec.base_name}Event(" not in cpp:
        cpp = insert_after_regex(
            cpp,
            r"void MainScreen::onMenuNetworkInfoEvent\(lv_event_t\* event\)\n\{[\s\S]*?\n\}\n",
            handler_impl,
        )

    marker = f"// ui-scaffold:{menu_name}:{spec.base_name}:menu-item"
    if marker not in cpp:
        menu_block = f"""
    {marker}
    lv_obj_t* generatedItem = lv_list_add_btn(list, nullptr, "{spec.menu_label}");
    styleListButton(generatedItem);
    lv_obj_add_event_cb(generatedItem, onMenu{spec.base_name}Event, LV_EVENT_CLICKED, this);
"""
        cpp = insert_after_regex(
            cpp,
            r"\n\s*lv_obj_t\*\s+themeItem\s*=\s*lv_list_add_btn\(list,\s*nullptr,\s*\"Theme View\"\);\n\s*styleListButton\(themeItem\);\n\s*lv_obj_add_event_cb\(themeItem,\s*onMenuThemeEvent,\s*LV_EVENT_CLICKED,\s*this\);\n",
            menu_block,
        )

    apply_or_print(h_path, h, dry_run)
    apply_or_print(cpp_path, cpp, dry_run)


def scaffold_screen_files(root: Path, spec: ScreenSpec, dry_run: bool, force: bool) -> None:
    h_path = root / "src/ui/screens" / f"{spec.file_stem}.h"
    cpp_path = root / "src/ui/screens" / f"{spec.file_stem}.cpp"

    header = generate_header(spec)
    impl = generate_cpp(spec)

    if dry_run:
        print(f"[dry-run] would create {h_path}")
        print(f"[dry-run] would create {cpp_path}")
        return

    write_file(h_path, header, force=force)
    write_file(cpp_path, impl, force=force)


def patch_named_menu(root: Path, spec: ScreenSpec, menu: MenuSpec, dry_run: bool) -> None:
    if menu.host_screen != "MainScreen":
        raise PatchError(
            f"Menu host screen '{menu.host_screen}' is not supported yet. "
            "Current generator supports only MainScreen menus."
        )
    patch_main_screen(root, spec, dry_run=dry_run, menu_name=menu.name)


def cmd_create_menu(args: argparse.Namespace) -> int:
    root = Path(args.project_root).resolve()
    registry_path = root / args.menu_registry
    registry = load_menu_registry(registry_path)

    if args.name in registry and not args.force:
        raise PatchError(f"Menu '{args.name}' already exists. Use --force to replace it.")

    registry[args.name] = {
        "host_screen": args.host_screen,
    }
    save_menu_registry(registry_path, registry, dry_run=args.dry_run)

    print(f"Registered menu '{args.name}' on host screen '{args.host_screen}'")
    if args.dry_run:
        print("Dry-run mode: no files were changed")
    return 0


def cmd_create_screen(args: argparse.Namespace) -> int:
    root = Path(args.project_root).resolve()
    spec = normalize_spec(args.name, args.flow, args.menu_label)
    registry_path = root / args.menu_registry
    registry = load_menu_registry(registry_path)

    wire_factory = args.wire_factory
    wire_navigator = args.wire_navigator
    wire_main_menu = args.wire_main_menu
    menu_spec: MenuSpec | None = None

    if args.attach_menu and args.menu_host_screen:
        raise PatchError("Use either --attach-menu or --menu-host-screen, not both")

    if args.attach_menu:
        menu_spec = ensure_menu_entry(
            registry,
            menu_name=args.attach_menu,
            host_screen=args.default_menu_host,
            create_if_missing=args.create_menu_if_missing,
        )
        wire_main_menu = True
    elif args.menu_host_screen:
        menu_name = f"auto:{args.menu_host_screen}"
        menu_spec = ensure_menu_entry(
            registry,
            menu_name=menu_name,
            host_screen=args.menu_host_screen,
            create_if_missing=True,
        )
        wire_main_menu = True

    if args.no_wire:
        wire_factory = False
        wire_navigator = False
        wire_main_menu = False

    scaffold_screen_files(root, spec, dry_run=args.dry_run, force=args.force)

    if wire_factory:
        patch_screen_factory(root, spec, dry_run=args.dry_run)

    if wire_navigator:
        patch_app_navigator(root, spec, dry_run=args.dry_run, wire_main_menu=wire_main_menu)

    if wire_main_menu:
        if menu_spec is None:
            menu_spec = MenuSpec(name=f"auto:{args.default_menu_host}", host_screen=args.default_menu_host)
            registry.setdefault(menu_spec.name, {"host_screen": menu_spec.host_screen})
        patch_named_menu(root, spec, menu=menu_spec, dry_run=args.dry_run)

    if args.attach_menu or args.menu_host_screen or wire_main_menu:
        save_menu_registry(registry_path, registry, dry_run=args.dry_run)

    print(f"Scaffolded {spec.class_name} with flow '{spec.flow}'")
    if menu_spec:
        print(
            f"Wiring: factory={wire_factory} navigator={wire_navigator} "
            f"main-menu={wire_main_menu} menu={menu_spec.name}@{menu_spec.host_screen}"
        )
    else:
        print(f"Wiring: factory={wire_factory} navigator={wire_navigator} main-menu={wire_main_menu}")
    if args.dry_run:
        print("Dry-run mode: no files were changed")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Generate UI screens and wiring stubs")
    parser.add_argument(
        "--project-root",
        default=str(Path(__file__).resolve().parents[1]),
        help="Project root directory",
    )
    parser.add_argument(
        "--menu-registry",
        default=DEFAULT_MENU_REGISTRY,
        help="Relative path to menu registry JSON",
    )

    sub = parser.add_subparsers(dest="command", required=True)

    create_screen = sub.add_parser("create-screen", help="Create a new screen scaffold")
    create_screen.add_argument("name", help="Screen name, e.g. DeviceSettings or DeviceSettingsScreen")
    create_screen.add_argument(
        "--flow",
        choices=sorted(VALID_FLOWS),
        default="close",
        help="Standard interaction flow preset",
    )
    create_screen.add_argument("--menu-label", help="Main menu label when --wire-main-menu is used")
    create_screen.add_argument("--wire-factory", action="store_true", help="Patch ScreenFactory")
    create_screen.add_argument("--wire-navigator", action="store_true", help="Patch AppNavigator")
    create_screen.add_argument("--wire-main-menu", action="store_true", help="Patch MainScreen burger menu")
    create_screen.add_argument("--attach-menu", help="Attach menu item to a named menu in registry")
    create_screen.add_argument("--menu-host-screen", help="Attach menu item by host screen name")
    create_screen.add_argument(
        "--default-menu-host",
        default="MainScreen",
        help="Host screen used when creating a missing named menu",
    )
    create_screen.add_argument(
        "--create-menu-if-missing",
        action="store_true",
        help="Create named menu in registry when --attach-menu does not exist",
    )
    create_screen.add_argument("--no-wire", action="store_true", help="Disable all wiring")
    create_screen.add_argument("--dry-run", action="store_true", help="Show actions without writing files")
    create_screen.add_argument("--force", action="store_true", help="Overwrite generated screen files if they exist")
    create_screen.set_defaults(handler=cmd_create_screen)

    create_menu = sub.add_parser("create-menu", help="Create or update a named menu in registry")
    create_menu.add_argument("name", help="Menu name, e.g. main-left")
    create_menu.add_argument(
        "--host-screen",
        default="MainScreen",
        help="Screen class this menu is attached to",
    )
    create_menu.add_argument("--dry-run", action="store_true", help="Show actions without writing files")
    create_menu.add_argument("--force", action="store_true", help="Replace existing menu definition")
    create_menu.set_defaults(handler=cmd_create_menu)

    create_widget = sub.add_parser("create-widget", help="Reserved for future widget generator")
    create_widget.set_defaults(handler=lambda _args: _print_not_implemented("create-widget"))

    create_flow = sub.add_parser("create-flow", help="Reserved for future flow generator")
    create_flow.set_defaults(handler=lambda _args: _print_not_implemented("create-flow"))

    return parser


def _print_not_implemented(command: str) -> int:
    print(f"{command} is not implemented yet")
    return 2


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        return int(args.handler(args))
    except (PatchError, ValueError) as err:
        print(f"error: {err}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
