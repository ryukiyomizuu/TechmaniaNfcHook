# Theme hooks

Themes must not load `TechmaniaNfcHook.dll`, enumerate HID devices, read COM
ports, or receive raw card IDs. The theme boundary is `tm.profile`.

## Login loop

```lua
-- Poll from the Login scene only.
if tm.profile.pollForCredential() then
    local kind = tm.profile.pendingCredentialKind() -- "nfc" or "usb"
    showAuthenticationAnimation(kind)

    if tm.profile.loginWithPendingCredential() then
        showAuthenticatedProfile(tm.profile.currentProfile())
    else
        showFirstTimeNameEntry()
    end
end
```

For first-time registration:

```lua
if tm.profile.createProfileWithPendingCredential(djName) then
    showAuthenticatedProfile(tm.profile.currentProfile())
else
    showRetryMessage()
end
```

Do not add a second loading screen. Start the authentic rainbow authentication
animation once, resolve the pending credential during it, then transition to
the named/icon-equipped profile result.

## Link NFC and USB to one profile

Linking is an authenticated settings/profile action, never an automatic tap:

```lua
if tm.profile.beginCredentialLink() then
    showPresentAnotherCredentialPrompt()
end

if tm.profile.pollForCredential() then
    if tm.profile.linkPendingCredential() then
        showLinkComplete()
    else
        showLinkError(tm.profile.credentialLinkStatus())
    end
end

-- Back/cancel:
tm.profile.cancelCredentialLink()
```

Possible link statuses are `waiting`, `linked`, `already-linked`,
`owned-by-another-profile`, `save-failed`, and `cancelled`.

## Exit/removal loop

Authentication remains valid after removal. At All Results or the final exit
screen, show the remove-card prompt only while the session credential is still
physically present:

```lua
if tm.profile.hasSessionCard() and
   tm.profile.pollSessionCardPresence() then
    showRemoveCredentialPopup(tm.profile.sessionCredentialKind())
else
    leaveSession()
end
```

Never call `logout()` in response to physical removal. Logout is a cabinet
session transition after results/exit completes.

## Required theme states

- idle Login: prompt for arcade card or USB profile
- authenticating: one authentic rainbow/loading animation
- known profile: show saved name, icon, plate/pattern, and level
- unknown credential: name-entry flow, then create with pending credential
- reader unavailable: keep USB and Guest controls usable
- linking: explicit prompt, success/error/cancel
- exit: remove-card popup driven by physical presence, not authentication

