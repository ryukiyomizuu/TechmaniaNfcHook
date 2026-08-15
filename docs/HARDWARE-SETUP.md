# AIC Pico hardware setup

The plugin uses the AIC Pico CardIO HID endpoint directly. The two serial ports
shown by Windows (`AIC Pico CLI Port` and `AIC Pico AIME Port`) are not used by
this DLL.

Required CardIO identity:

- Vendor ID: `0xCAFF`
- Product ID: `0x400E`
- Usage page: `0xFFCA`
- Usage: `0x0001`
- Minimum input report length: 9 bytes

Supported card reports:

- report ID `1`: FeliCa / amusement IC identity
- report ID `2`: MIFARE Ultralight / NTAG identity
- all-zero identity: card removed

Configure and verify the board with the upstream AIC Pico configurator and
firmware documentation. After configuration, close any application that holds
the CardIO HID interface and run `TechmaniaNfcProbe.exe --seconds 30`.

The DLL does not speak the serial AIME protocol and does not depend on COM port
numbers. Reassigning the CLI/AIME COM numbers therefore does not affect it.

