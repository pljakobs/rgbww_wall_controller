## Decision: we will use lvglCpp as the Cpp encapsulation layer.

## Goals:
- a "composable" object model that allwos me to define, as an example, a HSB color picker object (as we did, with a 2d gradient canvas, a hue slider and a point canvas, all the internal operations would be handled within that object (updating hue etc)
- we use delegates as the "emit" function, so a higher level object can subscribe to changing hsv values from that color picker
- we need some way to mimic vue's "slots" model for screens that have decoration (header, closer button etc) and an area for another object like the hsv picker to live in



# Plan: lvglCpp UI Rebuild
Replace the current EEZ-generated C++ connector approach with a fresh lvglCpp-based composition layer on top of the existing Sming display/LVGL runtime. Keep the proven platform stack (esp_lcd_adapter, lvgl_runtime_adapter, touch/display drivers), but rebuild the application/UI layer around composable C++ widgets, delegate-based event emission, and a slot-like screen composition model.

## Steps

Phase 1 
- Stabilize the UI baseline. Keep the current platform/runtime layer as-is and explicitly stop depending on the missing generated connector path from app/application.cpp. This gives the new UI layer a clean boundary.
Define a new handwritten C++ UI root, recommended under src/ui/{core,widgets,screens,layout,events} with a clear namespace such as product::ui.
Phase 2 
- Introduce lvglCpp as the thin object wrapper layer. Use it for typed LVGL object ownership and composition, but keep project-specific behavior in local classes rather than in the third-party wrapper.
Define base contracts: Widget, CompositeWidget, Screen, and SlotHost (or equivalent). Widget wraps a root LVGL object, CompositeWidget manages children, Screen is a top-level loadable view, and SlotHost exposes insertion points for child widgets.
Phase 3 
- Add delegates/signals for upward communication. Use strongly typed callbacks rather than a global event bus. Child emits, parent subscribes, parent may push state back down via explicit setters.
Define shared value types early, especially HsvColor and any small geometry/event structs, so delegate APIs stay independent from raw LVGL types.
Phase 4 
- Build the first composable widget: HSB color picker. Implement it as a composite that owns the 2D saturation/value canvas, hue slider, pointer marker, redraw logic, and internal synchronization.
Keep the HSB picker public API small: construct/mount, setValue, value, onColorChanged, onColorCommitted.
Phase 5 
- Implement slot-style composition. Create a DecoratedScreen or FramedScreen that owns common chrome like title/header/close button and exposes a body slot. Add more named slots only when real screens require them.
Prefer explicit slot mounting APIs over implicit templating. Example direction: setBody(std::unique_ptr<Widget>) or mountInto(SlotId::Body, child).
Phase 6 
- Rebuild screens around the new model. Start with one decorated screen hosting the HSB picker in its body slot, then migrate other screens using the same composition primitives.
Replace the current application’s direct connector dependency in app/application.cpp with a root app controller that constructs the screen tree, mounts the initial screen after esp_lcd_adapter_init(), registers touch, and binds delegates to app actions.
Phase 7 
- Remove obsolete generated UI coupling. Clean ui_codegen assumptions out of component.mk and decide whether Components/rgbww_wall_panel_ui stays temporarily as a reference or is removed once the new path is proven.
Phase 8 
- Add one canonical example pattern in code: decorated screen + mounted child widget + delegate subscription + reaction path. This becomes the template for future UI work.
Final integration: ensure all widget creation/mutation follows the LVGL locking/ownership rules from lvgl_runtime_adapter.h.


## demo app:

- defines a custom screen object containing:
  - a text lable 
    - width=100 (fixed)
    - height configurable, default 60px
    - color configurable (default 4189C8)
    - text configurable
    - text font configurable (default Montserrat_36)
    - a "close" button on the right side, controlled by a boolean, that, if clicked, closes the screen (not applicable on the main screen)
  - below that lable is free space where other objects can be rendered

- defines a custom HSV color picker containing:
  - a square canvas that serves as the S/V selector with a 2d gradient 
  - a 10x10px canvas (a single pixel black or white outline, turns white when the underlying color is very dark (v<25)) as the location marker
  - below that a horizontal slider (value 0-359) to select the hue component. Moving the slider updates the canvas color
  - a delegate that emits the current h,s,v value whenever it's changed
  - an ok button to close the color picker

- a color picker screen that places the HSV color picker on a custom screen 
- a main page with a single button to open the color picker 
