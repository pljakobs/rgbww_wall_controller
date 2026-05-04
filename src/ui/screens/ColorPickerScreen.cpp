#include "ui/screens/ColorPickerScreen.h"

namespace lightinator::ui::screens {

ColorPickerScreen::ColorPickerScreen(core::HsvColor initialColor, const core::UiTheme& theme)
    : initialColor_(core::clampHsv(initialColor)), currentColor_(initialColor_), theme_(theme)
{
}

void ColorPickerScreen::mount(lv_obj_t* parent)
{
    HeaderOptions header = {};
    header.text = "Color";
    header.showClose = true;
    header.height = theme_.headerHeight;
    header.color  = theme_.colors.headerBg;
    header.font   = theme_.fonts.header;

    decorated_ = std::make_unique<DecoratedScreen>(header);
    decorated_->setOnCloseRequested([this]() {
        if (onCloseRequested_) {
            onCloseRequested_();
        }
    });
    decorated_->mount(parent);
    setRoot(decorated_->root());

    auto picker = std::make_unique<widgets::HsvColorPicker>();
    picker->setValue(initialColor_);
    picker->setOnColorChanged([this](const core::HsvColor& color) {
        currentColor_ = color;
        if (onColorChanged_) {
            onColorChanged_(color);
        }
    });
    picker->setOnColorCommitted([this](const core::HsvColor& color) {
        currentColor_ = color;
        if (onColorChanged_) {
            onColorChanged_(color);
        }
        if (onCloseRequested_) {
            onCloseRequested_();
        }
    });
    decorated_->setBody(std::move(picker));
}

void ColorPickerScreen::setOnCloseRequested(std::function<void()> callback)
{
    onCloseRequested_ = std::move(callback);
}

void ColorPickerScreen::setOnColorChanged(std::function<void(const core::HsvColor&)> callback)
{
    onColorChanged_ = std::move(callback);
}

} // namespace lightinator::ui::screens
