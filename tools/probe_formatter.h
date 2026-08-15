#pragma once

#include "techmania_nfc_hook.h"

#include <string>

namespace techmania::nfc::probe {

std::string format_event(const tm_nfc_event& event);

}
