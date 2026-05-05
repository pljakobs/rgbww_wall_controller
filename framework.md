# Lightinator Framework Overview

This document describes the current architecture of the UI/application stack in rgbww_wall_panel.

## Design Rules

1. Single-direction flow
- Services publish events.
- UiStateStore holds canonical UI-facing state.
- Presenters map state to visible widgets.
- Screens emit user intents through callbacks.

2. Separation of responsibilities
- app/application.cpp: composition root only.
- AppUi: LVGL root ownership + public UI API.
- AppNavigator: route transitions only.
- ScreenFactory: screen construction and callback injection only.
- Services (WiFi/hardware/persistence): no direct widget mutations.

3. UI thread discipline
- All LVGL mutations must run under UiRuntimeService::runOnUiThread().

4. Event-driven rendering
- No periodic full-state UI push loop.
- UiRuntimeService start/stop are lifecycle hooks; they do not drive periodic UI ticks.

## Class Relationship Map

```text
app/application.cpp
  |
  +-- HardwareInitService
  +-- UiRuntimeService
  +-- AppWIFI
  |     \
  |      +-- Controllers (mDNS registry)
  +-- WifiConfigFlow
  +-- NetworkUiBinder
  +-- AppUi
         |
         +-- UiStateStore
         +-- NetworkInfoPresenter
         +-- AppNavigator
                |
                +-- ScreenFactory
                       |
                       +-- MainScreen
                       +-- ColorPickerScreen
                       +-- NetworkInfoScreen
                       +-- WifiConfigScreen
                       +-- SettingsScreen
                       +-- TouchCalibrationScreen
                       +-- ThemeSelectorScreen
                       +-- ThemePreviewScreen (editor)
                       +-- MenuTestScreen (still routable, menu entry removed)
```

## Core Components

### app/application.cpp
- Composes all runtime modules.
- Loads theme and display settings from ConfigDB.
- Initializes hardware and UI.
- Wires app-level callbacks:
  - Theme save/list/apply
  - Settings load/save/brightness preview
  - Touch calibration save

### HardwareInitService
Files: app/HardwareInitService.h, app/HardwareInitService.cpp

- Hardware bring-up for panel, backlight, GT911.
- Explicit lifecycle via init and shutdown plus destructor cleanup.
- Owns touch callback bridge registration/de-registration.

### DisplaySettingsService
Files: app/DisplaySettingsService.h, app/DisplaySettingsService.cpp

- Loads persisted display settings.
- Saves brightness/timeout settings.
- Solves and persists 5-point touch calibration affine matrix.
- Applies calibration back to hardware service.

### UiRuntimeService
Files: app/UiRuntimeService.h, app/UiRuntimeService.cpp

- Central LVGL lock wrapper: runOnUiThread.
- start and stop are symmetric lifecycle hooks (currently no periodic worker).

### NetworkUiBinder
Files: app/NetworkUiBinder.h, app/NetworkUiBinder.cpp

- Registers all AppWIFI callbacks.
- Pushes network state to AppUi under UiRuntimeService lock.
- Debounces neighbour bursts via policy::kNetworkNeighboursDebounceMs.

### AppUi
Files: src/ui/AppUi.h, src/ui/AppUi.cpp

- Owns LVGL root and top-level UI API.
- Holds UiStateStore and NetworkInfoPresenter.
- Delegates routing to AppNavigator.
- Diff-aware setters:
  - setNetworkInfo
  - setNeighbours

### AppNavigator
Files: src/ui/AppNavigator.h, src/ui/AppNavigator.cpp

- Owns active screen instances.
- Handles route transitions and remounts.
- Uses resetScreenInstances to reduce duplication.
- Emits network screen visibility changes via setOnNetworkInfoScreenChanged.

### ScreenFactory
Files: src/ui/ScreenFactory.h, src/ui/ScreenFactory.cpp

- Constructs screens and injects callbacks.
- Seeds initial settings state where required (for SettingsScreen).
- Keeps creation logic decoupled from mount location.

### UiStateStore + NetworkInfoPresenter
Files: src/ui/UiStateStore.h, src/ui/UiStateStore.cpp and src/ui/NetworkInfoPresenter.h, src/ui/NetworkInfoPresenter.cpp

- UiStateStore is canonical state (no LVGL calls).
- Presenter binds/unbinds based on active screen visibility.
- Presenter pushes only while bound.

## Event Flows

### 1) Startup
1. application.cpp creates services and storage roots.
2. Theme and display settings are loaded.
3. HardwareInitService::init brings up display/touch/backlight.
4. UiRuntimeService::runOnUiThread initializes AppUi.
5. NetworkUiBinder registers callbacks and syncs initial state.
6. WifiConfigFlow optionally opens WiFi config screen.

### 2) Network state update
1. AppWIFI event fires (connected/disconnected/scan/neighbour change).
2. NetworkUiBinder receives callback.
3. UiRuntimeService::runOnUiThread updates AppUi.
4. AppUi updates UiStateStore.
5. NetworkInfoPresenter applies visible-screen updates when bound.

### 3) Settings save
1. User adjusts sliders in SettingsScreen.
2. Preview brightness callback is throttled in-screen.
3. OK triggers ScreenFactory-injected save callback.
4. DisplaySettingsService persists brightness/timeout and applies to hardware.
5. Screen closes via onCloseRequested.

### 4) Touch calibration save + close
1. User taps five target crosshairs.
2. TouchCalibrationScreen captures raw + reference points.
3. onSaveRequested triggers DisplaySettingsService matrix solve/persist/apply.
4. Screen defers close until release event to avoid click-through reopening Settings calibration button.
5. Navigator returns to SettingsScreen once.

## How To Add New Elements

## A) Add a new screen manually
1. Create screen class in src/ui/screens.
2. Add include and factory method in ScreenFactory.h/.cpp.
3. Add route method and pointer in AppNavigator.h/.cpp.
4. Wire trigger callback from a source screen (often MainScreen burger menu).
5. If stateful, add state fields in UiStateStore and presenter updates.
6. Ensure LVGL mutations happen in UI thread context.

## B) Add a menu entry to MainScreen
1. Add callback setter in MainScreen.h/.cpp.
2. Add event handler and button row in burger menu list.
3. Connect handler to AppNavigator route.
4. Keep scaffold marker comments if generator-managed.

## C) Add a persisted setting
1. Extend app-config.cfgdb or app-data.cfgdb schema.
2. Update DisplaySettingsService or theme helpers in application.cpp.
3. Add UI controls in SettingsScreen or appropriate screen.
4. Wire load/save callbacks through ScreenFactory -> AppUi -> application.cpp.

## Generator Script

Script: tools/ui_scaffold.py

Purpose:
- Scaffold new screen source/header files.
- Optionally patch ScreenFactory, AppNavigator, and MainScreen menu wiring.
- Track named menu hosts in tools/ui_menus.json.

Primary commands:

```bash
python3 tools/ui_scaffold.py create-menu main-left --host-screen MainScreen
```

```bash
python3 tools/ui_scaffold.py create-screen DeviceSettings \
  --flow close-ok-cancel \
  --wire-factory \
  --wire-navigator \
  --attach-menu main-left \
  --create-menu-if-missing
```

Useful options:
- --dry-run: print actions without changing files.
- --force: overwrite generated screen files if they already exist.
- --no-wire: generate files only.

Current limitations:
- Menu host wiring currently supports MainScreen menus only.
- create-widget and create-flow subcommands are placeholders.

## Build and Verification

Build:

```bash
make SMING_ARCH=Esp32
```

When debugging UI regressions:
- Verify route wiring in AppNavigator.
- Verify callback injection in ScreenFactory.
- Verify state changes in UiStateStore before presenter notifications.
- Verify no callback mutates LVGL outside UiRuntimeService::runOnUiThread.
