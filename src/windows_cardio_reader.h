#pragma once

#include "event_source.h"

namespace techmania::nfc {

class WindowsCardIoReader final : public IEventSource {
public:
    void run(std::stop_token stop, const Sink& sink) override;
};

}
