#include "event_queue.h"
#include "test_support.h"

#include <vector>

using techmania::nfc::EventQueue;
using techmania::nfc::NativeEvent;

TEST_CASE("queue preserves FIFO event order") {
    EventQueue queue(4);
    queue.push({.type = TM_NFC_EVENT_READER_CONNECTED});
    queue.push({.type = TM_NFC_EVENT_CARD_PRESENT});

    NativeEvent event;
    REQUIRE(queue.try_pop(event));
    REQUIRE(event.type == TM_NFC_EVENT_READER_CONNECTED);
    REQUIRE(queue.try_pop(event));
    REQUIRE(event.type == TM_NFC_EVENT_CARD_PRESENT);
    REQUIRE(!queue.try_pop(event));
}

TEST_CASE("queue stays bounded and retains newest card transition order") {
    EventQueue queue(3);
    queue.push({.type = TM_NFC_EVENT_READER_CONNECTED});
    queue.push({.type = TM_NFC_EVENT_ERROR});
    queue.push({.type = TM_NFC_EVENT_READER_DISCONNECTED});
    queue.push({.type = TM_NFC_EVENT_CARD_REMOVED});
    queue.push({.type = TM_NFC_EVENT_CARD_PRESENT});

    REQUIRE(queue.size() == 3);
    NativeEvent event;
    REQUIRE(queue.try_pop(event));
    REQUIRE(event.type == TM_NFC_EVENT_READER_DISCONNECTED);
    REQUIRE(queue.try_pop(event));
    REQUIRE(event.type == TM_NFC_EVENT_CARD_REMOVED);
    REQUIRE(queue.try_pop(event));
    REQUIRE(event.type == TM_NFC_EVENT_CARD_PRESENT);
}

TEST_CASE("duplicate adjacent reader state notifications are coalesced") {
    EventQueue queue(4);
    queue.push({.type = TM_NFC_EVENT_READER_CONNECTED});
    queue.push({.type = TM_NFC_EVENT_READER_CONNECTED});

    REQUIRE(queue.size() == 1);
}
