#include <iostream>
#include "../rpc-midl/rpc_retc.h"
#include "map"
#include <vector>
#include <fstream>

#include "razerKeysToCorsair.h"
#include <RzErrors.h>
#include "cueProxy.h"

inline const std::string guidToString(REFGUID guid) {
	char szGuid[40] = {0};
	sprintf_s(szGuid, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
	return szGuid;
}

inline std::ostream& operator<<(std::ostream& os, REFGUID guid) {

	os << std::uppercase;
	os.width(8);
	os << guidToString(guid).c_str();
	os << std::nouppercase;
	return os;
}

std::ofstream m_logOutputStream = std::ofstream("retc-server.log", std::ofstream::out);
#define LOGE(x) m_logOutputStream << "ERRO : " << "[" << __FILE__ << "][" << __FUNCTION__ << "][Line " << __LINE__ << "] " << x << std::endl;
#define LOGI(x) m_logOutputStream << "INFO : " << "[" << __FILE__ << "][" << __FUNCTION__ << "][Line " << __LINE__ << "] " << x << std::endl;

#ifdef _DEBUG
#define LOGD(x) m_logOutputStream << "DEBU : " << "[" << __FILE__ << "][" << __FUNCTION__ << "][Line " << __LINE__ << "] " << x << std::endl;
#else
#define LOGD(x) NULL
#endif

// Custom compare function for GUIDs
struct RZEFFECTIDCmp {
	bool operator()(const GUID& Left, const GUID& Right) const {
		return memcmp(&Left, &Right, sizeof(Right)) < 0;
	}
};

typedef struct effectData {
	EFFECT_TYPE_RETC id;
	long size;
	char* data;
} effectData;

cueProxy m_CueProxy; // This is required to close the sdk connection again.

std::map<RZEFFECTID, effectData, RZEFFECTIDCmp> m_Effects;
bool m_bInitialized = false;

void reset() {
	LOGD("application logic reset");

	size_t effectCount = m_Effects.size();
	auto itr = m_Effects.begin();
	while (itr != m_Effects.end()) {
		delete itr->second.data;
		m_Effects.erase(itr++);
	}
	m_Effects.clear();

	LOGD("erased " << effectCount << " effects from map.");
	m_bInitialized = false;
}

CONTEXT_HANDLE initialize(handle_t hBinding) {
	// We only allow 1 session on this server!
	if (m_bInitialized) {
		LOGE("Only one session supported! Disconnected...");
		return nullptr;
	}

	m_bInitialized = true;

	if (!m_CueProxy.Load()) {
		LOGE("Could not load library: " << CORSAIR_DLL_NAME << "code:" << GetLastError());
		return NULL;
	}

	m_CueProxy.CorsairPerformProtocolHandshake();

	LOGI("session connected");

	return "sth";
}

RZRESULT playKeyboardEffect(EFFECT_TYPE_RETC type, unsigned long lArraySize, char* achArray, CONTEXT_HANDLE hContext) {
	std::vector<CorsairLedColor> vec;
	vec.reserve(22 * 6);

	LOGD("type: " << type << " size: " << lArraySize);
	if (type == KEYBOARD_SOLID) {
		ChromaSDK::Keyboard::STATIC_EFFECT_TYPE custEffect = (lArraySize > 0) ? *(struct ChromaSDK::Keyboard::STATIC_EFFECT_TYPE*)achArray : ChromaSDK::Keyboard::STATIC_EFFECT_TYPE{0};

		COLORREF origColor = custEffect.Color;
		if (origColor == NULL)
			origColor = custEffect.Color;

		CorsairLedColor ledColor = convertLedColor(origColor);
		for (int row = 0; row < MAX_ROW; row++) {
			for (int col = 0; col < MAX_COLUMN; col++) {
				ledColor.ledId = findCorrespondingLed(row, col);
				if (ledColor.ledId != CLI_Invalid)
					vec.push_back(ledColor);
			}
		}
	}
	else if (type == KEYBOARD_DATA || type == KEYBOARD_NONE) {
		CUSTOM_EFFECT_TYPE test = (lArraySize > 0) ? *(struct ChromaSDK::Keyboard::CUSTOM_EFFECT_TYPE*)achArray : ChromaSDK::Keyboard::CUSTOM_EFFECT_TYPE{0};

		for (int row = 0; row < 6; row++) {
			for (int col = 0; col < 22; col++) {
				CorsairLedColor ledColor = convertLedColor(test.Color[row][col]);
				ledColor.ledId = findCorrespondingLed(row, col);
				if (ledColor.ledId != CLI_Invalid)
					vec.push_back(ledColor);
			}
		}
	}
	else if (type == KEYBOARD_DATA_AND_KEY) {
		CUSTOM_KEY_EFFECT_TYPE test = (lArraySize > 0) ? *(struct ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE*)achArray : ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE{0};

		for (int row = 0; row < 6; row++) {
			for (int col = 0; col < 22; col++) {
				COLORREF origColor = test.Key[row][col];
				if (origColor == NULL)
					origColor = test.Color[row][col];

				CorsairLedColor ledColor = convertLedColor(origColor);
				ledColor.ledId = findCorrespondingLed(row, col);

				if (ledColor.ledId != CLI_Invalid)
					vec.push_back(ledColor);
			}
		}
	}
	else {
		LOGE("unimplemented effect: " << type << "size: " << lArraySize);
		return RZRESULT_NOT_SUPPORTED;
	}

	m_CueProxy.CorsairSetLedsColors(vec.size(), vec.data());

	if (const auto error = m_CueProxy.CorsairGetLastError()) {
		LOGE("CorsairSetLedsColors failed type: " << type << "size: " << lArraySize << "msg: " << m_CueProxy.corsairErrorToString(error).c_str());
		return RZRESULT_FAILED;
	}

	return RZRESULT_SUCCESS;
}

RZRESULT createEffect(EFFECT_TYPE_RETC type, unsigned long lArraySize, char* achArray, RZEFFECTID test, CONTEXT_HANDLE hContext) {

	auto a = effectData();
	a.id = type;
	a.size = lArraySize;
	a.data = new char[lArraySize];
	memcpy(a.data, achArray, lArraySize);

	LOGD("type: " << type << "size: " << lArraySize << test);

	m_Effects.insert(std::make_pair(test, a));

	return RZRESULT_SUCCESS;
}

RZRESULT setEffect(RZEFFECTID test, CONTEXT_HANDLE hContext) {
	auto it = m_Effects.find(test);
	if (it == m_Effects.end()) {
		LOGE("effect not found guid: " << test);
		return RZRESULT_INVALID_PARAMETER;
	}

	LOGD("effect found guid: " << test);
	return playKeyboardEffect(it->second.id, (unsigned long)it->second.size, it->second.data, hContext);
}


RZRESULT deleteEffect(RZEFFECTID test, CONTEXT_HANDLE hContext) {
	auto it = m_Effects.find(test);
	if (it == m_Effects.end()) {
		LOGE("effect not found guid: " << test);
		return RZRESULT_INVALID_PARAMETER;
	}

	LOGD("effect deleted guid: " << test);
	delete it->second.data;
	m_Effects.erase(it);

	return RZRESULT_SUCCESS;
}


void disconnect(CONTEXT_HANDLE* phContext) {
	LOGI("session " << *phContext << " closed");
	reset();
	*phContext = nullptr;
	m_CueProxy.Unload();
}


RPC_STATUS CALLBACK SecurityCallback(RPC_IF_HANDLE hInterface, void* pBindingHandle) {
	if (m_bInitialized)
		return RPC_S_ACCESS_DENIED;

	return RPC_S_OK;
}

int main() {
#ifndef _DEBUG
	//auto myConsole = GetConsoleWindow();
	//ShowWindow(myConsole, 0);
#endif

#ifdef _DEBUG
	m_logOutputStream.basic_ios<char>::rdbuf(std::cout.rdbuf());
#endif

	RPC_STATUS status;

	status = RpcServerUseProtseqEp(
		(RPC_WSTR)L"ncalrpc",
		0, 
		(RPC_WSTR)L"[retc-rpc]", 
		NULL);

	if (status)
		exit(status);

	status = RpcServerRegisterIf2(
		rpc_retc_v0_0_s_ifspec, 
		NULL, 
		NULL,
		RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH,
		RPC_C_LISTEN_MAX_CALLS_DEFAULT,
		(unsigned)-1, 
		SecurityCallback);

	if (status)
		exit(status);

	status = RpcServerListen(
		1, 
		RPC_C_LISTEN_MAX_CALLS_DEFAULT, 
		FALSE);

	if (status)
		exit(status);
}

void* __RPC_USER midl_user_allocate(size_t size) {
	return malloc(size);
}

void __RPC_USER midl_user_free(void* p) {
	free(p);
}

void __RPC_USER CONTEXT_HANDLE_rundown(CONTEXT_HANDLE hContext) {
	disconnect(&hContext);
}