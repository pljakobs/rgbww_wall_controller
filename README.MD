# rgbww_wall_panel

LVGL-based wall panel firmware for ESP32-S3 (GUITION 4848S040) built with Sming.

## Current State

- UI is handwritten C++ (no EEZ-generated runtime wiring in the active path).
- Navigation and screen construction are split across AppUi, AppNavigator, and ScreenFactory.
- Network Info updates are event-driven and debounced.
- Display settings and touch calibration persistence are handled by DisplaySettingsService.
- Theme list/apply/save is backed by ConfigDB and AppData.

## Build

From project root:

```bash
make SMING_ARCH=Esp32
```

If you need a clean rebuild:

```bash
make clean SMING_ARCH=Esp32
make SMING_ARCH=Esp32
```

## Runtime Architecture

Top-level composition starts in app/application.cpp:

- Creates ConfigDB roots (AppConfig, AppData)
- Builds AppUi, HardwareInitService, UiRuntimeService, AppWIFI, WifiConfigFlow, NetworkUiBinder
- Loads persisted display settings and touch calibration
- Initializes hardware
- Initializes UI under UiRuntimeService lock discipline
- Binds WiFi callbacks and synchronizes initial state to UI

Detailed class relationships, event flows, and extension patterns are documented in framework.md.

## Where Things Live

- App bootstrap and wiring: app/application.cpp
- Hardware lifecycle: app/HardwareInitService.h, app/HardwareInitService.cpp
- UI thread gateway: app/UiRuntimeService.h, app/UiRuntimeService.cpp
- Display settings + calibration persistence: app/DisplaySettingsService.h, app/DisplaySettingsService.cpp
- WiFi -> UI event binding: app/NetworkUiBinder.h, app/NetworkUiBinder.cpp
- UI root facade: src/ui/AppUi.h, src/ui/AppUi.cpp
- Navigation: src/ui/AppNavigator.h, src/ui/AppNavigator.cpp
- Screen assembly: src/ui/ScreenFactory.h, src/ui/ScreenFactory.cpp
- Canonical UI state store: src/ui/UiStateStore.h, src/ui/UiStateStore.cpp
- Network presenter: src/ui/NetworkInfoPresenter.h, src/ui/NetworkInfoPresenter.cpp
- Screen implementations: src/ui/screens/
- UI scaffold generator: tools/ui_scaffold.py
- Menu registry for generator: tools/ui_menus.json

## Adding UI Elements

Two supported paths:

1. Manual path (recommended for non-trivial screens)
- Add/modify screen class in src/ui/screens
- Wire callbacks in ScreenFactory
- Add route transition in AppNavigator
- Expose trigger from existing screen (often MainScreen)

2. Generator-assisted path (fast scaffold)
- Use tools/ui_scaffold.py to generate a screen and optional wiring stubs
- Review and adjust generated code before commit

See framework.md for concrete commands and step-by-step examples.

## Notes

- Repository includes external components under Components; do not update vendor docs when changing app logic.
- There may be unrelated dirty submodule pointers in local workspace state; keep commits focused on app files.
