## Local build configuration
## Parameters configured here will override default and ENV values.

## Use explicit component search paths.
## Components are vendored as local submodules under Components/.
## The esp-iot-solution LCD components (esp_lcd_st7701, esp_lcd_panel_io_additions)
## are fetched as a sparse-clone under libs/esp-iot-solution.
## Point Sming at the GUITION-specific sdkconfig additions (enables OPI PSRAM).
## Without PSRAM the 460 KB RGB framebuffer cannot be allocated and the driver
## crashes in lcd_rgb_panel_destory with a NULL hal.dev pointer.
SDK_CUSTOM_CONFIG := sdkconfig.custom

export COMPONENT_SEARCH_DIRS := \
    $(CURDIR)/Components \
    $(CURDIR)/libs

ESP_IOT_LCD_DIR := $(CURDIR)/libs/esp-iot-solution/components/display/lcd
ifneq (,$(wildcard $(ESP_IOT_LCD_DIR)))
export COMPONENT_SEARCH_DIRS += $(ESP_IOT_LCD_DIR)
endif

RGBWW_COMPONENT_DIR := $(if $(wildcard $(CURDIR)/Components/rgbww_wall_panel_ui/component.mk),$(CURDIR)/Components/rgbww_wall_panel_ui,$(CURDIR)/../rgbww_wall_panel_ui)
LVGL_RUNTIME_ADAPTER_DIR := $(if $(wildcard $(CURDIR)/Components/lvgl_runtime_adapter/component.mk),$(CURDIR)/Components/lvgl_runtime_adapter,$(CURDIR)/../lvgl_runtime_adapter)

export COMPONENT_SEARCH_DIRS += \
    $(RGBWW_COMPONENT_DIR) \
    $(LVGL_RUNTIME_ADAPTER_DIR)

COMPONENT_DEPENDS := esp_lcd_adapter rgbww_wall_panel_ui touch_driver_gt911

## esp_lcd and esp_psram are added to the Sming SDK_COMPONENTS list in
## /opt/sming/Sming/Arch/Esp32/Components/esp32/component.mk so they are
## built as part of the shared SDK build.  EXTRA_LIBS ensures they land
## inside --start-group / --end-group in the final link step.
EXTRA_LIBS := esp_lcd esp_psram
NUM_JOBS := $(shell echo $(($(nproc) + 2)))
MAKEFLAGS += -j$(NUM_JOBS)


## If you use third-party libraries and they are stored in libs/ 
## then you have to include them in the application configuration.
## Below is an example of using two third-party libraries and one global library 
# LOCAL_COMPONENTS := AXP202X BMA423
# COMPONENT_DEPENDS := Graphics $(LOCAL_COMPONENTS)
# COMPONENT_SUBMODULES := $(addprefix libs/,$(LOCAL_COMPONENTS))

## If your application is supporting only some architectures you can set them. If not set all architectures are supported
# SUPPORTED_ARCH := Host Esp32

ifneq (,$(SUPPORTED_ARCH))
	ifeq (,$(filter $(SMING_ARCH),$(SUPPORTED_ARCH)))
		$(error "Unsupported platform. Only Esp32 and Host are supported")
	endif
endif

COMPONENT_SRCDIRS := \
    app \
    src \
    src/ui_codegen/generated \
    src/Arch/$(SMING_ARCH)

COMPONENT_INCDIRS := \
    $(COMPONENT_SRCDIRS) \
    src/ui_codegen
