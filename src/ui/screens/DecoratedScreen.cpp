#include "ui/screens/DecoratedScreen.h"

namespace lightinator::ui::screens {

DecoratedScreen::DecoratedScreen(HeaderOptions options) : options_(options)
{
}

void DecoratedScreen::mount(lv_obj_t* parent)
{
    rootObj_ = std::make_unique<lvgl::widget::Object>(parent);
    lv_obj_t* root = rootObj_->GetObj();
    setRoot(root);

    lv_obj_set_size(root, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 14, 0);
    lv_obj_set_style_pad_row(root, 12, 0);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);

    headerObj_ = std::make_unique<lvgl::widget::Object>(root);
    lv_obj_t* header = headerObj_->GetObj();
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, options_.height);
    lv_obj_set_style_bg_color(header, options_.color, 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(header, 12, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_left(header, 10, 0);
    lv_obj_set_style_pad_right(header, 10, 0);
    lv_obj_set_style_pad_top(header, 6, 0);
    lv_obj_set_style_pad_bottom(header, 6, 0);
    lv_obj_set_style_pad_column(header, 10, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    titleLabel_ = std::make_unique<lvgl::widget::Label>(header);
    lv_obj_t* title = titleLabel_->GetObj();
    lv_label_set_text(title, options_.text);
    lv_obj_set_flex_grow(title, 1);
    lv_obj_set_style_min_width(title, 0, 0);
    lv_obj_set_style_text_font(title, options_.font, 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_pad_right(title, 6, 0);
    lv_label_set_long_mode(title, LV_LABEL_LONG_DOT);

    statusIconBox_ = std::make_unique<lvgl::widget::Object>(header);
    lv_obj_t* statusBox = statusIconBox_->GetObj();
    lv_obj_set_size(statusBox, 44, 44);
    lv_obj_set_flex_grow(statusBox, 0);
    lv_obj_set_style_bg_opa(statusBox, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(statusBox, 0, 0);
    lv_obj_set_style_pad_all(statusBox, 0, 0);
    lv_obj_add_flag(statusBox, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(statusBox, onHeaderStatusIconEvent, LV_EVENT_CLICKED, this);
    lv_obj_add_flag(statusBox, LV_OBJ_FLAG_HIDDEN);

    statusIconLabel_ = std::make_unique<lvgl::widget::Label>(statusBox);
    lv_obj_t* statusLabel = statusIconLabel_->GetObj();
    lv_obj_set_style_text_font(statusLabel, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(statusLabel, lv_color_white(), 0);
    lv_label_set_text_static(statusLabel, "");
    lv_obj_center(statusLabel);

    if (options_.showClose) {
        closeButton_ = std::make_unique<lvgl::widget::Button>(header);
        lv_obj_t* close = closeButton_->GetObj();
        lv_obj_set_size(close, 54, 44);
        lv_obj_set_flex_grow(close, 0);
        lv_obj_set_style_radius(close, 10, 0);
        lv_obj_set_style_bg_color(close, lv_color_hex(0x224F77), 0);
        lv_obj_set_style_border_width(close, 0, 0);
        lv_obj_add_event_cb(close, onCloseButtonEvent, LV_EVENT_CLICKED, this);

        auto* closeLabel = lv_label_create(close);
        lv_label_set_text_static(closeLabel, LV_SYMBOL_CLOSE);
        lv_obj_center(closeLabel);
    }

    bodyObj_ = std::make_unique<lvgl::widget::Object>(root);
    lv_obj_t* body = bodyObj_->GetObj();
    lv_obj_set_width(body, lv_pct(100));
    lv_obj_set_flex_grow(body, 1);
    lv_obj_set_style_bg_opa(body, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(body, 0, 0);
    lv_obj_set_style_pad_all(body, 0, 0);

    if (bodyWidget_) {
        bodyWidget_->mount(body);
    }
}

void DecoratedScreen::setBody(std::unique_ptr<core::Widget> widget)
{
    bodyWidget_ = std::move(widget);
    if (bodyObj_ && bodyWidget_) {
        bodyWidget_->mount(bodyObj_->GetObj());
    }
}

lv_obj_t* DecoratedScreen::bodySlot() const
{
    return bodyObj_ ? bodyObj_->GetObj() : nullptr;
}

void DecoratedScreen::setHeaderStatusIcon(const char* iconText, bool visible)
{
    if(!statusIconBox_ || !statusIconLabel_) {
        return;
    }

    lv_label_set_text(statusIconLabel_->GetObj(), iconText ? iconText : "");
    if(visible) {
        lv_obj_clear_flag(statusIconBox_->GetObj(), LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(statusIconBox_->GetObj(), LV_OBJ_FLAG_HIDDEN);
    }
}

void DecoratedScreen::setOnCloseRequested(std::function<void()> callback)
{
    onCloseRequested_ = std::move(callback);
}

void DecoratedScreen::setOnHeaderStatusIconTapped(std::function<void()> callback)
{
    onHeaderStatusIconTapped_ = std::move(callback);
}

void DecoratedScreen::onCloseButtonEvent(lv_event_t* event)
{
    auto* self = static_cast<DecoratedScreen*>(lv_event_get_user_data(event));
    if (self == nullptr) {
        return;
    }

    if (self->onCloseRequested_) {
        self->onCloseRequested_();
    }
}

void DecoratedScreen::onHeaderStatusIconEvent(lv_event_t* event)
{
    auto* self = static_cast<DecoratedScreen*>(lv_event_get_user_data(event));
    if(self == nullptr) {
        return;
    }

    if(self->onHeaderStatusIconTapped_) {
        self->onHeaderStatusIconTapped_();
    }
}

} // namespace lightinator::ui::screens
