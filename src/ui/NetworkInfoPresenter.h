#pragma once

#include "ui/UiStateStore.h"
#include "ui/screens/NetworkInfoScreen.h"

namespace lightinator::ui {

/**
 * NetworkInfoPresenter — binds UiStateStore to NetworkInfoScreen.
 *
 * Holds a non-owning pointer to the active NetworkInfoScreen.
 * Renders to the screen only while bound (i.e. while the screen is visible).
 * All state-to-view mapping is centralised here; AppUi only calls the
 * onNetworkInfoChanged / onNeighboursChanged notification methods.
 */
class NetworkInfoPresenter {
public:
    explicit NetworkInfoPresenter(UiStateStore& state);

    /// Bind to the newly-shown screen and seed it with the current store state.
    /// Pass nullptr to unbind (screen navigated away).
    void bind(screens::NetworkInfoScreen* screen);

    /// Explicit unbind; equivalent to bind(nullptr).
    void unbind();

    /// Called when network info in the store may have changed. No-op if unbound.
    void onNetworkInfoChanged();

    /// Called when neighbours in the store may have changed. No-op if unbound.
    void onNeighboursChanged();

private:
    void pushNetworkInfo();
    void pushNeighbours();

    UiStateStore& state_;
    screens::NetworkInfoScreen* screen_ = nullptr;
};

} // namespace lightinator::ui
