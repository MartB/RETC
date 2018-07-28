// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "RazerSDK.h"
/**
 * This sdk is just forwarding to the real dll.
 * WARNING: If the "real" dll is the patched stub we created, this will enter an endless loop!
 * NEVER CREATE GLOBAL DLL OVERWRITES in System32 with this SDK enabled.
 */
RazerSDK::RazerSDK() {
	this->SDK_NAME = L"RazerPassthrough";

#ifdef _WIN64
	this->SDK_DLL = "RzChromaSDK64.dll";
#else
	this->SDK_DLL = "RzChromaSDK.dll";
#endif
	this->DLL_FUNCTION_LIST = {
		{"Init", nullptr},
		{"UnInit", nullptr},
		{"CreateKeyboardEffect", nullptr},
		{"CreateMouseEffect", nullptr},
		{"CreateHeadsetEffect", nullptr},
		{"CreateMousepadEffect", nullptr},
		{"CreateKeypadEffect", nullptr},
		{"CreateChromaLinkEffect", nullptr},
		{"SetEffect", nullptr},
		{"DeleteEffect", nullptr},
		{"QueryDevice", nullptr}
	};
}

// This allows the use of the Razer Chroma Emulators in debug.
#if defined (_DEBUG)
#define devicePhysicallyPresent true
#else
#define devicePhysicallyPresent deviceInfo.Connected == 1
#endif

bool RazerSDK::initialize() {
	// Initialize dll functions
	SDKLoaderMapNameToFunction(Init);
	SDKLoaderMapNameToFunction(UnInit);
	SDKLoaderMapNameToFunction(CreateKeyboardEffect);
	SDKLoaderMapNameToFunction(CreateMouseEffect);
	SDKLoaderMapNameToFunction(CreateHeadsetEffect);
	SDKLoaderMapNameToFunction(CreateMousepadEffect);
	SDKLoaderMapNameToFunction(CreateKeypadEffect);
	SDKLoaderMapNameToFunction(CreateChromaLinkEffect);	
	SDKLoaderMapNameToFunction(SetEffect);
	SDKLoaderMapNameToFunction(DeleteEffect);
	SDKLoaderMapNameToFunction(QueryDevice);

	auto res = Init();
	if (res != RZRESULT_SUCCESS) {
		LOG_E("failed with code {0}", res);
		return false;
	}

	// Check for connected razer devices.
	DEVICE_INFO_TYPE deviceInfo = {};
	for (const auto &razerdevguid : LookupArrays::razerDevices) {
		if (QueryDevice(razerdevguid, deviceInfo) == RZRESULT_SUCCESS && devicePhysicallyPresent) {
			auto deviceType = razerToRETCDeviceTYPE(deviceInfo);
			if (deviceType == ESIZE) {
				continue;
			}

			if (CONFIG->GetBool(SDK_CONFIG_SECTION, L"overwritedevices", true)) {
				auto findResult = std::find_if(std::begin(LookupMaps::razerStringToDevID), std::end(LookupMaps::razerStringToDevID), [&](const auto &pair) {
					return pair.second == razerdevguid;
				});

				if (findResult == std::end(LookupMaps::razerStringToDevID)) {
					LOG->error("Connected device is not fully supported by this application, please report it: {}", guidToString(razerdevguid));
				}
				else {
					LOG->info(L"Found valid razer device altering emulated device id: {}", findResult->first.c_str());
					CONFIG->SetWString(SDK_MAIN_CONFIG_SECTION, LookupArrays::EM_KEYS[deviceType], findResult->first.c_str());
				}
			}

			enableSupportFor(deviceType);
		}
	}

	return true;
}

void RazerSDK::reset() {
	UnInit();
}

RZRESULT RazerSDK::playEffect(RETCDeviceType device, int type, const char data[]) {
	RZRESULT res;
	switch (device) {
	case KEYBOARD:
		res = CreateKeyboardEffect(static_cast<Keyboard::EFFECT_TYPE>(type), PRZPARAM(data), nullptr);
		break;
	case MOUSE:
		res = CreateMouseEffect(static_cast<Mouse::EFFECT_TYPE>(type), PRZPARAM(data), nullptr);
		break;
	case HEADSET:
		res = CreateHeadsetEffect(static_cast<Headset::EFFECT_TYPE>(type), PRZPARAM(data), nullptr);
		break;
	case MOUSEPAD:
		res = CreateMousepadEffect(static_cast<Mousepad::EFFECT_TYPE>(type), PRZPARAM(data), nullptr);
		break;
	case KEYPAD:
		res = CreateKeypadEffect(static_cast<Keypad::EFFECT_TYPE>(type), PRZPARAM(data), nullptr);
		break;
	case SYSTEM:
		res = CreateChromaLinkEffect(static_cast<ChromaLink::EFFECT_TYPE>(type), PRZPARAM(data), nullptr);
		break;
	default:
		res = RZRESULT_INVALID;
		break;
	}

#ifdef _DEBUG
	if (res != RZRESULT_SUCCESS) {
		LOG_D("failed with {0}", res);
	}
#endif

	return res;
}

 RETCDeviceType RazerSDK::razerToRETCDeviceTYPE(DEVICE_INFO_TYPE devType) {
	switch (devType.DeviceType) {
	case DEVICE_INFO_TYPE::DEVICE_KEYBOARD:
		return KEYBOARD;
	case DEVICE_INFO_TYPE::DEVICE_MOUSE: 
		return MOUSE;
	case DEVICE_INFO_TYPE::DEVICE_HEADSET:
		return HEADSET;
	case DEVICE_INFO_TYPE::DEVICE_MOUSEPAD:
		return MOUSEPAD;
	case DEVICE_INFO_TYPE::DEVICE_KEYPAD:
		return KEYPAD;
	case DEVICE_INFO_TYPE::DEVICE_SYSTEM:
		return SYSTEM;
	case DEVICE_INFO_TYPE::DEVICE_INVALID:
		return ESIZE;
	default:
		return ESIZE;
	}
}