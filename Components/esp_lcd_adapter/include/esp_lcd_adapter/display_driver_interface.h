/**
 * Display Driver Interface Contract
 *
 * This header defines the abstract interface that all board-specific display drivers
 * must implement. The esp_lcd_adapter uses this interface to remain board-agnostic.
 *
 * Design principles:
 * - Drivers are opaque handles (void*) to allow polymorphism in C.
 * - All lifecycle operations are explicit (new, delete, ready check).
 * - Flush is the sole rendering operation; drivers batch pixels internally.
 * - Error handling is uniform (esp_err_t).
 * - No C++ dependencies; pure C interface.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Display driver opaque handle.
 * Each board-specific driver (e.g. GUITION_4848S040) casts this to its own type.
 */
typedef void *display_driver_handle_t;

/**
 * Display driver configuration structure.
 * Board-specific drivers may extend this with additional fields at runtime.
 */
typedef struct {
    uint16_t width;
    uint16_t height;
    // Board-specific drivers may add more fields as needed.
} display_driver_config_t;

/**
 * Display driver function table (vtable pattern in C).
 * Drivers implement this to provide polymorphic behavior.
 */
typedef struct {
    /**
     * Initialize the driver instance.
     * Called exactly once after creation.
     */
    esp_err_t (*init)(display_driver_handle_t driver, const display_driver_config_t *config);

    /**
     * Deinitialize and release resources.
     * Called exactly once before destruction.
     */
    esp_err_t (*deinit)(display_driver_handle_t driver);

    /**
     * Return true if driver is ready to accept flush calls.
     */
    bool (*is_ready)(display_driver_handle_t driver);

    /**
     * Flush a rectangular pixel region to the display.
     * The driver is responsible for DMA submission and completion tracking.
     *
     * Args:
     *   driver: opaque driver handle
     *   x1, y1: top-left corner (inclusive)
     *   x2, y2: bottom-right corner (inclusive)
     *   pixel_data: pointer to pixel buffer (format determined by driver)
     *   pixel_data_size: size in bytes
     *
     * Returns:
     *   ESP_OK on success
     *   ESP_ERR_INVALID_ARG if coordinates are out of bounds or pixel_data invalid
     *   ESP_ERR_INVALID_STATE if driver not initialized
     *   ESP_ERR_NO_MEM if DMA buffer allocation fails
     */
    esp_err_t (*flush)(
        display_driver_handle_t driver,
        uint16_t x1,
        uint16_t y1,
        uint16_t x2,
        uint16_t y2,
        const void *pixel_data,
        size_t pixel_data_size);

    /**
     * Wait for the last flush to complete (blocking).
     * Used by adapters to ensure frame synchronization before issuing next flush.
     *
     * Returns:
     *   ESP_OK on success
     *   ESP_ERR_TIMEOUT if wait exceeds internal timeout
     *   ESP_ERR_INVALID_STATE if no flush is in flight
     */
    esp_err_t (*wait_flush_complete)(display_driver_handle_t driver);
} display_driver_vtable_t;

/**
 * Abstract display driver base instance.
 * Each board driver embeds this structure.
 */
typedef struct {
    const display_driver_vtable_t *vtable;
    display_driver_handle_t impl;  // Board-specific implementation handle
} display_driver_t;

/**
 * Create a new display driver instance.
 * Each board driver provides this factory function with their own naming:
 *   esp_err_t display_driver_guition_4848s040_create(
 *       const display_driver_config_t *config,
 *       display_driver_t **out_driver);
 */

/**
 * Destroy a display driver instance and release all resources.
 * Board drivers provide their own destructor with their own naming.
 */

/**
 * Get driver width in pixels.
 */
static inline uint16_t display_driver_get_width(const display_driver_t *driver) {
    return driver->impl ? ((display_driver_config_t *)driver->impl)->width : 0;
}

/**
 * Get driver height in pixels.
 */
static inline uint16_t display_driver_get_height(const display_driver_t *driver) {
    return driver->impl ? ((display_driver_config_t *)driver->impl)->height : 0;
}

/**
 * Initialize driver (calls vtable->init).
 */
static inline esp_err_t display_driver_init(
    display_driver_t *driver,
    const display_driver_config_t *config) {
    if (driver == NULL || driver->vtable == NULL || driver->vtable->init == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    return driver->vtable->init(driver->impl, config);
}

/**
 * Deinitialize driver (calls vtable->deinit).
 */
static inline esp_err_t display_driver_deinit(display_driver_t *driver) {
    if (driver == NULL || driver->vtable == NULL || driver->vtable->deinit == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    return driver->vtable->deinit(driver->impl);
}

/**
 * Check driver ready state (calls vtable->is_ready).
 */
static inline bool display_driver_is_ready(const display_driver_t *driver) {
    if (driver == NULL || driver->vtable == NULL || driver->vtable->is_ready == NULL) {
        return false;
    }
    return driver->vtable->is_ready(driver->impl);
}

/**
 * Flush pixel region (calls vtable->flush).
 */
static inline esp_err_t display_driver_flush(
    display_driver_t *driver,
    uint16_t x1,
    uint16_t y1,
    uint16_t x2,
    uint16_t y2,
    const void *pixel_data,
    size_t pixel_data_size) {
    if (driver == NULL || driver->vtable == NULL || driver->vtable->flush == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    return driver->vtable->flush(driver->impl, x1, y1, x2, y2, pixel_data, pixel_data_size);
}

/**
 * Wait for flush completion (calls vtable->wait_flush_complete).
 */
static inline esp_err_t display_driver_wait_flush_complete(display_driver_t *driver) {
    if (driver == NULL || driver->vtable == NULL || driver->vtable->wait_flush_complete == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    return driver->vtable->wait_flush_complete(driver->impl);
}

#ifdef __cplusplus
}
#endif
