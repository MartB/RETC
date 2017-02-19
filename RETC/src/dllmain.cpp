#include "stdafx.h"
#include "RzChromaSDKDefines.h"
#include "RzErrors.h"
#include "RzChromaSDKTypes.h"
#include "razerConversionLayer.h"

#define RzApi extern "C" __declspec (dllexport)

RzApi RZRESULT Init();
RzApi RZRESULT UnInit();
RzApi RZRESULT CreateKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
RzApi RZRESULT SetEffect(RZEFFECTID EffectId);
RzApi RZRESULT DeleteEffect(RZEFFECTID EffectId);

//////////////////////////////////////////////////////////////////////////
// All functions below this line are not implemented.
RzApi RZRESULT CreateEffect(RZDEVICEID DeviceId, ChromaSDK::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
RzApi RZRESULT CreateHeadsetEffect(ChromaSDK::Headset::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
RzApi RZRESULT CreateMousepadEffect(ChromaSDK::Mousepad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);

RzApi RZRESULT CreateMouseEffect(ChromaSDK::Mouse::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
RzApi RZRESULT CreateKeypadEffect(ChromaSDK::Keypad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);

RzApi RZRESULT QueryDevice(RZDEVICEID DeviceId, ChromaSDK::DEVICE_INFO_TYPE &DeviceInfo);

RzApi RZRESULT RegisterEventNotification(HWND hWnd);
RzApi RZRESULT UnregisterEventNotification();
// Not implemented end
//////////////////////////////////////////////////////////////////////////

static BOOL WINAPI consoleController(DWORD dwCtrlEvent) { return dwCtrlEvent == CTRL_C_EVENT; }

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  callReason, LPVOID lpReserved)
{
	switch (callReason)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

razerConversionLayer layer;

RZRESULT Init()
{	
	LOGD("");
	if (!layer.connect())
		return RZRESULT_INVALID;

	return RZRESULT_SUCCESS;
}


RZRESULT UnInit()
{
	LOGD("");
	layer.destroy();
	return RZRESULT_SUCCESS;
}

RZRESULT CreateKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId)
{
	LOGD("Effect:"+ ptrStr(Effect)+ "pParam:"+ ptrStr(pParam) + "pEffectID" + ((pEffectId != NULL) ? guidToString(*pEffectId) : "none!"));
	if (!layer.createKeyboardEffect(Effect, pParam, pEffectId))
		return RZRESULT_INVALID;

	return RZRESULT_SUCCESS;
}

RZRESULT SetEffect(RZEFFECTID EffectId)
{
	LOGD(guidToString(EffectId));
	if (!layer.setActiveEffect(EffectId))
		return RZRESULT_INVALID;

	return RZRESULT_SUCCESS;
}

RZRESULT DeleteEffect(RZEFFECTID EffectId)
{
	LOGD(guidToString(EffectId));
	if (!layer.deleteEffect(EffectId))
		return RZRESULT_INVALID;

	return RZRESULT_SUCCESS;
}


RZRESULT CreateEffect(RZDEVICEID DeviceId, ChromaSDK::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId)
{
	//#todo
	return RZRESULT_SUCCESS;
}

RZRESULT CreateHeadsetEffect(ChromaSDK::Headset::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId)
{
	//#todo
	return RZRESULT_SUCCESS;
}

RZRESULT CreateMousepadEffect(ChromaSDK::Mousepad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId)
{
	//#todo
	return RZRESULT_SUCCESS;
}

RZRESULT CreateMouseEffect(ChromaSDK::Mouse::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId)
{
	//#todo
	return RZRESULT_SUCCESS;
}

RZRESULT CreateKeypadEffect(ChromaSDK::Keypad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId)
{
	//#todo
	return RZRESULT_SUCCESS;
}

RZRESULT QueryDevice(RZDEVICEID DeviceId, ChromaSDK::DEVICE_INFO_TYPE &DeviceInfo)
{
	//#todo
	return RZRESULT_SUCCESS;
}

RZRESULT RegisterEventNotification(HWND hWnd) 
{
	//#todo
	return RZRESULT_SUCCESS;
}

RZRESULT UnregisterEventNotification()
{
	//#todo
	return RZRESULT_SUCCESS;
}