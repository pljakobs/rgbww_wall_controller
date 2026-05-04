#include "ui/screens/MenuTestScreen.h"

namespace lightinator::ui::screens {

MenuTestScreen::MenuTestScreen(const core::UiTheme& theme)
    : theme_(theme)
{
}

void MenuTestScreen::setOnCloseRequested(std::function<void()> callback)
{
    onCloseRequested_ = std::move(callback);
}

void MenuTestScreen::mount(lv_obj_t* parent)
{
    HeaderOptions header = {};
    header.text = "MenuTest";
    header.showClose = true;
    header.height = theme_.headerHeight;
    header.color = theme_.colors.headerBg;
    header.font = theme_.fonts.header;

    decorated_ = std::make_unique<DecoratedScreen>(header);
    decorated_->setOnCloseRequested([this]() {
        if (onCloseRequested_) {
            onCloseRequested_();
        }
    });

    decorated_->mount(parent);
    setRoot(decorated_->root());

    lv_obj_t* body = decorated_->bodySlot();
    lv_obj_set_style_bg_color(body, theme_.colors.contentBg, 0);
    lv_obj_set_style_bg_opa(body, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(body, 12, 0);
    lv_obj_set_style_pad_row(body, 10, 0);
    lv_obj_set_flex_flow(body, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(body, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t* title = lv_label_create(body);
    lv_label_set_text_static(title, "TODO: implement screen content");
    lv_obj_set_style_text_font(title, theme_.fonts.contentSubheader, 0);
    lv_obj_set_style_text_color(title, theme_.colors.contentFg, 0);
}

} // namespace lightinator::ui::screens
