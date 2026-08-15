# Troubleshooting

## Probe never reports `reader connected`

- Confirm the AIC Pico CardIO HID interface exists. The two serial ports alone
  are not sufficient.
- Confirm VID `CAFF`, PID `400E`, usage page `FFCA`, usage `0001`.
- Close configurators or other readers that may hold the HID interface.
- Reconnect the board and rerun the probe.

## Unity says the DLL is missing

- File name must be `TechmaniaNfcHook.dll`.
- Place it under `Assets/Plugins/x86_64/` or beside the built player where
  Unity resolves native plugins.
- Confirm the plugin importer targets Windows x86_64.

## `BadImageFormatException`

The game and DLL architectures differ. This repository produces x64 only.

## ABI mismatch or missing entry point

The C# bridge and native DLL came from different revisions. Deploy the two
together and verify the five exports in `docs/ABI.md`.

## Game pauses when a card is inserted

Do not call any HID/serial read from Unity or Lua. Only `NfcReaderService.Poll`
belongs on Unity's thread; it drains an already-populated native queue. Check
that exactly one service instance owns the native runtime.

## Card removal logs the player out

This is a session-layer bug. The physical presence flag and authenticated
profile identity must be separate. Removal may gate the exit popup but must not
clear records, options, score routes, or the session profile.

## Avoid sensitive logs

The included probe redacts IDs. Do not add raw `credentialKey`, FeliCa IDm, or
MIFARE UID output to production logs. Log source/kind/state only.

