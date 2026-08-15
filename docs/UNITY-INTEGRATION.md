# Unity integration

## Install

1. Copy `TechmaniaNfcHook.dll` to `Assets/Plugins/x86_64/`.
2. In the Unity plugin importer, enable Editor and Standalone for Windows x86_64
   only. Do not enable x86 or non-Windows targets.
3. Copy the two files in `unity/Runtime/` into a runtime scripts folder.
4. Create one `NfcReaderService` in the process-wide session/profile owner.
5. Call `Start()` after profile storage initializes, call `Poll()` from bounded
   login/session polling, and call `Dispose()` on application quit.

```csharp
private static NfcReaderService reader;

public static void InitializeReaders()
{
    reader = new NfcReaderService();
    reader.Start(); // false means NFC unavailable; do not disable USB/Guest
}

public static void PumpReader()
{
    reader?.Poll(16);
    while (reader != null && reader.TryDequeue(out var observation))
    {
        // Feed the profile/session coordinator. Do not log CredentialKey.
    }
}

public static void ShutdownReaders()
{
    reader?.Dispose();
    reader = null;
}
```

## Profile contract

Persist a stable profile ID and a list of credential aliases. Do not make the
physical card ID the profile's primary key. This permits USB + NFC linking and
maps cleanly to a future API-backed profile service.

The deployed TECHMANIA integration exposes:

- `pollForCredential`
- `pendingCredentialKind`
- `loginWithPendingCredential`
- `createProfileWithPendingCredential`
- `beginCredentialLink`
- `linkPendingCredential`
- `cancelCredentialLink`
- `credentialLinkStatus`
- `sessionCredentialKind`
- `hasSessionCard` / `pollSessionCardPresence`

## Deployment helper

```powershell
./scripts/deploy-techmania.ps1 -ProjectPath "C:\path\to\TECHMANIA"
```

The helper validates both source and destination and copies only the x64 DLL.

