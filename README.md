## What is RETC
RETC is a custom server application with a dll shim for clients, designed to be the middleman between the Razer Chroma SDK and various output sdks. 
I believe in performance so RETC is designed to run with the smallest amount of overhead possible.
## Installation:
### Automatic
Just download the latest installer executable and follow the installation steps. 
You should have working effects afterwards, if not proceed to the troubleshooting section.

If you want to add a custom application see `Manual->Patching application`
### Manual
#### Patching application
Copy both `RzChromaSDK.dll` and `RzChromaSDK64.dll` into the folder of the Application you want to use e.g `C:\Program Files\Overwatch`.

**WARNING:
Never move these dlls to a global dll loading path like `C:\Windows\System32` or the retc-server.exe directory itself otherwise you will end up crashing the application.**
#### Preparing files
Download the latest release from github or compile it yourself.
Download and extract the corsair cue sdk from the [official corsair download page](http://downloads.corsair.com/download?item=Files/CUE/CUESDK_2.10.91.zip) 
 Create a new folder called `retc-server` this will be the place where you start RETC from.
##### The following steps depend on your systems architecture
i.e whether you are using a `64bit (x64, x86_64, AMD64, etc.)` or `32bit (x86, i386, etc)` system.
###### **64 Bit**
Put the extracted `CUESDK\bin\amd64\CUESDK.x64_2015.dll` into the `retc-server` folder you just created.
Copy the `retc-rpc-server-64.exe` from the github download into the same folder.

###### **32 Bit**
Put the extracted `CUESDK\bin\i386\CUESDK_2015.dll` into the `retc-server` folder you just created.
Copy the `retc-rpc-server.exe` from the github download into the same folder.
# Usage
**Choose one of the following sections depending on how you installed RETC.** 
### Automatic installation
The service should start right after the installation is done and should continue doing so on every boot.
However if thats not the case check your services within windows and make sure you find a **RETC** service thats set to autostart.

### Manual installation
Before you start any games run **either** `retc-rpc-server.exe` **or** `retc-rpc-server-64.exe`, depending on the system architecture you have, and **leave the console window open**.

# Supported Output SDKs:
##### Only 1 output SDK is supported per device type (Keyboard, etc.).
### Corsair
Only the effect types in the following list are supported.
```
Keyboards:
    CHROMA_NONE, CHROMA_STATIC, CHROMA_CUSTOM, CHROMA_CUSTOM_KEY
Mice:
    CHROMA_NONE, CHROMA_STATIC, CHROMA_CUSTOM, CHROMA_CUSTOM2
Headsets:
    CHROMA_NONE, CHROMA_STATIC, CHROMA_CUSTOM
Mousepads: (since sdk version 2015)
    CHROMA_NONE, CHROMA_STATIC, CHROMA_CUSTOM 
```
### Razer
**The razer sdk implementation is just forwarding to the real and installed sdk if available.**
The only functionality you lose is the specific device effects from *CreateEffect*. (no biggie)

### Which applications are supported?
Pretty much every application that uses the Razer Chroma SDK through the supplied dlls is supported.
Keep in mind that some (now deprecated) effects might not work correctly in some output sdks.

# Troubleshooting:
## Corsair Output SDK
##### Did you start the Corsair Utility Engine ?
##### Did you enable the SDK in Corsair CUE ?
![cue-sdk-settings](http://i.imgur.com/c7d7hLR.png)
## Razer Output SDK
##### Make sure Razer Synapse is installed properly and allows SDK access.
## General
Take a look at the `server.log` it can help with your problems.
### Service
Restart the RETC service from within `services.msc` (win+r and type it in).
Then select the RTEC service, rightclick it and hit restart.
## Found an issue ?
### Try to fix it yourself.
This is an opensource project if you think you can fix the issue yourself do so and shoot me a pull request.
### Create an Issue on github.
If nothing works for you just report the issue and let the community help you.
