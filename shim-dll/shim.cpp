#include <iostream>
#include "../rpc-midl/rpc_retc.h"
#include <fstream>

#include "RzErrors.h"
#include "RzChromaSDKTypes.h"
#include "RzChromaSDKDefines.h"

bool createUniqueEffectID(RZEFFECTID* guid) {
	return CoCreateGuid(guid) == S_OK;
}

#define RzApi extern "C" __declspec (dllexport)

bool m_Initialized = false;

RzApi RZRESULT Init();
RzApi RZRESULT UnInit();

RzApi RZRESULT CreateEffect(RZDEVICEID DeviceId, ChromaSDK::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
RzApi RZRESULT CreateKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
RzApi RZRESULT CreateMouseEffect(ChromaSDK::Mouse::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
RzApi RZRESULT CreateHeadsetEffect(ChromaSDK::Headset::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
RzApi RZRESULT SetEffect(RZEFFECTID EffectId);
RzApi RZRESULT DeleteEffect(RZEFFECTID EffectId);
RzApi RZRESULT DeleteEffect(RZEFFECTID EffectId);



//////////////////////////////////////////////////////////////////////////
// All functions below this line are not implemented.
RzApi RZRESULT CreateMousepadEffect(ChromaSDK::Mousepad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
RzApi RZRESULT CreateKeypadEffect(ChromaSDK::Keypad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId);
RzApi RZRESULT QueryDevice(RZDEVICEID DeviceId, ChromaSDK::DEVICE_INFO_TYPE &DeviceInfo);

RzApi RZRESULT RegisterEventNotification(HWND hWnd);
RzApi RZRESULT UnregisterEventNotification();
//////////////////////////////////////////////////////////////////////////

//#define LOGE(x) m_logOutputStream << "ERRO : " << "[" << __FILE__ << "][" << __FUNCTION__ << "][Line " << __LINE__ << "] " << x << std::endl; #todo rpc call to server for logging
#define LOGE(x) NULL

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD /*callReason*/, LPVOID /*lpReserved*/) {
	return TRUE;
}

RPC_STATUS status;
CONTEXT_HANDLE hContext;
handle_t hRetcBinding = nullptr;

RZRESULT Init() {
	if (m_Initialized)
		return RZRESULT_ALREADY_INITIALIZED;

	RPC_WSTR szStringBinding = nullptr;
	status = RpcStringBindingCompose(nullptr, (RPC_WSTR)L"ncalrpc", nullptr, (RPC_WSTR)L"[retc-rpc]", nullptr, &szStringBinding);

	if (status)
		return RZRESULT_SERVICE_NOT_ACTIVE;

	status = RpcBindingFromStringBinding(szStringBinding, &hRetcBinding);

	if (status) {
		RpcStringFree(&szStringBinding);
		return RZRESULT_SERVICE_NOT_ACTIVE;
	}

	status = RpcStringFree(&szStringBinding);

	if (status)
		return RZRESULT_SERVICE_NOT_ACTIVE;

	status = RpcEpResolveBinding(hRetcBinding, rpc_retc_v2_1_c_ifspec);
	if (status) {
		LOGE("Server not running please start it!");
		return RZRESULT_SERVICE_NOT_ACTIVE;
	}

	RpcTryExcept {
			hContext = initialize(hRetcBinding);
			if (hContext == nullptr)
				return RZRESULT_SERVICE_NOT_ACTIVE;

			m_Initialized = true;
			return RZRESULT_SUCCESS;
		}
		RpcExcept(1) {
			LOGE("Connect call failed code: " << RpcExceptionCode());
			return RZRESULT_SERVICE_NOT_ACTIVE;
		}
		RpcEndExcept
}

RZRESULT UnInit() {
	if (!m_Initialized)
		return RZRESULT_NOT_VALID_STATE;

		RpcTryExcept {
			disconnect(&hContext);
		}
		RpcExcept(1) {
			LOGE("Disconnect call failed code: " << RpcExceptionCode());
			return RZRESULT_NOT_VALID_STATE;
		}
		RpcEndExcept

	status = RpcBindingFree(&hRetcBinding);

	m_Initialized = false;

	if (status)
		return RZRESULT_NOT_VALID_STATE;

	return RZRESULT_SUCCESS;
}

const unsigned long genericEffectsizeLookupArray[DEVICE_TYPE_RETC::KEYPAD + 1][ChromaSDK::CHROMA_RESERVED - 1] = {
	{ sizeof(ChromaSDK::Keyboard::WAVE_EFFECT_TYPE), 0, sizeof(ChromaSDK::Keyboard::BREATHING_EFFECT_TYPE),0 , sizeof(ChromaSDK::Keyboard::REACTIVE_EFFECT_TYPE), sizeof(ChromaSDK::Keyboard::STATIC_EFFECT_TYPE), sizeof(ChromaSDK::Keyboard::CUSTOM_EFFECT_TYPE) },
	{ sizeof(ChromaSDK::Mouse::WAVE_EFFECT_TYPE), sizeof(ChromaSDK::Mouse::SPECTRUMCYCLING_EFFECT_TYPE), sizeof(ChromaSDK::Mouse::BREATHING_EFFECT_TYPE),sizeof(ChromaSDK::Mouse::BLINKING_EFFECT_TYPE), sizeof(ChromaSDK::Mouse::REACTIVE_EFFECT_TYPE),	sizeof(ChromaSDK::Mouse::STATIC_EFFECT_TYPE), sizeof(ChromaSDK::Mouse::CUSTOM_EFFECT_TYPE) },
	{ 0, sizeof(ChromaSDK::SPECTRUMCYCLING_EFFECT_TYPE), sizeof(ChromaSDK::Headset::BREATHING_EFFECT_TYPE), 0, 0, sizeof(ChromaSDK::Headset::STATIC_EFFECT_TYPE), sizeof(ChromaSDK::Headset::CUSTOM_EFFECT_TYPE) },
	{ 0 },
	{ 0 }
};

RZRESULT CreateEffect(RZDEVICEID DeviceId, ChromaSDK::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId) {
	bool bStoreEffect = (pEffectId == nullptr) ? false : true;

	unsigned long iSize = 0;
	DEVICE_TYPE_RETC deviceType;

	if (DeviceId == ChromaSDK::BLACKWIDOW_CHROMA || DeviceId == ChromaSDK::BLACKWIDOW_CHROMA_TE ||
		DeviceId == ChromaSDK::BLACKWIDOW_X_CHROMA || DeviceId == ChromaSDK::DEATHSTALKER_CHROMA ||
		DeviceId == ChromaSDK::OVERWATCH_KEYBOARD || DeviceId == ChromaSDK::BLACKWIDOW_X_TE_CHROMA ||
		DeviceId == ChromaSDK::ORNATA_CHROMA || DeviceId == ChromaSDK::BLADE_STEALTH || DeviceId == ChromaSDK::BLADE)
	{
		deviceType = KEYBOARD;
	}
	else if (DeviceId == ChromaSDK::DEATHADDER_CHROMA || DeviceId == ChromaSDK::MAMBA_CHROMA_TE ||
		DeviceId == ChromaSDK::DIAMONDBACK_CHROMA || DeviceId == ChromaSDK::MAMBA_CHROMA ||
		DeviceId == ChromaSDK::NAGA_EPIC_CHROMA || DeviceId == ChromaSDK::NAGA_CHROMA ||
		DeviceId == ChromaSDK::OROCHI_CHROMA || DeviceId == ChromaSDK::NAGA_HEX_CHROMA) 
	{
		deviceType = MOUSE;
	}
	else if (DeviceId == ChromaSDK::KRAKEN71_CHROMA || DeviceId == ChromaSDK::MANOWAR_CHROMA ||	DeviceId == ChromaSDK::FIREFLY_CHROMA)
	{
		deviceType = HEADSET;
	}
	else {
		return RZRESULT_DEVICE_NOT_AVAILABLE;
	}

	if (Effect != ChromaSDK::CHROMA_NONE)
		iSize = genericEffectsizeLookupArray[deviceType][Effect-1];

	RZEFFECTID newEffectID;
	createUniqueEffectID(&newEffectID);

	RpcTryExcept{
		RZRESULT res = CreateEffectGeneric(DeviceId, deviceType, Effect, iSize, reinterpret_cast<char*>(pParam), newEffectID, bStoreEffect, hContext);
		if (res == RZRESULT_SUCCESS && bStoreEffect)
			*pEffectId = newEffectID;

		return res;
	}
	RpcExcept(1) {
		LOGE("CreateEffect call failed code: " << RpcExceptionCode());
		return RZRESULT_NOT_VALID_STATE;
	}
	RpcEndExcept
}

RZRESULT CreateKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId) {
	using namespace ChromaSDK::Keyboard;

	bool bStoreEffect = (pEffectId == nullptr) ? false : true;

	unsigned long iSize = 0;
	switch (Effect) {
	case CHROMA_STATIC:
		iSize = sizeof(ChromaSDK::Keyboard::STATIC_EFFECT_TYPE);
		break;
	case CHROMA_NONE:
		break;

	case CHROMA_CUSTOM:
		iSize = sizeof(ChromaSDK::Keyboard::CUSTOM_EFFECT_TYPE);
		break;

	case CHROMA_CUSTOM_KEY:
		iSize = sizeof(ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE);
		break;

	default:
		return RZRESULT_NOT_SUPPORTED;
	}

	if (!bStoreEffect) {
		RpcTryExcept {
				return playKeyboardEffect(Effect, iSize, reinterpret_cast<char*>(pParam), hContext);
			}
			RpcExcept(1) {
				LOGE("call failed code: " << RpcExceptionCode());
				return RZRESULT_NOT_VALID_STATE;
			}
			RpcEndExcept
	}

	RZEFFECTID newEffectID;
	createUniqueEffectID(&newEffectID);

	RpcTryExcept {
			RZRESULT res = createEffectInternal(Effect, KEYBOARD, iSize, reinterpret_cast<char*>(pParam), newEffectID, hContext);
			if (res == RZRESULT_SUCCESS)
				*pEffectId = newEffectID;

			return res;
		}
		RpcExcept(1) {
			LOGE("call failed code: " << RpcExceptionCode());
			return RZRESULT_NOT_VALID_STATE;
		}
		RpcEndExcept
}

RZRESULT CreateMouseEffect(ChromaSDK::Mouse::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId) {
	using namespace ChromaSDK::Mouse;

	bool bStoreEffect = (pEffectId == nullptr) ? false : true;

	unsigned long iSize = 0;
	switch (Effect) {
	case CHROMA_STATIC:
		iSize = sizeof(STATIC_EFFECT_TYPE);
		break;
	case CHROMA_NONE:
		break;
	case CHROMA_CUSTOM:
		iSize = sizeof(CUSTOM_EFFECT_TYPE);
		break;
	case CHROMA_CUSTOM2:
		iSize = sizeof(CUSTOM_EFFECT_TYPE2);
		break;
	default:
		return RZRESULT_NOT_SUPPORTED;
	}

	if (!bStoreEffect) {
		RpcTryExcept{
			return playMouseEffect(Effect, iSize, reinterpret_cast<char*>(pParam), hContext);
		}
			RpcExcept(1) {
			LOGE("call failed code: " << RpcExceptionCode());
			return RZRESULT_NOT_VALID_STATE;
		}
		RpcEndExcept
	}

	RZEFFECTID newEffectID;
	createUniqueEffectID(&newEffectID);

	RpcTryExcept{
		RZRESULT res = createEffectInternal(Effect, MOUSE, iSize, reinterpret_cast<char*>(pParam), newEffectID, hContext);
	if (res == RZRESULT_SUCCESS)
		*pEffectId = newEffectID;

	return res;
	}
		RpcExcept(1) {
		LOGE("call failed code: " << RpcExceptionCode());
		return RZRESULT_NOT_VALID_STATE;
	}
	RpcEndExcept
}


RZRESULT CreateHeadsetEffect(ChromaSDK::Headset::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId) {
	using namespace ChromaSDK::Headset;

	bool bStoreEffect = (pEffectId == nullptr) ? false : true;

	unsigned long iSize = 0;
	switch (Effect) {
	case CHROMA_STATIC:
		iSize = sizeof(STATIC_EFFECT_TYPE);
		break;
	case CHROMA_NONE:
		break;
	case CHROMA_CUSTOM:
		iSize = sizeof(CUSTOM_EFFECT_TYPE);
		break;
	default:
		return RZRESULT_NOT_SUPPORTED;
	}

	if (!bStoreEffect) {
		RpcTryExcept{
			return playHeadsetEffect(Effect, iSize, reinterpret_cast<char*>(pParam), hContext);
		}
			RpcExcept(1) {
			LOGE("call failed code: " << RpcExceptionCode());
			return RZRESULT_NOT_VALID_STATE;
		}
		RpcEndExcept
	}

	RZEFFECTID newEffectID;
	createUniqueEffectID(&newEffectID);

	RpcTryExcept{
		RZRESULT res = createEffectInternal(Effect, HEADSET, iSize, reinterpret_cast<char*>(pParam), newEffectID, hContext);
	if (res == RZRESULT_SUCCESS)
		*pEffectId = newEffectID;

	return res;
	}
	RpcExcept(1) {
		LOGE("call failed code: " << RpcExceptionCode());
		return RZRESULT_NOT_VALID_STATE;
	}
	RpcEndExcept
}

RZRESULT CreateMousepadEffect(ChromaSDK::Mousepad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId) {
	return RZRESULT_SUCCESS;
}

RZRESULT CreateKeypadEffect(ChromaSDK::Keypad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID *pEffectId) {
	return RZRESULT_SUCCESS;
}

RZRESULT SetEffect(RZEFFECTID EffectId) {
		RpcTryExcept {
			return setEffect(EffectId, hContext);
		}
		RpcExcept(1) {
			LOGE("call failed code: " << RpcExceptionCode());
			return RZRESULT_NOT_VALID_STATE;
		}
		RpcEndExcept
}

RZRESULT DeleteEffect(RZEFFECTID EffectId) {
		RpcTryExcept {
			return deleteEffect(EffectId, hContext);
		}
		RpcExcept(1) {
			LOGE("Runtime reported exception");
			return RZRESULT_NOT_VALID_STATE;
		}
		RpcEndExcept
}

RZRESULT QueryDevice(RZDEVICEID DeviceId, ChromaSDK::DEVICE_INFO_TYPE &DeviceInfo) {
	return RZRESULT_SUCCESS; //#todo add method
}

RZRESULT RegisterEventNotification(HWND hWnd) {
	return RZRESULT_SUCCESS; //#todo add method
}

RZRESULT UnregisterEventNotification() {
	return RZRESULT_SUCCESS; //#todo add method
}


void* __RPC_USER midl_user_allocate(size_t size) {
	return malloc(size);
}

void __RPC_USER midl_user_free(void* p) {
	free(p);
}
