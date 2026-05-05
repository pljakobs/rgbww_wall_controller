Great question. Right now AppUi is doing four jobs at once: navigation, screen construction, state storage, and UI update routing. You can scale much better if those are split.

Most visible hotspots are in src/ui/AppUi.cpp, around showMainScreen, showColorPickerScreen, showNetworkInfoScreen, showWifiConfigScreen, and setNetworkInfo.

A cleaner separation looks like this:

1. AppNavigator (only screen transitions)
- Owns current screen enum and transition rules.
- Example API: showMain, showWifiConfig, showColorPicker, showNetworkInfo.
- Moves all screen-switching logic out of AppUi.

2. UiStateStore (only state)
- Holds wifiConnected, IP, netmask, gateway, neighbours, currentColor.
- Exposes update methods and change notifications.
- No LVGL calls inside this layer.

3. ScreenFactory (only construction/wiring)
- Creates screen instances and injects callbacks.
- Keeps creation details for MainScreen, WifiConfigScreen, ColorPickerScreen, NetworkInfoScreen in one place.
- Removes repetitive setup code from AppUi.

4. ScreenController or Presenter per screen (only binding state to view)
- MainScreenPresenter updates wifi icon, color preview.
- NetworkInfoPresenter updates labels and neighbour list.
- WifiConfigPresenter handles scan/connect/status text lifecycle.
- Removes direct state push calls from AppUi.

5. Thin AppUi shell (only LVGL root ownership)
- Keeps root object lifecycle and mounting host.
- Delegates everything else to Navigator + Factory + StateStore.
- AppUi becomes very small and stable.

Recommended migration order (low risk):

1. ✅ Extract UiStateStore first, keep AppUi calling into it.
2. ✅ Extract AppNavigator next, move showX methods there.
3. ✅ Add ScreenFactory to centralize screen construction.
4. ✅ Introduce one presenter first (NetworkInfo), then expand to others.
5. Finally shrink AppUi to root management only.

What you get:

1. Better testability (state logic can be unit-tested without LVGL).
2. Easier feature growth (new screen = add presenter + factory entry, not edit a god class).
3. Lower coupling and fewer regressions when changing network/UI behavior.

Also separate bootstrap boilerplate from application.cpp

application.cpp is currently doing hardware setup, runtime setup, callback wiring, state synchronization, and timer scheduling in one place. For larger growth, split this into focused services.

1. AppBootstrap (or AppCompositionRoot)
- Owns startup sequence only.
- Creates and wires AppConfig, AppWIFI, AppUi, WifiConfigFlow.
- Keeps init() readable and linear.

2. HardwareInitService
- Encapsulates backlight, display adapter init, touch init.
- Exposes one result object with capabilities (displayOk, touchOk).

3. UiRuntimeService
- Owns LVGL lock/unlock usage and periodic tick registration.
- Provides runOnUiThread(lambda) helper to avoid lock boilerplate at every callback.

4. NetworkUiBinder
- Encapsulates all AppWIFI -> AppUi callback registration.
- Handles status, connected/disconnected, scan completed, and initial sync.
- Keeps networking concerns out of application entrypoint.

5. StartupStateSync
- One function to push initial network state and neighbours into UI.
- Reused after reconnect events to avoid duplicated update code.

Recommended migration order for application.cpp cleanup:

1. ✅ Extract HardwareInitService first (lowest risk).
2. ✅ Extract NetworkUiBinder for callback setup.
3. ✅ Extract UiRuntimeService for timer + lock helpers.
4. ✅ Introduce AppBootstrap last to compose all modules.

Outcome:
- Much smaller init() function.
- Clear ownership for hardware, runtime, and app wiring.
- Easier testing/mocking of startup paths and error handling.

Also optimize update cycles (Network Info CPU load)

Current pain point: Network Info refresh is too frequent and causes high CPU when screen is open. mDNS neighbours do not change at frame-rate speed and should not be redrawn continuously.

1. Move from polling to event-driven updates
- Stop unconditional periodic push of network state into UI.
- Publish only on meaningful events: WiFi connect/disconnect, DHCP change, mDNS neighbour add/remove/update.

2. Add per-field refresh throttling
- Fast-changing fields (connection status): max 2-4 Hz.
- IP/netmask/gateway: update only on value change.
- mDNS neighbours: max 0.5-1 Hz, or immediate only when list actually changes.

3. Add dirty-flag + diff rendering in NetworkInfoPresenter
- Track last rendered snapshot (status, ip, netmask, gateway, neighbours hash/count).
- If unchanged, skip label/list updates entirely.
- For neighbour list, avoid full lv_obj_clean/recreate when only one entry changed.

4. Refresh only while visible
- Run network-info UI updates only when NetworkInfoScreen is active.
- When hidden, keep state updates in store but defer rendering.

5. Coalesce burst updates
- During reconnect or mDNS bursts, coalesce into one UI update per throttle window.
- Use a small deferred timer (for example 200-500 ms) to batch updates.
 

6. Add performance guardrails
- Add lightweight instrumentation around update_ui_network_state and screen render methods.
- Track average refresh time and update frequency.
- Set target CPU budget for idle-on-network-screen mode and verify after each change.

Suggested implementation order:

1. ✅ Implement visible-screen gating first (quick win).
2. ✅ Add snapshot diff checks in presenter.
3. ✅ Add neighbour-list-specific throttling and coalescing (300 ms debounce in NetworkUiBinder; Controllers fires onChange callback on add/update/expire).
4. ✅ Remove remaining unconditional timer-driven UI pushes.


Looks:

- the visible elements should follow a theming model, allowing to apply various themes. The main theming elements are colors, element sizes and forms and fonts. We can extend the app-config.cfgdb to store themes I suggest we start with 
  - header 
    - height
    - background color
    - foreground color
    - font height
    - edge rounding 
  - font family
  - font size



Architecture ground rules

These are guardrails for all upcoming work. New code should follow these rules unless there is an explicit documented exception.

1. Single-direction data flow
- Services publish state changes.
- UiStateStore owns canonical UI-facing state.
- Presenters map state to views.
- Screens emit user intents only.

2. Hard separation of responsibilities
- AppUi: LVGL root ownership only.
- AppNavigator: route/screen transitions only.
- ScreenFactory: screen creation and callback injection only.
- Presenters: rendering and view binding only.
- Services (WiFi/mDNS/hardware): no direct LVGL widget updates.

3. UI-thread discipline
- All LVGL mutations go through UiRuntimeService (or equivalent UI-thread gateway).
- No networking/hardware callback may call widget APIs directly.

4. Event-driven updates by default
- Do not use unconditional periodic full-state UI pushes.
- Update only on meaningful state changes.
- Throttle or coalesce noisy sources (especially mDNS neighbour updates).

5. Render efficiency requirements
- Diff before render.
- Skip unchanged label/list updates.
- Avoid full list rebuilds when only incremental neighbour changes occurred.

6. Visible-screen rendering policy
- Render only when the target screen is active.
- Hidden screens may receive state updates in the store but should not consume render budget.

7. Lifecycle clarity
- Startup wiring belongs in bootstrap/composition root.
- Feature logic belongs in services and presenters, not application.cpp.

8. Testability baseline
- Unit tests for state updates and presenter diff/throttle behavior.
- Integration smoke test for startup composition and basic route transitions.

Definition of done for architecture stabilization

1. application.cpp is composition/bootstrap orchestration only.
2. AppUi no longer owns navigation, business state, or service callback wiring.
3. Network Info screen update path is event-driven + throttled with low idle CPU.
4. At least one full screen flow is presenter-driven end-to-end (Network Info recommended first).



Performance optimization:

  - object reuse (done for menu, continue elsewhere)
  - avoid remount/recreate churn
  - diff-based updates
  - async flush/buffer tuning

we need a setings page.
among other this, that should allow to 
- set the backlight brightness (store value in Configdb (see app-config.cfgdb))
- set the backlight timeout
- allow for touch calibration (five point calibration, corners and center, propose a json schama for the calibration matrix to be stored in configdb)

Issues: 
  - fixed: ghost touches are filtered with configurable stable touch time in the GT911 path
  - fixed: the touch calibration screen registers touches using press/pressing hit-testing and captures raw touch data

  - fixed: calibration screen shows only one active colored crosshair at a time
  - fixed: calibration screen shows the detected position with a dimmed secondary crosshair
  - fixed: screen backlight timeout switches off after inactivity and the wake touch is consumed
  - fixed: the timeout slider uses 5s increments and the max value maps to "never" using `-1` in ConfigDB
  - fixed: the backlight brightness slider previews live changes on a 20 ms throttle
  - fixed: brightness changes are written to ConfigDB 250 ms after slider release
   
  - fixed: calibration now auto-closes after the fifth crosshair is captured and saved
  - fixed: calibration now shows a dimmed cross-hair at the detected touch position on screen
  - fixed: settings screen OK/Cancel are functional (OK saves; Cancel restores preview brightness and closes)

Code review issues (maintainability + clarity):
  - fixed: remove stale `createTouchCalibrationScreen(onSaved)` API contract and call sites; keep only close callback
  - fixed: remove dead deferred-save timer plumbing from SettingsScreen after moving to explicit OK save semantics
  - fixed: remove AppUi file-static theme apply callback state; use instance-owned callback storage
  - fixed: define explicit lifecycle/teardown contract for HardwareInitService touch callback bridge (`s_activeHardwareService`) via `shutdown()` and destructor cleanup
  - fixed: remove UiRuntimeService periodic tick (AppUi updates remain event-driven)
  - fixed: reduce AppNavigator screen-transition duplication with a common `resetScreenInstances()` helper
  - fixed: split `application.cpp` responsibilities by extracting display settings + touch calibration persistence/solver into `DisplaySettingsService`
  - fixed: consolidate behavioral constants into shared policy headers (`app/AppPolicy.h`, `src/ui/core/UiPolicy.h`)
  - fixed: gate MenuTestScreen from normal user navigation (menu entry removed)
  - fixed: clean up AppNavigator header formatting/style drift
  - fixed: document Controllers::addOrUpdate merge semantics for empty name/ip updates
  - open: introduce a lightweight test target and smoke tests for navigation/calibration/settings persistence flows
  - fixed: hue slider mapping corrected (slider direction now matches displayed hue strip)


add features:
  from ~/devel/esp_rgbww_firmware, copy ./include/MultiOutputStream.h and ./include/udpSyslogStream.h and wire those in to the app including a configuration settings page
  
later:

Moving from an integrated app and framework to a framework:

Define stable framework interfaces.
Move generic screens/components into a framework namespace/package.
Keep app-specific services (WiFi, mDNS, ConfigDB, app wiring) in the application.
Add storage adapters so theme editor/selector can save to JSON or ConfigDB through the same interface.
What Belongs In Framework vs App
Framework candidate:

Theme model and theme rendering contract
Theme selector screen
Theme editor screen
HSV color picker widget
Generic navigable menu screen/pattern
Base decorated screen/header shell
Screen navigation primitives and factory interfaces
Optional persistence abstraction for themes
App-specific:

ConfigDB implementation
AppWIFI and network binders/presenters
Product menu entries and feature routes
Hardware bootstrap and platform setup
Product defaults and product-specific theme packs
Concrete Boundary Proposal
Create three layers:

UI Core (framework core)
Theme types, base screen abstractions, navigation interfaces, generic widgets
No ConfigDB, no WiFi, no product logic
Mostly what is now in UiTheme.h, decorated screen base, color picker, selector/editor logic
UI Feature Pack (framework features)
ThemeSelectorScreen, ThemeEditorScreen, generic menu screen
Depends only on UI Core + LVGL
Uses interfaces for persistence and listing
Product App
Implements storage and service adapters
Wires callbacks and routes
Keeps current app services and state store
Key Interfaces To Introduce
These are the main seams that let you detach ConfigDB cleanly:

Theme repository interface
listThemes()
getThemeById(id)
saveTheme(theme)
optional deleteTheme(id)
Theme source metadata
isBuiltin
source type enum (builtin, user, imported)
used by naming strategy in selector/editor
Naming strategy interface
suggestNameForEdit(theme, allThemes)
default strategy can implement your current:
builtin: name-user
user: name-edit, name-edit-1, etc.
Serializer interface
toJson(theme)
fromJson(json)
lets you support JSON export/import and ConfigDB persistence side-by-side
Menu model interface
menu as data (items + actions), not hardcoded app buttons
How Your Existing Files Map
Good extraction candidates:

ThemeSelectorScreen.cpp
ThemePreviewScreen.cpp (rename to ThemeEditorScreen in framework)
HsvColorPicker.cpp
DecoratedScreen.cpp
Needs app adapter after extraction:

ScreenFactory.cpp
AppNavigator.cpp
builtin_themes.cpp (can be split into framework default pack + app pack)
application.cpp
Recommended Extraction Sequence

Move theme domain types and naming logic into framework core.
Extract ThemeSelectorScreen + ThemeEditorScreen to framework feature pack with repository interface.
Extract HSV picker and DecoratedScreen.
Replace direct callbacks in factory with injected interfaces from app layer.
Add two repository implementations in app:
ConfigDbThemeRepository
JsonFileThemeRepository (or in-memory for test/dev)
Keep AppNavigator in app first, then optionally provide a generic navigator helper later.
Packaging Options

Monorepo component style:
Components/ui_framework_core
Components/ui_framework_features
Product app depends on both
Separate repository:
Better reusability/versioning
Slightly higher initial setup effort
Hybrid:
Start as components in current repo, split later once stable
Biggest Risks

Callback sprawl and ownership issues across screen lifetimes
Leaking app-specific assumptions into framework (ConfigDB IDs, product menu labels)
Theme persistence coupling to one backend
Navigation assumptions tied to one app route graph
What To Do Next

I can implement the first extraction step now: create a ThemeRepository interface and migrate selector/editor to use it behind ScreenFactory while leaving behavior unchanged.
Then I can add a ConfigDB adapter and a JSON adapter side-by-side, so you can choose storage per build profile.
If you want, I’ll start with step 1 directly and keep it as a non-breaking refactor.

Qeuestion:
- can the rendering be delegated to the 2nd core of the esp32s3? 
  - sming usually diables the 2nd core and runs in a single (network) task
  - sming itself is not thread safe (everything happens in the event loop)
  - pushing the lvgl rendering to 2nd core needs specific message passing
Decision: deferred