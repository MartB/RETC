#include <iostream>
#include "../rpc-midl/rpc_retc.h"
#include <unordered_map>
#include <vector>
#include <fstream>

#include "razerKeysToCorsair.h"
#include <RzErrors.h>
#include "cueProxy.h"
#include "server.h"
static std::ofstream m_logOutputStream = std::ofstream("retc-server.log", std::ofstream::out);
#define LOGE(x) m_logOutputStream << "ERRO : " << "[" << __FILE__ << "][" << __FUNCTION__ << "][Line " << __LINE__ << "] " << x << std::endl;
#define LOGI(x) m_logOutputStream << "INFO : " << "[" << __FILE__ << "][" << __FUNCTION__ << "][Line " << __LINE__ << "] " << x << std::endl;

#ifdef _DEBUG
#define LOGD(x) m_logOutputStream << "DEBU : " << "[" << __FILE__ << "][" << __FUNCTION__ << "][Line " << __LINE__ << "] " << x << std::endl;
#else
#define LOGD(x) NULL
#endif

static cueProxy m_CueProxy;

static std::unordered_map<RZEFFECTID, effectData> m_Effects;
static bool m_bInitialized = false;

static std::vector<CorsairLedId> m_availableKeyboardLeds;
static std::vector<CorsairLedId> m_availableMouseLeds;
static std::vector<CorsairLedId> m_availableHeadsetLeds;

/* Global vectors for performance reasons.
* Trivial destructors are guaranteed an O(1) complexity on .resize(0) 
* The capacity is not altered after the .resize() */
static std::vector<CorsairLedColor> m_keyboardColorVector;
static std::vector<CorsairLedColor> m_mouseColorVector;
static std::vector<CorsairLedColor> m_headsetColorVector;

typedef int(*playFunctionPointer)(int type, unsigned long lArraySize, char* achArray, CONTEXT_HANDLE hContext);
static playFunctionPointer deviceSpecificPlayFunctions[5];

void reset() {
	LOGD("application logic reset");

	// This is required to close the sdk connection again.
	m_CueProxy.Unload();

	auto itr = m_Effects.begin();
	while (itr != m_Effects.end()) {
		delete itr->second.data;
		m_Effects.erase(itr++);
	}
	m_Effects.clear();

	LOGD("erased " << m_Effects.size() << " effects from map.");

	clearandshrink(m_availableKeyboardLeds);
	clearandshrink(m_availableMouseLeds);
	clearandshrink(m_availableHeadsetLeds);
	clearandshrink(m_keyboardColorVector);
	clearandshrink(m_mouseColorVector);
	clearandshrink(m_headsetColorVector);

	m_bInitialized = false;
}

CONTEXT_HANDLE initialize(handle_t hBinding) {
	// We only allow 1 session on this server!
	if (m_bInitialized) {
		LOGE("Only one session supported! Disconnected...");
		return nullptr;
	}

	if (!m_CueProxy.Load()) {
		LOGE("Could not load library: " << CORSAIR_DLL_NAME << "code:" << GetLastError());
		return nullptr;
	}

	CorsairProtocolDetails hsDetails = m_CueProxy.CorsairPerformProtocolHandshake();
	if (hsDetails.breakingChanges == true) {
		LOGE("Could not perform handshake: Breaking changes between library versions please update!");
		return nullptr;
	}

	if (hsDetails.serverProtocolVersion == 0 || hsDetails.serverVersion == NULL) {
		LOGE("Could not perform handshake: CUE not running on your pc!");
		return nullptr;
	}

	int devCount = m_CueProxy.CorsairGetDeviceCount();
	if (devCount == 0) {
		LOGE("No supported devices found, aborting!");
		return nullptr;
	}

	int devicesWithLightingSupport = 0;
	for (int i = 0; i < devCount; i++) {
		const CorsairDeviceInfo *devInfo = m_CueProxy.CorsairGetDeviceInfo(i);
		if (devInfo->capsMask == CDC_Lighting) {
			devicesWithLightingSupport++;
			switch (devInfo->type) {
			case CDT_Mouse: {
				const auto &numberOfKeys = devInfo->physicalLayout - CPL_Zones1 + 1;
				m_availableMouseLeds.reserve(numberOfKeys);
				m_mouseColorVector.reserve(numberOfKeys);
				for (auto i = 0; i < numberOfKeys; i++) {
					auto ledId = static_cast<CorsairLedId>(CLM_1 + i);
					m_availableMouseLeds.push_back(ledId);
				}
			} break;
			case CDT_Keyboard: {
				const auto &ledPositions = m_CueProxy.CorsairGetLedPositions();

				if (!ledPositions)
					break;
				
				const auto &ledData = ledPositions->pLedPosition;
				const auto &ledCount = ledPositions->numberOfLed;
				m_keyboardColorVector.reserve(ledCount);
				m_availableKeyboardLeds.reserve(ledCount);
				for (auto i = 0; i < ledCount; i++) {
					auto ledId = ledData[i].ledId;
					m_availableKeyboardLeds.push_back(ledId);
				}
			} break;
			case CDT_Headset: {
				m_headsetColorVector.reserve(2);
				m_availableHeadsetLeds.push_back(CLH_LeftLogo);
				m_availableHeadsetLeds.push_back(CLH_RightLogo);
			} break;
			}
		}

		LOGI("Found device: " << devInfo->model << " type: "<< deviceTypeToString(devInfo->type) << " hasLighting: " << devInfo->capsMask << " physicalLayout: " << devInfo->physicalLayout);
	}

	if (devicesWithLightingSupport == 0) {
		LOGE("Non of your corsair devices have lighting support, aborting!");
		return nullptr;
	}

	m_bInitialized = true;

	LOGI("session connected");

	return "sth";
}


int playKeyboardEffect(int type, unsigned long lArraySize, char* achArray, CONTEXT_HANDLE hContext) {
	if (m_availableKeyboardLeds.size() == 0) {
		return RZRESULT_NOT_SUPPORTED;
	}

	using namespace ChromaSDK::Keyboard; // Important do not remove!

	LOGD("type: " << type << " size: " << lArraySize);
	int row;
	int col;

	if (type == CHROMA_STATIC) {
		const auto &custEffect = (lArraySize > 0) ? *(struct STATIC_EFFECT_TYPE*)achArray : STATIC_EFFECT_TYPE{0};
		auto ledColor = convertLedColor(custEffect.Color);

		for (const auto ledId : m_availableKeyboardLeds) {
			if (!findKeyboardLed(ledId, &row, &col))
				continue;

			ledColor.ledId = ledId;
			m_keyboardColorVector.push_back(ledColor);
		}
	}
	else if (type == CHROMA_CUSTOM || type == CHROMA_NONE) {
		const auto &custEffect = (lArraySize > 0) ? *(struct CUSTOM_EFFECT_TYPE*)achArray : CUSTOM_EFFECT_TYPE{ 0 };

		for (const auto ledId : m_availableKeyboardLeds) {
			if (!findKeyboardLed(ledId, &row, &col))
				continue;

			auto ledColor = convertLedColor(custEffect.Color[row][col]);
			ledColor.ledId = ledId;
			m_keyboardColorVector.push_back(ledColor);
		}
	}
	else if (type == CHROMA_CUSTOM_KEY) {
		const auto &custEffect = (lArraySize > 0) ? *(struct CUSTOM_KEY_EFFECT_TYPE*)achArray : CUSTOM_KEY_EFFECT_TYPE{0};

		for (const auto ledId : m_availableKeyboardLeds) {
			if (!findKeyboardLed(ledId, &row, &col))
				continue;

			auto origColor = custEffect.Key[row][col];
			if (origColor == NULL)
				origColor = custEffect.Color[row][col];

			auto ledColor = convertLedColor(origColor);
			ledColor.ledId = ledId;
			m_keyboardColorVector.push_back(ledColor);
		}
	}
	else {
		LOGE("unimplemented effect: " << type << "size: " << lArraySize);
		return RZRESULT_NOT_SUPPORTED;
	}

	m_CueProxy.CorsairSetLedsColors((int)m_keyboardColorVector.size(), m_keyboardColorVector.data());
	m_keyboardColorVector.resize(0);

	if (const auto error = m_CueProxy.CorsairGetLastError()) {
		LOGE("CorsairSetLedsColors failed type: " << type << "size: " << lArraySize << "msg: " << m_CueProxy.corsairErrorToString(error).c_str());
		return RZRESULT_NOT_SUPPORTED;
	}

	return RZRESULT_SUCCESS;
}

int playMouseEffect(int type, unsigned long lArraySize, char* achArray, CONTEXT_HANDLE hContext) {
	if (m_availableMouseLeds.size() == 0) {
		return RZRESULT_NOT_SUPPORTED;
	}

	using namespace ChromaSDK::Mouse; // Important do not remove!

	int row;
	int col;

	if (type == CHROMA_STATIC) {
		const auto &custEffect = (lArraySize > 0) ? *(struct STATIC_EFFECT_TYPE*)achArray : STATIC_EFFECT_TYPE{};
		auto ledColor = convertLedColor(custEffect.Color);

		if (custEffect.LEDId == RZLED_ALL) {
			for (const auto ledId : m_availableMouseLeds) {
				if (!findMouseLed(ledId, &row, &col))
					continue;

				ledColor.ledId = ledId;
				m_mouseColorVector.push_back(ledColor);
			}
		}
		else {
			ledColor.ledId = findMouseLed(custEffect.LEDId);

			if (ledColor.ledId != CLI_Invalid)
				m_mouseColorVector.push_back(ledColor);
		}
	}
	else if (type == CHROMA_CUSTOM || type == CHROMA_NONE) {
		const auto &custEffect = (lArraySize > 0) ? *(struct CUSTOM_EFFECT_TYPE*)achArray : CUSTOM_EFFECT_TYPE{ 0 };

		for (const auto ledId : m_availableMouseLeds) {
			int val = findMouseLed(ledId);
			if (val == -1)
				continue;
			
			auto ledColor = convertLedColor(custEffect.Color[val]);
			ledColor.ledId = ledId;
			m_mouseColorVector.push_back(ledColor);
		}
	}
	else if (type == CHROMA_CUSTOM2) {
		const auto &custEffect = (lArraySize > 0) ? *(struct CUSTOM_EFFECT_TYPE2*)achArray : CUSTOM_EFFECT_TYPE2{ 0 };

		for (const auto ledId : m_availableMouseLeds) {
			if (!findMouseLed(ledId, &row, &col))
				continue;

			auto ledColor = convertLedColor(custEffect.Color[row][col]);
			ledColor.ledId = ledId;

			m_mouseColorVector.push_back(ledColor);
		}
	}
	else {
		LOGE("unimplemented effect: " << type << "size: " << lArraySize);
		return RZRESULT_NOT_SUPPORTED;
	}

	m_CueProxy.CorsairSetLedsColors((int)m_mouseColorVector.size(), m_mouseColorVector.data());
	m_mouseColorVector.resize(0);

	if (const auto error = m_CueProxy.CorsairGetLastError()) {
		LOGE("CorsairSetLedsColors failed type: " << type << "size: " << lArraySize << "msg: " << m_CueProxy.corsairErrorToString(error).c_str());
		return RZRESULT_NOT_SUPPORTED;
	}

	return RZRESULT_SUCCESS;
}

int playHeadsetEffect(int type, unsigned long lArraySize, char* achArray, CONTEXT_HANDLE hContext) {
	if (m_availableHeadsetLeds.size() == 0) {
		return RZRESULT_NOT_SUPPORTED;
	}

	using namespace ChromaSDK::Headset; // Important do not remove!

	if (type == CHROMA_STATIC) {
		const auto &custEffect = (lArraySize > 0) ? *(struct STATIC_EFFECT_TYPE*)achArray : STATIC_EFFECT_TYPE{};
		CorsairLedColor ledColor = convertLedColor(custEffect.Color);

		for (const auto led : m_availableHeadsetLeds) {
			ledColor.ledId = led;
			m_headsetColorVector.push_back(ledColor);
		}
	}
	else if (type == CHROMA_CUSTOM || type == CHROMA_NONE) {
		const auto &custEffect = (lArraySize > 0) ? *(struct CUSTOM_EFFECT_TYPE*)achArray : CUSTOM_EFFECT_TYPE{ 0 };

		int val = -1;
		for (const auto led : m_availableHeadsetLeds) {
			int val = findHeadsetLed(led);
			if (val == -1)
				continue;

			auto ledColor = convertLedColor(custEffect.Color[val]);
			ledColor.ledId = led;
			m_headsetColorVector.push_back(ledColor);
		}
	}

	m_CueProxy.CorsairSetLedsColors((int)m_headsetColorVector.size(), m_headsetColorVector.data());
	m_headsetColorVector.resize(0);

	if (const auto error = m_CueProxy.CorsairGetLastError()) {
		LOGE("CorsairSetLedsColors failed type: " << type << "size: " << lArraySize << "msg: " << m_CueProxy.corsairErrorToString(error).c_str());
		return RZRESULT_NOT_SUPPORTED;
	}

	return RZRESULT_SUCCESS;
}

int playKeypadEffect(int type, unsigned long lArraySize, char* achArray, CONTEXT_HANDLE hContext) {
	return RZRESULT_NOT_SUPPORTED;
}

int playMousepadEffect(int type, unsigned long lArraySize, char* achArray, CONTEXT_HANDLE hContext) {
	return RZRESULT_NOT_SUPPORTED;
}

int CreateEffectGeneric(const RZDEVICEID DeviceId, DEVICE_TYPE_RETC deviceType, int Effect, unsigned long lArraySize, char* achArray, const RZEFFECTID pEffectId, boolean storeEffect, CONTEXT_HANDLE hContext) {

	//#todo config parameter we simply cant support more than 1 device per group with corsairs sdk
	if (deviceType == KEYBOARD && DeviceId != ChromaSDK::BLACKWIDOW_CHROMA) {
		return RZRESULT_DEVICE_NOT_AVAILABLE;
	} 

	if (deviceType == MOUSE && DeviceId != ChromaSDK::DEATHADDER_CHROMA) { // the mamba has side leds we cant use
		return RZRESULT_DEVICE_NOT_AVAILABLE;
	}

	if (deviceType == HEADSET && DeviceId != ChromaSDK::KRAKEN71_CHROMA) { // take the one with the most leds
		return RZRESULT_DEVICE_NOT_AVAILABLE;
	}

	if (Effect > 8) {
		LOGE("Invalid effect index: " << Effect);
		return RZRESULT_NOT_SUPPORTED;
	}

	int effectType = effectTypeLookupArray[deviceType][Effect];

	if (effectType == -1)
		return RZRESULT_NOT_SUPPORTED;
	
	if (!storeEffect) {
		return deviceSpecificPlayFunctions[deviceType](effectType, lArraySize, achArray, hContext);
	}

	return createEffectInternal(effectType, deviceType, lArraySize, achArray, pEffectId, hContext);
}

int createEffectInternal(int type, DEVICE_TYPE_RETC deviceType,  unsigned long lArraySize, char* achArray, RZEFFECTID test, CONTEXT_HANDLE hContext) {

	auto effData = effectData();
	effData.id = type;
	effData.deviceType = deviceType;
	effData.size = lArraySize;
	effData.data = new char[lArraySize];
	std::copy(achArray, achArray + lArraySize, effData.data);

	LOGD("type: " << type << "size: " << lArraySize << test);

	m_Effects.insert(std::make_pair(test, effData));

	return RZRESULT_SUCCESS;
}

int setEffect(const RZEFFECTID effID, CONTEXT_HANDLE hContext) {
	auto it = m_Effects.find(effID);
	if (it == m_Effects.end()) {
		LOGE("effect not found guid: " << effID);
		return RZRESULT_NOT_FOUND;
	}

	LOGD("effect found guid: " << effID);
	
	const effectData &tmp = it->second;

	return deviceSpecificPlayFunctions[tmp.deviceType](tmp.id, static_cast<unsigned long>(tmp.size), tmp.data, hContext);
}


int deleteEffect(const RZEFFECTID effID, CONTEXT_HANDLE hContext) {
	auto it = m_Effects.find(effID);
	if (it == m_Effects.end()) {
		LOGE("effect not found guid: " << effID);
		return RZRESULT_INVALID_PARAMETER;
	}

	LOGD("effect deleted guid: " << effID);
	delete it->second.data;
	m_Effects.erase(it);

	return RZRESULT_SUCCESS;
}


void disconnect(CONTEXT_HANDLE* phContext) {
	LOGI("session " << *phContext << " closed");
	reset();
	*phContext = nullptr;
}


RPC_STATUS CALLBACK SecurityCallback(RPC_IF_HANDLE hInterface, void* pBindingHandle) {
	return RPC_S_OK;
}

int main() {
	// redirect console output
	m_logOutputStream.basic_ios<char>::rdbuf(std::cout.rdbuf());

	deviceSpecificPlayFunctions[KEYBOARD] = (playFunctionPointer)playKeyboardEffect;
	deviceSpecificPlayFunctions[MOUSE] = (playFunctionPointer)playMouseEffect;
	deviceSpecificPlayFunctions[HEADSET] = (playFunctionPointer)playHeadsetEffect;
	deviceSpecificPlayFunctions[MOUSEPAD] = (playFunctionPointer)playMousepadEffect;
	deviceSpecificPlayFunctions[KEYPAD] = (playFunctionPointer)playKeypadEffect;

	RPC_STATUS status;

	status = RpcServerUseProtseqEp((RPC_WSTR)L"ncalrpc", 0, (RPC_WSTR)L"[retc-rpc]", NULL);

	if (status)
		exit(status);

	status = RpcServerRegisterIf2(rpc_retc_v2_1_s_ifspec, NULL, NULL, RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH, RPC_C_LISTEN_MAX_CALLS_DEFAULT, (unsigned)-1, SecurityCallback);

	if (status)
		exit(status);

	status = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, FALSE);

	if (status)
		exit(status);

	return 0;
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