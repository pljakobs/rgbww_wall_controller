#include "ui/NetworkInfoPresenter.h"

namespace lightinator::ui {

NetworkInfoPresenter::NetworkInfoPresenter(UiStateStore& state)
    : state_(state)
{
}

void NetworkInfoPresenter::bind(screens::NetworkInfoScreen* screen)
{
    screen_ = screen;
    if (screen_) {
        pushNetworkInfo();
        pushNeighbours();
    }
}

void NetworkInfoPresenter::unbind()
{
    screen_ = nullptr;
}

void NetworkInfoPresenter::onNetworkInfoChanged()
{
    if (!screen_) {
        return;
    }
    pushNetworkInfo();
}

void NetworkInfoPresenter::onNeighboursChanged()
{
    if (!screen_) {
        return;
    }
    pushNeighbours();
}

void NetworkInfoPresenter::pushNetworkInfo()
{
    screen_->setNetworkInfo(
        state_.wifiConnected(),
        state_.ipAddress(),
        state_.netmask(),
        state_.gateway());
}

void NetworkInfoPresenter::pushNeighbours()
{
    screen_->setNeighbours(state_.neighbours());
}

} // namespace lightinator::ui
