#pragma once

#include <memory>

#include "ui/core/Widget.h"

namespace lightinator::ui::core {

class SlotHost {
public:
    virtual ~SlotHost() = default;
    virtual void setBody(std::unique_ptr<Widget> widget) = 0;
};

} // namespace lightinator::ui::core
