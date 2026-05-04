#include "mdnsHandler.h"

#include <Network/Mdns/Resource.h>

mdnsHandler::mdnsHandler(Controllers& controllers) : controllers_(controllers)
{
}

mdnsHandler::~mdnsHandler()
{
    searchTimer_.stop();
    if(responder_) {
        mDNS::server.removeHandler(*responder_);
    }
    mDNS::server.removeHandler(*this);
}

void mdnsHandler::setHostname(const String& hostname)
{
    if(responder_) {
        mDNS::server.removeHandler(*responder_);
    }

    responder_ = std::make_unique<mDNS::Responder>();
    responder_->begin(hostname);

    webService_.setInstance(hostname);

    responder_->addService(webService_);
    mDNS::server.addHandler(*responder_);
}

void mdnsHandler::start()
{
    if(started_) {
        return;
    }
    started_ = true;

    if(!responder_) {
        char defaultName[64];
        m_snprintf(defaultName, sizeof(defaultName), "rgbwwControl-%lu", static_cast<unsigned long>(system_get_chip_id()));
        setHostname(String(defaultName));
    }

    mDNS::server.addHandler(*this);

    searchTimer_.setCallback(sendSearchCb, this);
    searchTimer_.setIntervalMs(searchIntervalMs_);
    searchTimer_.startOnce();

    sendSearch();
}

bool mdnsHandler::onMessage(mDNS::Message& message)
{
    if(!message.isReply()) {
        return false;
    }

    return processSwarmResponse(message);
}

void mdnsHandler::sendSearchCb(void* self)
{
    auto* handler = static_cast<mdnsHandler*>(self);
    if(handler == nullptr) {
        return;
    }

    handler->sendSearch();
}

void mdnsHandler::sendSearch()
{
    mDNS::server.search(searchName_.c_str());
    controllers_.removeExpired(searchIntervalMs_ / 1000);
    searchTimer_.startOnce();
}

bool mdnsHandler::processSwarmResponse(mDNS::Message& message)
{
    auto* srvAnswer = message[mDNS::ResourceType::SRV];
    if(srvAnswer == nullptr) {
        return false;
    }

    const String serviceName = String(srvAnswer->getName());
    if(!serviceName.endsWith(F("._lightinator._tcp.local"))) {
        return false;
    }

    auto* txtAnswer = message[mDNS::ResourceType::TXT];
    auto* aAnswer = message[mDNS::ResourceType::A];
    if(txtAnswer == nullptr || aAnswer == nullptr) {
        return false;
    }

    mDNS::Resource::TXT txt(*txtAnswer);
    const uint32_t id = txt[F("id")].toInt();
    if(id == 0 || id == system_get_chip_id()) {
        return false;
    }

    const String type = txt[F("type")];
    if(type.length() > 0 && type != F("host")) {
        return false;
    }

    String host = String(aAnswer->getName());
    host.replace(F(".local"), String());

    controllers_.addOrUpdate(id, host, String(aAnswer->getRecordString()), aAnswer->getTtl());
    return true;
}
