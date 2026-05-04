## Local build configuration
## Parameters configured here will override default and ENV values.

## Target chip for the Guition JC4848S040 board (ESP32-S3)
ESP_VARIANT := esp32s3

## Use explicit component search paths.
## Components are vendored as local submodules under Components/.
## The esp-iot-solution LCD components (esp_lcd_st7701, esp_lcd_panel_io_additions)
## are fetched as a sparse-clone under libs/esp-iot-solution.
## Optionally use GUITION-specific sdkconfig additions (for example OPI PSRAM).
## If sdkconfig.custom is absent we fall back to Sming defaults.
ifneq (,$(wildcard $(CURDIR)/sdkconfig.custom))
SDK_CUSTOM_CONFIG := sdkconfig.custom
endif

HWCONFIG=partitions

export COMPONENT_SEARCH_DIRS := \
    $(CURDIR)/Components \
    $(CURDIR)/libs

ESP_IOT_LCD_DIR := $(CURDIR)/Components/esp-iot-solution/components/display/lcd
ifneq (,$(wildcard $(ESP_IOT_LCD_DIR)))
export COMPONENT_SEARCH_DIRS += $(ESP_IOT_LCD_DIR)
endif

LVGL_RUNTIME_ADAPTER_DIR := $(if $(wildcard $(CURDIR)/Components/lvgl_runtime_adapter/component.mk),$(CURDIR)/Components/lvgl_runtime_adapter,$(CURDIR)/../lvgl_runtime_adapter)

export COMPONENT_SEARCH_DIRS += \
    $(LVGL_RUNTIME_ADAPTER_DIR)

COMPONENT_DEPENDS := esp_lcd_adapter touch_driver_gt911 lvglCpp ConfigDB MDNS LittleFS

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
    src/ui \
    src/ui/core \
    src/ui/widgets \
    src/ui/screens \
    src/Arch/$(SMING_ARCH)

COMPONENT_INCDIRS := \
    $(COMPONENT_SRCDIRS)
