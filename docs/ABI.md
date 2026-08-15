# Native ABI

ABI version: `0x00010000`  
Architecture: Windows x64  
Calling convention: C `__cdecl`

The public declaration is [techmania_nfc_hook.h](../include/techmania_nfc_hook.h).

## Exports

| Export | Purpose |
| --- | --- |
| `tm_nfc_get_abi_version` | Returns the ABI version before startup. |
| `tm_nfc_start` | Starts the single native reader worker. Idempotent. |
| `tm_nfc_stop` | Cancels reads, joins the worker, and clears runtime state. |
| `tm_nfc_poll` | Pops at most one queued event without blocking. |
| `tm_nfc_get_reader_state` | Returns stopped, searching, connected, or error. |

No other symbols are part of the supported interface.

## `tm_nfc_event`

The event is packed to one-byte alignment and is exactly 60 bytes:

```c
typedef struct tm_nfc_event {
    uint32_t struct_size;
    uint32_t abi_version;
    uint32_t sequence;
    int32_t event_type;
    int32_t card_kind;
    uint8_t card_id[8];
    int32_t error_code;
    uint8_t reserved[28];
} tm_nfc_event;
```

The caller sets `struct_size` and `abi_version` before every poll. A poll return
of `1` means an event was written, `0` means the queue is empty, and a negative
value is an ABI/argument/runtime error.

Card identity bytes are opaque. Consumers may encode them for lookup but must
not infer ownership, player name, or account data from them.

## Events

- `READER_CONNECTED`
- `READER_DISCONNECTED`
- `CARD_PRESENT`
- `CARD_REMOVED`
- `ERROR`

Sequence numbers let the managed side preserve event order. Repeated input
reports for a held card do not generate repeated Present events.

