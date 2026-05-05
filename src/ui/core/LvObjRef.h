#pragma once

#include <lvgl.h>

namespace lightinator::ui::core {

/**
 * Non-owning LVGL object reference that auto-nulls on LV_EVENT_DELETE.
 *
 * This helps avoid use-after-free by turning stale lv_obj_t* into nullptr.
 */
class LvObjRef {
public:
    LvObjRef() = default;

    explicit LvObjRef(lv_obj_t* obj)
    {
        attach(obj);
    }

    LvObjRef(const LvObjRef&) = delete;
    LvObjRef& operator=(const LvObjRef&) = delete;

    LvObjRef(LvObjRef&& other) noexcept
    {
        attach(other.obj_);
        other.detach();
    }

    LvObjRef& operator=(LvObjRef&& other) noexcept
    {
        if (this != &other) {
            attach(other.obj_);
            other.detach();
        }
        return *this;
    }

    ~LvObjRef()
    {
        detach();
    }

    void attach(lv_obj_t* obj)
    {
        if (obj_ == obj) {
            return;
        }
        detach();
        obj_ = obj;
        if (obj_ != nullptr && lv_obj_is_valid(obj_)) {
            lv_obj_add_event_cb(obj_, onObjDelete, LV_EVENT_DELETE, this);
        }
    }

    void detach()
    {
        if (obj_ != nullptr && lv_obj_is_valid(obj_)) {
            lv_obj_remove_event_cb_with_user_data(obj_, onObjDelete, this);
        }
        obj_ = nullptr;
    }

    lv_obj_t* get() const
    {
        return (obj_ != nullptr && lv_obj_is_valid(obj_)) ? obj_ : nullptr;
    }

    bool valid() const
    {
        return get() != nullptr;
    }

private:
    static void onObjDelete(lv_event_t* event)
    {
        auto* self = static_cast<LvObjRef*>(lv_event_get_user_data(event));
        if (self != nullptr) {
            self->obj_ = nullptr;
        }
    }

    lv_obj_t* obj_ = nullptr;
};

} // namespace lightinator::ui::core
