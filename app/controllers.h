#pragma once

#include <SmingCore.h>

#include <functional>
#include <vector>

class Controllers {
public:
    struct Entry {
        uint32_t id = 0;
        String name;
        String ip;
        int ttl = 0;
        bool online = false;
    };

    /// Upsert controller entry by id.
    ///
    /// Update semantics:
    /// - name/ip are only replaced when non-empty values are provided.
    /// - ttl/online are always refreshed from the incoming ttl.
    void addOrUpdate(uint32_t id, const String& name, const String& ip, int ttl);
    void removeExpired(int elapsedSeconds);

    size_t size() const
    {
        return entries_.size();
    }

    const Entry* findById(uint32_t id) const;
    const std::vector<Entry>& entries() const
    {
        return entries_;
    }

    /// Called whenever the list changes (entry added, updated, or went offline).
    void setOnChangedCallback(std::function<void()> cb)
    {
        onChanged_ = std::move(cb);
    }

private:
    std::vector<Entry> entries_;
    std::function<void()> onChanged_;
};
