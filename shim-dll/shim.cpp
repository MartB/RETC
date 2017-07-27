// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <iostream>
#include "../rpc-midl/rpc_retc.h"
#include "../server-exe/commonData.h"
#include "RzErrors.h"
#include "RzChromaSDKTypes.h"
#include "RzChromaSDKDefines.h"
using namespace ChromaSDK;

#define RzApi extern "C" __declspec (dllexport)

RzApi RZRESULT Init();
RzApi RZRESULT UnInit();

RzApi RZRESULT CreateEffect(RZDEVICEID DeviceId, EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
RzApi RZRESULT CreateKeyboardEffect(Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
RzApi RZRESULT CreateMouseEffect(Mouse::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
RzApi RZRESULT CreateHeadsetEffect(Headset::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
RzApi RZRESULT CreateMousepadEffect(Mousepad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
RzApi RZRESULT CreateKeypadEffect(Keypad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);

RzApi RZRESULT SetEffect(RZEFFECTID EffectId);
RzApi RZRESULT DeleteEffect(RZEFFECTID EffectId);

RzApi RZRESULT QueryDevice(RZDEVICEID DeviceId, DEVICE_INFO_TYPE& DeviceInfo);

//////////////////////////////////////////////////////////////////////////
// All functions below this line are not implemented.
RzApi RZRESULT RegisterEventNotification(HWND hWnd);
RzApi RZRESULT UnregisterEventNotification();
//////////////////////////////////////////////////////////////////////////

bool m_Initialized = false;
bool isInitialized() { return m_Initialized; }

CONTEXT_HANDLE rpcCTXHandle;
RETCClientConfig CONFIG;

handle_t hRetcBinding = nullptr;

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD /*callReason*/, LPVOID /*lpReserved*/) {
	return TRUE;
}

BOOL hasSupportFor(RETCDeviceType type) {
	if (!isInitialized() || type >= ESIZE || type < KEYBOARD) {
		return false;
	}

	return CONFIG.supportedDeviceTypes[type];
}

RZRESULT Init() {
	if (m_Initialized) {
		return RZRESULT_SERVICE_NOT_ACTIVE;
	}

	RPC_WSTR szStringBinding = nullptr;

	auto status = RpcStringBindingCompose(nullptr, RPC_WSTR(L"ncalrpc"), nullptr, RPC_WSTR(L"[retc-rpc]"), nullptr, &szStringBinding);

	if (status) {
		return RZRESULT_SERVICE_NOT_ACTIVE;
	}

	status = RpcBindingFromStringBinding(szStringBinding, &hRetcBinding);

	if (status) {
		RpcStringFree(&szStringBinding);
		return RZRESULT_SERVICE_NOT_ACTIVE;
	}

	status = RpcStringFree(&szStringBinding);

	if (status) {
		return RZRESULT_SERVICE_NOT_ACTIVE;
	}

	status = RpcEpResolveBinding(hRetcBinding, rpc_retc_v2_3_c_ifspec);
	if (status) {
		return RZRESULT_SERVICE_NOT_ACTIVE;
	}

	RpcTryExcept
		rpcCTXHandle = initialize(hRetcBinding, &CONFIG);
		if (rpcCTXHandle == nullptr) {
			return RZRESULT_SERVICE_NOT_ACTIVE;
		}

		m_Initialized = true;
		return RZRESULT_SUCCESS;
	RpcExcept(1)
		return RZRESULT_SERVICE_NOT_ACTIVE;
	RpcEndExcept
}

RZRESULT UnInit() {
	if (!m_Initialized) {
		return RZRESULT_NOT_VALID_STATE;
	}

	RpcTryExcept
		disconnect(&rpcCTXHandle);
	RpcExcept(1)
		return RZRESULT_NOT_VALID_STATE;
	RpcEndExcept

	auto status = RpcBindingFree(&hRetcBinding);

	m_Initialized = false;
	return (!status) ? RZRESULT_SUCCESS : RZRESULT_NOT_VALID_STATE;
}


RZRESULT sendEffect(RETCDeviceType deviceType, int effectID, PRZPARAM effectData, RZEFFECTID* pEffectId) {
	if (!hasSupportFor(deviceType)) {
		return RZRESULT_DEVICE_NOT_AVAILABLE;
	}

	auto isNoneEffect = (effectID == CHROMA_NONE);
	auto effectSize = isNoneEffect ? 0 : LookupArrays::effectSize[deviceType][effectID - 1];

	if (effectSize == 0 && !isNoneEffect) {
		return RZRESULT_NOT_SUPPORTED;
	}

	RpcTryExcept
		return playEffect(deviceType, effectID, pEffectId, effectSize, reinterpret_cast<char*>(effectData), rpcCTXHandle);
	RpcExcept(1)
		return RZRESULT_NOT_VALID_STATE;
	RpcEndExcept
}

RZRESULT CreateEffect(RZDEVICEID DeviceId, EFFECT_TYPE effectID, PRZPARAM pParam, RZEFFECTID* pEffectId) {
	auto deviceType = ESIZE;

	if (DeviceId == GUID_NULL) {
		deviceType = ALL;
	}
	else {
		for (int devID = KEYBOARD; devID < ALL; devID++) {
			if (DeviceId == CONFIG.emulatedDeviceIDS[devID]) {
				deviceType = static_cast<RETCDeviceType>(devID);
				break;
			}
		}
	}

	if (deviceType == ESIZE) {
		return RZRESULT_DEVICE_NOT_AVAILABLE;
	}

	auto realEffectID = (deviceType == ALL) ? effectID : LookupArrays::genericEffectType[deviceType][effectID];
	return sendEffect(deviceType, realEffectID, pParam, pEffectId);
}

RZRESULT CreateKeyboardEffect(Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId) {
	return sendEffect(KEYBOARD, Effect, pParam, pEffectId);
}

RZRESULT CreateMouseEffect(Mouse::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId) {
	return sendEffect(MOUSE, Effect, pParam, pEffectId);
}

RZRESULT CreateHeadsetEffect(Headset::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId) {
	return sendEffect(HEADSET, Effect, pParam, pEffectId);
}

RZRESULT CreateMousepadEffect(Mousepad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId) {
	return sendEffect(MOUSEPAD, Effect, pParam, pEffectId);
}

RZRESULT CreateKeypadEffect(Keypad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId) {
	return sendEffect(KEYPAD, Effect, pParam, pEffectId);
}


RZRESULT SetEffect(RZEFFECTID EffectId) {
	RpcTryExcept
		return setEffect(EffectId, rpcCTXHandle);
	RpcExcept(1)
		return RZRESULT_NOT_VALID_STATE;
	RpcEndExcept
}

RZRESULT DeleteEffect(RZEFFECTID EffectId) {
	RpcTryExcept
		return deleteEffect(EffectId, rpcCTXHandle);
	RpcExcept(1)
		return RZRESULT_NOT_VALID_STATE;
	RpcEndExcept
}

RZRESULT QueryDevice(RZDEVICEID DeviceID, DEVICE_INFO_TYPE& DeviceInfo) {
	for (int devID = KEYBOARD; devID < ALL; devID++) {
		if (DeviceID == CONFIG.emulatedDeviceIDS[devID]) {
			DeviceInfo.Connected = CONFIG.supportedDeviceTypes[devID];
			return RZRESULT_SUCCESS;
		}
	}

	DeviceInfo.Connected = FALSE;
	return RZRESULT_DEVICE_NOT_AVAILABLE;
}

RZRESULT RegisterEventNotification(HWND /*hWnd*/) {
	return RZRESULT_SUCCESS;
}

RZRESULT UnregisterEventNotification() {
	return RZRESULT_SUCCESS;
}


void* __RPC_USER midl_user_allocate(size_t size) {
	return malloc(size);
}

void __RPC_USER midl_user_free(void* p) {
	free(p);
}
