#pragma once

#include <Windows.h>
#include <string>

#include "CUESDK.h"

typedef bool (*CORSAIRSETLEDSCOLORS)(int, CorsairLedColor*);
typedef bool (*CORSAIRSETLEDSCOLORSASYNC)(int, CorsairLedColor*, void (*CallbackType)(void*, bool, CorsairError), void*);
typedef int (*CORSAIRGETDEVICECOUNT)();
typedef CorsairDeviceInfo*(*CORSAIRGETDEVICEINFO)(int);
typedef CorsairLedPositions*(*CORSAIRGETLEDPOSITIONS)();
typedef CorsairLedId (*CORSAIRGETLEDIDFORKEYNAME)(char);
typedef bool (*CORSAIRREQUESTCONTROL)(CorsairAccessMode);
typedef CorsairProtocolDetails (*CORSAIRPERFORMPROTOCOLHANDSHAKE)();
typedef CorsairError (*CORSAIRGETLASTERROR)();

#ifdef _WIN64
#define CORSAIR_DLL_NAME "CUESDK.x64_2013.dll"
#else
#define CORSAIR_DLL_NAME "CUESDK_2013.dll"
#endif

#define CORSAIR_DLL_SETLEDSCOLORS "CorsairSetLedsColors"
#define CORSAIR_DLL_SETLEDSCOLORSASYNC "CorsairSetLedsColorsAsync"
#define CORSAIR_DLL_GETDEVICECOUNT "CorsairGetDeviceCount"
#define CORSAIR_DLL_GETDEVICEINFO "CorsairGetDeviceInfo"
#define CORSAIR_DLL_GETLEDPOSITIONS "CorsairGetLedPositions"
#define CORSAIR_DLL_GETLEDIDFORKEYNAME "CorsairGetLedIdForKeyName"
#define CORSAIR_DLL_REQUESTCONTROL "CorsairRequestControl"
#define CORSAIR_DLL_PERFORMPROTOCOLHANDSHAKE "CorsairPerformProtocolHandshake"
#define CORSAIR_DLL_GETLASTERROR "CorsairGetLastError"


class cueProxy {

public:
	bool Load();
	bool Unload();
	bool isLoaded() { return hInstance != nullptr; }

	std::string corsairErrorToString(const CorsairError error);

	// CUESDK function declarations
	CORSAIRPERFORMPROTOCOLHANDSHAKE CorsairPerformProtocolHandshake;
	CORSAIRSETLEDSCOLORS CorsairSetLedsColors;
	CORSAIRSETLEDSCOLORSASYNC CorsairSetLedsColorsAsync;
	CORSAIRGETLEDPOSITIONS CorsairGetLedPositions;
	CORSAIRGETLEDIDFORKEYNAME CorsairGetLedIdForKeyName;
	CORSAIRREQUESTCONTROL CorsairRequestControl;
	CORSAIRGETLASTERROR CorsairGetLastError;
	CORSAIRGETDEVICECOUNT CorsairGetDeviceCount;
	CORSAIRGETDEVICEINFO CorsairGetDeviceInfo;

protected:
	bool ReleaseLibrary();

private:
	HINSTANCE hInstance = nullptr;

};