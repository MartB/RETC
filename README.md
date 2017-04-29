# RETC

RETC is a custom dll shim and a server application designed to be the middleman between the Razer Chroma SDK and the Corsair CUE SDK.

# Usage:

- Copy `RzChromaSDK[64].dll` into either `C:\Windows\System32` or the game client directory.
- Download the corsair cue sdk from the [official corsair download page](http://downloads.corsair.com/download?item=Files/Gaming-Keyboards/CUESDK_2.4.67.zip) 
- Put the `CUESDK[.x64]_2015.dll` files from the extracted archive folder `CUESDK\bin\[i386,amd64]` into the same folder as retc-rpc-server[-64].exe
- Before you start any games run `retc-rpc-server[-64].exe` and leave the console window open.

# Supported features:
Only the following types of effects are supported in the Corsair output SDK as of now:
```
Keyboards:
    CHROMA_NONE, CHROMA_STATIC, CHROMA_CUSTOM, CHROMA_CUSTOM_KEY
Mice:
    CHROMA_NONE, CHROMA_STATIC, CHROMA_CUSTOM, CHROMA_CUSTOM2
Headsets:
    CHROMA_NONE, CHROMA_STATIC, CHROMA_CUSTOM
Mousepads:
    CHROMA_NONE, CHROMA_STATIC, CHROMA_CUSTOM (since sdk version 2015)
```

### Which games are supported?
Pretty much every game that uses the Razer Chroma SDK is supported but keep in mind that some effects might not work correctly.

A small list of software that i tested with my K70 RGB
```
Overwatch
RazerChromaSDKSample
KeyboardVisualizer (in razer only mode)
```

# FAQ:
### My effects dont work, what should i do ?
Did you start the Corsair Utility Engine ?
Did you enable the SDK in Corsair CUE ?
![cue-sdk-settings](http://i.imgur.com/c7d7hLR.png)
Read the `server.log` it should help you with your problems.
The simple turning it off and on again approach will also work wonders most of the time if something fails.
