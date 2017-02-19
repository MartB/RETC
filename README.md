# RETC

RETC is a custom dll shim and a server application designed to be the middleman between the Razer Chroma SDK and the Corsair CUE SDK.

# Usage:

- Copy RzChromaSDK[64].dll into either C:\Windows\System32 or the game client directory.
- Before you start any games start retc-rpc-server[-64].exe and leave the console window open.

# Supported features:
Only the following types of effects are supported as of now:
```
Keyboards:
    CHROMA_NONE, CHROMA_STATIC, CHROMA_CUSTOM, CHROMA_CUSTOM_KEY
Mice:
    none
Headsets:
    none
Mousepads
    none
```

### Which games are supported?
Pretty much every game that uses the Razer Chroma SDK is supported but keep in mind that some effects might not work correctly.

A small list of software that i tested with my K70 RGB
```
Overwatch
RazerChromaSDKSample
```

# FAQ:
### My effects dont work, what should i do ?
Did you start the Corsair Utility Engine ?
Did you enable the SDK in Corsair CUE ?
![cue-sdk-settings](http://i.imgur.com/c7d7hLR.png)
Read the `retc-server.log` it should help you with your problems.
The simple turning it off and on again approach will also work wonders most of the time if something fails.
