#pragma once

#include <lvgl.h>

namespace lightinator::ui::core {

class Widget {
public:
    virtual ~Widget() = default;

    virtual void mount(lv_obj_t* parent) = 0;

    lv_obj_t* root() const
    {
        return root_;
    }

protected:
    void setRoot(lv_obj_t* root)
    {
        root_ = root;
    }

private:
    lv_obj_t* root_ = nullptr;
};

} // namespace lightinator::ui::core
