#pragma once

#include <memory>
#include <vector>

#include "ui/core/Widget.h"

namespace lightinator::ui::core {

class CompositeWidget : public Widget {
protected:
    void mountChild(std::unique_ptr<Widget> child, lv_obj_t* parent)
    {
        if (!child) {
            return;
        }

        child->mount(parent);
        children_.push_back(std::move(child));
    }

private:
    std::vector<std::unique_ptr<Widget>> children_;
};

} // namespace lightinator::ui::core
