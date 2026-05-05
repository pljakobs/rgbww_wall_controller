# esp_lcd_adapter

Bridge between the LVGL runtime and board-specific display drivers using esp_lcd.

## Architecture

```
LVGL Runtime
     ↓
esp_lcd_adapter (this component)
     ↓
display_driver_interface.h (abstract contract)
     ↓
Board-specific driver (e.g., display_driver_guition_4848s040)
     ↓
esp_lcd panel + DMA + GPIO
```

## Interface Contract

All board-specific display drivers implement the `display_driver_interface.h` contract:

- **Opaque handles**: Drivers are polymorphic via void pointers and vtables.
- **Lifecycle**: Create → Init → Ready check → Flush → Deinit → Destroy.
- **Flush**: Non-blocking pixel submission; drivers handle DMA internally.
- **Sync**: `wait_flush_complete()` provides frame synchronization.
- **Error handling**: Uniform `esp_err_t` return codes.

## Board-Specific Drivers

Each driver (e.g., `display_driver_guition_4848s040`) must:

1. Define a board-specific config struct extending `display_driver_config_t`.
2. Implement the `display_driver_vtable_t` function pointers.
3. Provide factory functions:
   - `display_driver_<name>_create()` – allocate and initialize.
   - `display_driver_<name>_destroy()` – deallocate and release resources.
4. Optionally provide accessor functions (e.g., `display_driver_<name>_get_config()`).

## Example Usage (from adapter perspective)

```c
#include "esp_lcd_adapter/display_driver_interface.h"
#include "display_driver_guition_4848s040/display_driver_guition_4848s040.h"

// Create
display_driver_guition_4848s040_config_t cfg = {
    .base = {.width = 480, .height = 480},
    .pclk_frequency_hz = 12000000,
    .pclk_inverted = false,
    .dma_buffer_size = 0,  // Use default
};

display_driver_t *driver;
esp_err_t ret = display_driver_guition_4848s040_create(&cfg, &driver);
assert(ret == ESP_OK);

// Initialize
ret = display_driver_init(driver, &cfg.base);
assert(ret == ESP_OK);
assert(display_driver_is_ready(driver));

// Flush pixel region
ret = display_driver_flush(
    driver,
    0, 0, 479, 479,
    pixel_buffer,
    pixel_buffer_size);
assert(ret == ESP_OK);

// Wait for completion
ret = display_driver_wait_flush_complete(driver);
assert(ret == ESP_OK);

// Deinit and destroy
display_driver_deinit(driver);
display_driver_guition_4848s040_destroy(driver);
```

## Design Rationale

- **Abstraction**: Adapters are decoupled from hardware details via vtables.
- **Polymorphism in C**: Function pointers + void pointers enable multiple drivers.
- **Clear ownership**: Drivers own DMA setup, sync primitives, and esp_lcd panels.
- **Adapter responsibility**: Adapters focus on LVGL integration and frame scheduling.
- **No circular dependencies**: Drivers do not know about adapters or LVGL.

## Next Steps

- Implement `esp_lcd_adapter.c` with frame buffer management and LVGL callbacks.
- Wire real esp_lcd panel initialization into board-specific drivers.
- Add integration tests.
