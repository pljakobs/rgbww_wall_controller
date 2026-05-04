#include "controllers.h"

#include <algorithm>

void Controllers::addOrUpdate(uint32_t id, const String& name, const String& ip, int ttl)
{
    if(id == 0) {
        return;
    }

    auto it = std::find_if(entries_.begin(), entries_.end(), [id](const Entry& entry) { return entry.id == id; });
    if(it == entries_.end()) {
        Entry entry;
        entry.id = id;
        entry.name = name;
        entry.ip = ip;
        entry.ttl = ttl;
        entry.online = ttl > 0;
        entries_.push_back(entry);
    if(onChanged_) { onChanged_(); }
    }

    if(name.length() > 0) {
        it->name = name;
    }
    if(ip.length() > 0) {
        it->ip = ip;
    }
    it->ttl = ttl;
    it->online = ttl > 0;
    if(onChanged_) { onChanged_(); }
}

void Controllers::removeExpired(int elapsedSeconds)
{
    bool anyChanged = false;
    for(auto& entry : entries_) {
        if(entry.ttl <= 0) {
            if(entry.online) { entry.online = false; anyChanged = true; }
            continue;
        }

        entry.ttl = std::max(0, entry.ttl - elapsedSeconds);
        if(entry.ttl == 0 && entry.online) {
            entry.online = false;
            anyChanged = true;
        }
    }
    if(anyChanged && onChanged_) { onChanged_(); }
}

const Controllers::Entry* Controllers::findById(uint32_t id) const
{
    auto it = std::find_if(entries_.begin(), entries_.end(), [id](const Entry& entry) { return entry.id == id; });
    if(it == entries_.end()) {
        return nullptr;
    }

    return &(*it);
}
