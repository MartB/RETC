// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <iostream>
#include "../rpc-midl/rpc_retc.h"
#include "../server-exe/commonData.h"
#include "RzErrors.h"
#include "RzChromaSDKTypes.h"
using namespace ChromaSDK;

#define RzApi extern "C" __declspec (dllexport)

RzApi RZRESULT Init();
RzApi RZRESULT UnInit();

RzApi RZRESULT CreateEffect(RZDEVICEID deviceId, EFFECT_TYPE effectId, PRZPARAM pParam, RZEFFECTID* pEffectId);
RzApi RZRESULT CreateKeyboardEffect(Keyboard::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
RzApi RZRESULT CreateMouseEffect(Mouse::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
RzApi RZRESULT CreateHeadsetEffect(Headset::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
RzApi RZRESULT CreateMousepadEffect(Mousepad::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
RzApi RZRESULT CreateKeypadEffect(Keypad::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
RzApi RZRESULT CreateChromaLinkEffect(ChromaLink::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId);

RzApi RZRESULT SetEffect(RZEFFECTID EffectId);
RzApi RZRESULT DeleteEffect(RZEFFECTID EffectId);

RzApi RZRESULT QueryDevice(RZDEVICEID DeviceId, DEVICE_INFO_TYPE& DeviceInfo);

//////////////////////////////////////////////////////////////////////////
// All functions in this block are not implemented.
RzApi RZRESULT RegisterEventNotification(HWND hWnd);
RzApi RZRESULT UnregisterEventNotification();
//////////////////////////////////////////////////////////////////////////

bool m_Initialized = false;
bool isInitialized() { return m_Initialized; }

CONTEXT_HANDLE rpcCTXHandle;
RETCClientConfig *CONFIG;

handle_t hRetcBinding = nullptr;

#define ABORT_IF_NOT_INITIALIZED if (!m_Initialized) { return RZRESULT_NOT_VALID_STATE; }

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD /*callReason*/, LPVOID /*lpReserved*/) {
	return TRUE;
}

BOOL hasSupportFor(const RETCDeviceType type) {
	if (!isInitialized() || type >= ESIZE) {
		return false;
	}

	return CONFIG->supportedDeviceTypes[type];
}

RZRESULT Init() {
	if (m_Initialized) {
		return RZRESULT_NOT_VALID_STATE;
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

	status = RpcEpResolveBinding(hRetcBinding, rpc_retc_v2_5_c_ifspec);
	if (status) {
		return RZRESULT_SERVICE_NOT_ACTIVE;
	}

	RpcTryExcept
		CONFIG = new RETCClientConfig;
		rpcCTXHandle = initialize(hRetcBinding, CONFIG);
		if (CONFIG == nullptr) {
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

	const auto status = RpcBindingFree(&hRetcBinding);
	m_Initialized = false;
	delete CONFIG;
	return (!status) ? RZRESULT_SUCCESS : RZRESULT_NOT_VALID_STATE;
}


RZRESULT sendEffect(const RETCDeviceType deviceType, const int effectID, const PRZPARAM effectData, RZEFFECTID* pEffectId) {
	ABORT_IF_NOT_INITIALIZED;

	if (!hasSupportFor(deviceType)) {
		return RZRESULT_DEVICE_NOT_AVAILABLE;
	}

	const auto isNoneEffect = (effectID == CHROMA_NONE);
	const auto effectSize = isNoneEffect ? 0 : LookupArrays::effectSize[deviceType][effectID - 1];

	if (effectSize == 0 && !isNoneEffect) {
		return RZRESULT_NOT_SUPPORTED;
	}

	RpcTryExcept
		return playEffect(deviceType, effectID, pEffectId, effectSize, reinterpret_cast<char*>(effectData), rpcCTXHandle);
	RpcExcept(1)
		return RZRESULT_NOT_VALID_STATE;
	RpcEndExcept
}

// ReSharper disable CppParameterMayBeConst
RZRESULT CreateEffect(RZDEVICEID deviceId, EFFECT_TYPE effectId, PRZPARAM pParam, RZEFFECTID* pEffectId) { //-V813
	ABORT_IF_NOT_INITIALIZED;

	auto deviceType = ESIZE;
	if (deviceId == GUID_NULL) {
		deviceType = ALL;
	}
	else {
		for (int devID = KEYBOARD; devID < ALL; devID++) {
			if (deviceId == CONFIG->emulatedDeviceIDS[devID]) {
				deviceType = static_cast<RETCDeviceType>(devID);
				break;
			}
		}
	}

	if (deviceType == ESIZE) {
		return RZRESULT_DEVICE_NOT_AVAILABLE;
	}

	const auto realEffectID = (deviceType == ALL) ? effectId : LookupArrays::genericEffectType[deviceType][effectId];
	return sendEffect(deviceType, realEffectID, pParam, pEffectId);
}

RZRESULT CreateKeyboardEffect(Keyboard::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId) {
	return sendEffect(KEYBOARD, effect, pParam, pEffectId);
}

RZRESULT CreateMouseEffect(Mouse::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId) {
	return sendEffect(MOUSE, effect, pParam, pEffectId);
}

RZRESULT CreateHeadsetEffect(Headset::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId) {
	return sendEffect(HEADSET, effect, pParam, pEffectId);
}

RZRESULT CreateMousepadEffect(Mousepad::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId) {
	return sendEffect(MOUSEPAD, effect, pParam, pEffectId);
}

RZRESULT CreateKeypadEffect(Keypad::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID* pEffectId) {
	return sendEffect(KEYPAD, effect, pParam, pEffectId);
}

RZRESULT CreateChromaLinkEffect(ChromaLink::EFFECT_TYPE effect, PRZPARAM pParam, RZEFFECTID * pEffectId) {
	return sendEffect(SYSTEM, effect, pParam, pEffectId);
}

RZRESULT SetEffect(RZEFFECTID EffectId) { //-V813
	ABORT_IF_NOT_INITIALIZED;

	RpcTryExcept
		return setEffect(EffectId, rpcCTXHandle);
	RpcExcept(1)
		return RZRESULT_NOT_VALID_STATE;
	RpcEndExcept
}

RZRESULT DeleteEffect(RZEFFECTID EffectId) { //-V813
	ABORT_IF_NOT_INITIALIZED;

	RpcTryExcept
		return deleteEffect(EffectId, rpcCTXHandle);
	RpcExcept(1)
		return RZRESULT_NOT_VALID_STATE;
	RpcEndExcept
}

RZRESULT QueryDevice(RZDEVICEID DeviceID, DEVICE_INFO_TYPE& DeviceInfo) { //-V813
	ABORT_IF_NOT_INITIALIZED;

	for (int devId = KEYBOARD; devId < ALL; devId++) {
		if (DeviceID == CONFIG->emulatedDeviceIDS[devId]) {
			DeviceInfo.Connected = CONFIG->supportedDeviceTypes[devId];
			return RZRESULT_SUCCESS;
		}
	}

	DeviceInfo.Connected = FALSE;
	return RZRESULT_DEVICE_NOT_AVAILABLE;
}

RZRESULT RegisterEventNotification(HWND /*hWnd*/) {
	ABORT_IF_NOT_INITIALIZED;

	return RZRESULT_SUCCESS;
}

RZRESULT UnregisterEventNotification() {
	ABORT_IF_NOT_INITIALIZED;

	return RZRESULT_SUCCESS;
}

// ReSharper restore CppParameterMayBeConst


void* __RPC_USER midl_user_allocate(const size_t size) {
	return malloc(size);
}

void __RPC_USER midl_user_free(void* p) {
	free(p);
}
