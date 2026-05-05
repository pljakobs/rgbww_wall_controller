#pragma once

#include <memory>
#include <vector>

#include <lvgl.h>
#include <lvglCpp.h>

#include "ui/core/Screen.h"
#include "ui/core/UiTheme.h"
#include "ui/screens/DecoratedScreen.h"

namespace lightinator::ui::screens {

class NetworkInfoScreen : public core::Screen {
public:
    struct Neighbour {
        String name;
        String ip;
    };

    explicit NetworkInfoScreen(const core::UiTheme& theme);

    void mount(lv_obj_t* parent) override;

    void setOnCloseRequested(std::function<void()> callback);
    void setNetworkInfo(bool connected, const String& ip, const String& netmask, const String& gateway);
    void setNeighbours(const std::vector<Neighbour>& neighbours);

private:
    void refreshLabels();
    void refreshNeighbours();

    bool connected_ = false;
    String ip_;
    String netmask_;
    String gateway_;
    core::UiTheme theme_;
    std::vector<Neighbour> neighbours_;
    std::function<void()> onCloseRequested_;

    std::unique_ptr<DecoratedScreen> decorated_;
    std::unique_ptr<lvgl::widget::Object> bodyLayout_;
    std::unique_ptr<lvgl::widget::Label> connectedLabel_;
    std::unique_ptr<lvgl::widget::Label> ipLabel_;
    std::unique_ptr<lvgl::widget::Label> netmaskLabel_;
    std::unique_ptr<lvgl::widget::Label> gatewayLabel_;
    std::unique_ptr<lvgl::widget::Label> neighboursTitleLabel_;
    std::unique_ptr<lvgl::widget::Object> neighboursList_;
};

} // namespace lightinator::ui::screens
