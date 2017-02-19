#include <iostream>
#include "../rpc-midl/rpc_retc.h"
#include <fstream>

#include "RzErrors.h"
#include "RzChromaSDKTypes.h"


bool createUniqueEffectID(RZEFFECTID* guid) {
	return CoCreateGuid(guid) == S_OK;
}

#define RzApi extern "C" __declspec (dllexport)

bool m_Initialized = false;

RzApi RZRESULT Init();
RzApi RZRESULT UnInit();
RzApi RZRESULT CreateKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
RzApi RZRESULT SetEffect(RZEFFECTID EffectId);
RzApi RZRESULT DeleteEffect(RZEFFECTID EffectId);

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

	status = RpcEpResolveBinding(hRetcBinding, rpc_retc_v0_0_c_ifspec);
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
		return RZRESULT_SERVICE_NOT_ACTIVE;

		RpcTryExcept {
			disconnect(&hContext);
		}
		RpcExcept(1) {
			LOGE("Disconnect call failed code: " << RpcExceptionCode());
			return RZRESULT_SERVICE_NOT_ACTIVE;
		}
		RpcEndExcept

	status = RpcBindingFree(&hRetcBinding);

	m_Initialized = false;

	if (status)
		return RZRESULT_SERVICE_NOT_ACTIVE;

	return RZRESULT_SUCCESS;
}


RZRESULT CreateKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId) {

	bool bStoreEffect = (pEffectId == nullptr) ? false : true;

	EFFECT_TYPE_RETC internalType = EFFECT_TYPE_RETC::KEYBOARD_NONE;

	size_t iSize = 0;
	switch (Effect) {
	case ChromaSDK::Keyboard::CHROMA_STATIC:
		iSize = sizeof(ChromaSDK::Keyboard::STATIC_EFFECT_TYPE);
		internalType = EFFECT_TYPE_RETC::KEYBOARD_SOLID;
		break;
	case ChromaSDK::Keyboard::CHROMA_NONE:
		break;

	case ChromaSDK::Keyboard::CHROMA_CUSTOM:
		internalType = EFFECT_TYPE_RETC::KEYBOARD_DATA;
		iSize = sizeof(ChromaSDK::Keyboard::CUSTOM_EFFECT_TYPE);
		break;

	case ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY:
		internalType = EFFECT_TYPE_RETC::KEYBOARD_DATA_AND_KEY;
		iSize = sizeof(ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE);
		break;

	default:
		return RZRESULT_NOT_SUPPORTED;
	}


	if (!bStoreEffect) {
		RpcTryExcept {
				playKeyboardEffect(internalType, iSize, reinterpret_cast<char*>(pParam), hContext);
			}
			RpcExcept(1) {
				LOGE("call failed code: " << RpcExceptionCode());
				return RZRESULT_SERVICE_NOT_ACTIVE;
			}
			RpcEndExcept
		return RZRESULT_SUCCESS;
	}

	RZEFFECTID newEffectID;
	createUniqueEffectID(&newEffectID);

	RpcTryExcept {
			RZRESULT res = createEffect(internalType, iSize, reinterpret_cast<char*>(pParam), newEffectID, hContext);
			if (res == RZRESULT_SUCCESS)
				*pEffectId = newEffectID;

			return res;
		}
		RpcExcept(1) {
			LOGE("call failed code: " << RpcExceptionCode());
			return RZRESULT_SERVICE_NOT_ACTIVE;
		}
		RpcEndExcept
}

RZRESULT SetEffect(RZEFFECTID EffectId) {
		RpcTryExcept {
			return setEffect(EffectId, hContext);
		}
		RpcExcept(1) {
			LOGE("call failed code: " << RpcExceptionCode());
			return RZRESULT_SERVICE_NOT_ACTIVE;
		}
		RpcEndExcept
}

RZRESULT DeleteEffect(RZEFFECTID EffectId) {
		RpcTryExcept {
			return deleteEffect(EffectId, hContext);
		}
		RpcExcept(1) {
			LOGE("Runtime reported exception");
			return RZRESULT_SERVICE_NOT_ACTIVE;
		}
		RpcEndExcept
}


void* __RPC_USER midl_user_allocate(size_t size) {
	return malloc(size);
}

void __RPC_USER midl_user_free(void* p) {
	free(p);
}
