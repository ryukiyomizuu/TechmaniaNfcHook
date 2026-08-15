#pragma once

#include "native_event.h"

#include <functional>
#include <stop_token>

namespace techmania::nfc {

class IEventSource {
public:
    using Sink = std::function<void(const NativeEvent&)>;

    virtual ~IEventSource() = default;
    virtual void run(std::stop_token stop, const Sink& sink) = 0;
};

}
