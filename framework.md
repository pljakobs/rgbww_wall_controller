# Lightinator UI Framework

This document describes the architecture and components of the UI/application framework that has emerged from the `rgbww_wall_panel` project. The framework targets ESP32-S3 with LVGL v8 running under the Sming RTOS environment.

---

## Core Principles

1. **Single-direction data flow** — Services publish state changes → `UiStateStore` holds canonical UI state → Presenters map state to widgets → Screens emit user intents only.
2. **Hard separation of responsibilities** — Each component has one job. No LVGL calls outside UI-layer code; no networking callbacks touching widgets directly.
3. **UI-thread discipline** — All LVGL mutations go through `UiRuntimeService::runOnUiThread()`. Hardware and networking callbacks never call widget APIs directly.
4. **Event-driven updates** — No unconditional periodic full-state pushes. Updates happen only on meaningful state changes.
5. **Diff before render** — `UiStateStore` tracks last committed state; setters return `bool changed`. Presenters skip rendering if nothing actually changed.
6. **Visible-screen gating** — Presenters bind to a screen when it becomes visible and unbind when it is hidden. State accumulates in the store while unbound; the screen is seeded from the store on next bind.
7. **Lifecycle clarity** — Startup wiring belongs in the composition root (`application.cpp` + `AppBootstrap`). Feature logic belongs in services and presenters.

---

## Layer Map

```
application.cpp          ← composition root (no business logic)
    │
    ├── HardwareInitService    ← backlight / display / touch bring-up
    ├── UiRuntimeService       ← LVGL lock + tick timer
    ├── AppWIFI                ← WiFi / mDNS service
    │       └── Controllers    ← mDNS neighbour registry (fires onChange callback)
    ├── WifiConfigFlow         ← WiFi setup flow coordinator
    ├── NetworkUiBinder        ← wires AppWIFI events → AppUi calls (debounced)
    └── AppUi                  ← LVGL root ownership + public state API
            ├── UiStateStore       ← canonical UI state (no LVGL)
            ├── NetworkInfoPresenter ← binds UiStateStore ↔ NetworkInfoScreen
            └── AppNavigator       ← screen transitions
                    └── ScreenFactory  ← allocates + wires screen instances
```

---

## Components

### `HardwareInitService`
**File:** `app/HardwareInitService.h/.cpp`  
**Namespace:** `lightinator`

Encapsulates hardware bring-up for the Guition ESP32-4848S040 panel: backlight GPIO, `esp_lcd` RGB panel adapter, and GT911 touch driver.

```cpp
HardwareInitService hw;
HardwareCapabilities caps = hw.init();
if (!caps.displayOk) { /* abort */ }
```

`HardwareCapabilities` fields: `displayOk`, `touchOk`.

---

### `UiRuntimeService`
**File:** `app/UiRuntimeService.h/.cpp`  
**Namespace:** `lightinator`

Owns the LVGL tick timer (180 ms) and all `lvgl_runtime_adapter_lock/unlock` calls.

```cpp
UiRuntimeService runtime;

// Execute any LVGL mutation safely:
runtime.runOnUiThread([&]() { ui.init(); });

// Start the tick loop (call after LVGL init):
runtime.start(ui);
```

**Rule:** Never call `lvgl_runtime_adapter_lock/unlock` directly anywhere else. All LVGL mutations must pass through `runOnUiThread`.

---

### `AppUi`
**File:** `src/ui/AppUi.h/.cpp`  
**Namespace:** `lightinator::ui`

LVGL root ownership and the public state API surface.  
Owns `UiStateStore`, `NetworkInfoPresenter`, and `AppNavigator`.

```cpp
AppUi ui;
ui.init();                              // must run under LVGL lock
ui.setNetworkInfo(true, ip, nm, gw);   // diffs, skips render if unchanged
ui.setNeighbours(neighbourList);        // diffs, skips render if unchanged
ui.tickAnimation();                     // called by UiRuntimeService each tick
ui.showWifiConfigScreen();
ui.closeWifiConfigScreen();
```

---

### `UiStateStore`
**File:** `src/ui/UiStateStore.h/.cpp`  
**Namespace:** `lightinator::ui`

Plain C++ store — zero LVGL calls. Holds the last committed UI-facing state. All setters diff against stored state and return `true` only when the value actually changed.

```cpp
UiStateStore store;
bool changed = store.setNetworkInfo(connected, ip, nm, gw);
bool changed = store.setNeighbours(neighbours);

// Read:
store.wifiConnected();
store.ipAddress();
store.neighbours();   // const std::vector<Neighbour>&
```

---

### `AppNavigator`
**File:** `src/ui/AppNavigator.h/.cpp`  
**Namespace:** `lightinator::ui`

Screen transition logic. Owns all active screen pointers and delegates construction to `ScreenFactory`. Destroys the previous screen before mounting the next via `clearRoot()`.

```cpp
// Constructed by AppUi:
AppNavigator nav(root, state, currentColor);

nav.showMainScreen();
nav.showNetworkInfoScreen();
nav.showColorPickerScreen();
nav.showWifiConfigScreen();
nav.closeWifiConfigScreen();

// Presenter lifecycle hook:
nav.setOnNetworkInfoScreenChanged([](screens::NetworkInfoScreen* s) {
    presenter.bind(s);   // s == nullptr when screen is hidden
});
```

---

### `ScreenFactory`
**File:** `src/ui/ScreenFactory.h/.cpp`  
**Namespace:** `lightinator::ui`

Allocates screen instances and injects all callbacks. Returns fully-wired `unique_ptr`. Mounting is the caller's responsibility (separation of construction from mount-point).

```cpp
ScreenFactory factory(state, currentColor);

auto main = factory.createMainScreen(
    []{ nav.showColorPickerScreen(); },
    []{ nav.showNetworkInfoScreen(); });

auto net  = factory.createNetworkInfoScreen([]{ nav.showMainScreen(); });
auto cfg  = factory.createWifiConfigScreen();
auto cp   = factory.createColorPickerScreen([]{ nav.showMainScreen(); });
```

**Rule:** `ScreenFactory` does not seed state into screens — that is the presenter's job (`bind()` seeds from `UiStateStore` on first attach).

---

### `NetworkInfoPresenter`
**File:** `src/ui/NetworkInfoPresenter.h/.cpp`  
**Namespace:** `lightinator::ui`

Binds `UiStateStore` state to `NetworkInfoScreen`. Holds a non-owning pointer to the currently visible screen; all rendering methods are no-ops while unbound.

```cpp
NetworkInfoPresenter presenter(state);

// AppNavigator fires these via setOnNetworkInfoScreenChanged:
presenter.bind(screen);   // seeds full state; subscribes to updates
presenter.unbind();        // equivalent to bind(nullptr)

// AppUi calls these after updating the store:
presenter.onNetworkInfoChanged();
presenter.onNeighboursChanged();
```

**Lifecycle:**
1. Navigator shows `NetworkInfoScreen` → fires `cb(screen*)` → `presenter.bind(screen)` seeds store state into the screen.
2. Navigator hides screen → fires `cb(nullptr)` → `presenter.unbind()` — subsequent calls are no-ops.
3. `AppUi::setNetworkInfo` / `setNeighbours` update the store *then* call the presenter notify methods. The presenter renders only if bound.

---

### `NetworkUiBinder`
**File:** `app/NetworkUiBinder.h/.cpp`  
**Namespace:** `lightinator`

Wires all `AppWIFI` event callbacks to `AppUi` calls. Owns the neighbour debounce timer.

```cpp
NetworkUiBinder binder(ui, wifi, wifiFlow, runtime);
binder.bind();        // registers all WiFi callbacks
binder.syncState();   // push current state once at startup (run under lock)
```

**Debounce:** mDNS arrivals fire `Controllers::onChanged` at burst rate. `NetworkUiBinder` arms a 300 ms one-shot `SimpleTimer` on each notification; successive arrivals within the window restart the timer. On fire, `runOnUiThread` pushes the full neighbour list.

---

### `WifiConfigFlow`
**File:** `src/ui/WifiConfigFlow.h/.cpp`  
**Namespace:** `lightinator::ui`

Coordinates the WiFi setup user flow: when to auto-open the config screen, how to respond to scan results and status messages, and when to close after successful connection.

```cpp
WifiConfigFlow flow(ui, wifi);
flow.startIfNeeded();         // auto-opens screen if not configured

// Called by NetworkUiBinder:
flow.onWifiConnected();
flow.onWifiDisconnected();
flow.onStatusMessage(msg);
flow.onScanCompleted(networks);
```

---

### `Controllers`
**File:** `app/controllers.h/.cpp`

mDNS neighbour registry. Maintains a list of discovered `Entry` values (`id`, `name`, `ip`, `ttl`, `online`). Fires an `onChanged` callback when the list is mutated.

```cpp
Controllers ctrl;
ctrl.setOnChangedCallback([]{ /* debounce + push */ });

ctrl.addOrUpdate(id, name, ip, ttl);      // inserts or updates; fires callback
ctrl.removeExpired(elapsedSeconds);        // ages TTLs; fires callback on status flip
const auto& entries = ctrl.entries();
```

---

## Widget Hierarchy

```
core::Widget              ← pure abstract: mount(parent), root()
    └── core::CompositeWidget  ← adds mountChild() + child ownership list
            └── core::Screen   ← marker base for top-level screens
                    └── screens::DecoratedScreen   ← header + body slot + close btn
                    └── screens::MainScreen
                    └── screens::NetworkInfoScreen
                    └── screens::WifiConfigScreen
                    └── screens::ColorPickerScreen

core::SlotHost            ← interface: setBody(Widget)
    └── screens::DecoratedScreen  (also inherits Screen)
```

### `core::Widget`
Pure virtual base. Implement `mount(lv_obj_t* parent)` to create LVGL objects. Call `setRoot(obj)` to register the widget's root for use by `root()`.

### `core::CompositeWidget`
Adds `mountChild(unique_ptr<Widget>, parent)`: mounts the child then transfers ownership into an internal vector. Use this to build compound widgets whose children need guaranteed lifetime.

### `core::Screen`
Marker class. No additional API beyond `CompositeWidget`. Signals that this widget is a full-screen root.

### `screens::DecoratedScreen`
Reusable full-screen template: coloured header strip with title label + optional close button + optional status icon, and a body content slot.

```cpp
HeaderOptions opts;
opts.text       = "Network";
opts.height     = 60;
opts.color      = lv_color_hex(0x4189C8);
opts.font       = &lv_font_montserrat_36;
opts.showClose  = true;

auto screen = std::make_unique<DecoratedScreen>(opts);
screen->mount(parent);
screen->setOnCloseRequested([]{ nav.showMainScreen(); });
screen->setHeaderStatusIcon(LV_SYMBOL_WIFI, true);
screen->setOnHeaderStatusIconTapped([]{ /* ... */ });

// Place body content:
screen->setBody(std::make_unique<MyBodyWidget>());
// or use bodySlot() directly:
auto bodyLayout = lv_obj_create(screen->bodySlot());
```

---

## Value Types

### `core::HsvColor`
**File:** `src/ui/core/HsvColor.h`

Fixed-point HSV representation. Hue: 0–3590 (× 10 deg), Saturation/Value: 0–1000 (× 10 %).

```cpp
HsvColor c { .h = 1200, .s = 1000, .v = 800 };  // 120°, 100%, 80%
uint16_t lvHue    = toLvHue(c.h);       // → 120 (0–359 for LVGL)
uint8_t  lvSat    = toLvPercent(c.s);   // → 100 (0–100 for LVGL)
HsvColor clamped  = clampHsv(c);
```

---

## Startup Sequence

`application.cpp` is the composition root. The canonical init order:

```cpp
void init()
{
    // 1. Allocate services
    s_cfg           = make_unique<AppConfig>("app-config");
    s_wifi          = make_unique<AppWIFI>(*s_cfg);
    s_wifi_flow     = make_unique<WifiConfigFlow>(s_ui, *s_wifi);
    s_net_ui_binder = make_unique<NetworkUiBinder>(s_ui, *s_wifi,
                          s_wifi_flow.get(), s_ui_runtime);

    // 2. Hardware bring-up
    auto caps = s_hw.init();
    if (!caps.displayOk) return;

    // 3. LVGL init (under lock)
    s_ui_runtime.runOnUiThread([&]{ s_ui.init(); });

    // 4. Wire events, start networking, sync initial state
    s_net_ui_binder->bind();
    s_wifi->init();
    s_ui_runtime.runOnUiThread([&]{ s_net_ui_binder->syncState(); });

    // 5. Start WiFi config flow if needed
    s_wifi_flow->startIfNeeded();

    // 6. Start LVGL tick
    s_ui_runtime.start(s_ui);
}
```

---

## How to Add a New Screen

1. **Create the screen class** in `src/ui/screens/MyScreen.h/.cpp` inheriting `core::Screen` (or `DecoratedScreen` for the standard header/body layout).
2. **Add a factory method** to `ScreenFactory`: `createMyScreen(callbacks...)` — allocate, inject callbacks, return `unique_ptr`.
3. **Add navigation methods** to `AppNavigator`: `showMyScreen()` / `closeMyScreen()`. Call `clearRoot()` first, then factory, then `screen->mount(root_)`.
4. **Add state** to `UiStateStore` if the screen needs persistent state (add field + diffing setter returning `bool`).
5. **Write a presenter** (`MyScreenPresenter`) if the screen needs to reflect store state:
   - Constructor takes `UiStateStore&`.
   - `bind(MyScreen*)` seeds state, stores non-owning pointer.
   - `unbind()` (or `bind(nullptr)`) clears pointer; subsequent notify calls are no-ops.
   - Notify methods (`onXChanged()`) check bound pointer before calling screen API.
6. **Register the presenter lifecycle hook** in `AppUi::init()`:
   ```cpp
   navigator_->setOnMyScreenChanged([this](screens::MyScreen* s){
       myPresenter_.bind(s);
   });
   ```
7. **Expose state setter** on `AppUi` that updates the store and calls the presenter notify method.
8. **Wire the event** in `NetworkUiBinder` (or a new binder) if the state comes from a service.

---

## Namespace Summary

| Namespace | Contents |
|---|---|
| `lightinator` | Application-layer services: `HardwareInitService`, `UiRuntimeService`, `NetworkUiBinder` |
| `lightinator::ui` | UI framework root: `AppUi`, `AppNavigator`, `ScreenFactory`, `UiStateStore`, presenters, flows |
| `lightinator::ui::core` | Primitives: `Widget`, `CompositeWidget`, `Screen`, `SlotHost`, `HsvColor` |
| `lightinator::ui::screens` | Concrete screens: `MainScreen`, `NetworkInfoScreen`, `ColorPickerScreen`, `WifiConfigScreen`, `DecoratedScreen` |

---

## File Layout

```
app/
    application.cpp          ← composition root
    HardwareInitService.h/cpp
    UiRuntimeService.h/cpp
    NetworkUiBinder.h/cpp
    controllers.h/cpp         ← mDNS neighbour registry
    networking.h/cpp          ← AppWIFI
    mdnsHandler.h/cpp

src/ui/
    AppUi.h/cpp
    AppNavigator.h/cpp
    ScreenFactory.h/cpp
    UiStateStore.h/cpp
    NetworkInfoPresenter.h/cpp
    WifiConfigFlow.h/cpp
    core/
        Widget.h
        CompositeWidget.h
        Screen.h
        SlotHost.h
        HsvColor.h
    screens/
        DecoratedScreen.h/cpp
        MainScreen.h/cpp
        NetworkInfoScreen.h/cpp
        ColorPickerScreen.h/cpp
        WifiConfigScreen.h/cpp
```
