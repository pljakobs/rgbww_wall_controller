#pragma once

#include <SmingCore.h>
#include <Network/Mdns/Handler.h>
#include <Network/Mdns/Responder.h>
#include <SimpleTimer.h>

#include <memory>

#include "controllers.h"

class mdnsHandler : public mDNS::Handler {
public:
    explicit mdnsHandler(Controllers& controllers);
    ~mdnsHandler() override;

    void start();
    void setHostname(const String& hostname);
    bool onMessage(mDNS::Message& message) override;

private:
    class WebService : public mDNS::Service {
    public:
        void setInstance(const String& instance)
        {
            instance_ = instance;
        }

        String getInstance() override
        {
            return instance_;
        }

        String getName() override
        {
            return F("http");
        }

        Protocol getProtocol() override
        {
            return Protocol::Tcp;
        }

        uint16_t getPort() override
        {
            return 80;
        }

        void addText(mDNS::Resource::TXT& txt) override
        {
            txt.add(F("type=host"));
        }

    private:
        String instance_;
    };

    static void sendSearchCb(void* self);
    void sendSearch();
    bool processSwarmResponse(mDNS::Message& message);

    Controllers& controllers_;
    std::unique_ptr<mDNS::Responder> responder_;
    WebService webService_;
    SimpleTimer searchTimer_;
    String searchName_ = "_lightinator._tcp.local";
    int searchIntervalMs_ = 15000;
    bool started_ = false;
};
