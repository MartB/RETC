#include "RazerSDK.h"
#include "CorsairSDK.h"
/**
 * This sdk is just forwarding to the real dll.
 * WARNING: If the "real" dll is the patched stub we created, this will enter an endless loop!
 * NEVER CREATE GLOBAL DLL OVERWRITES in System32 with this SDK enabled.
 */
RazerSDK::RazerSDK() {
	this->SDK_NAME = "RazerPassthrough";

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
		{"SetEffect", nullptr},
		{"DeleteEffect", nullptr},
	};
}

bool RazerSDK::initialize() {
	// Initialize dll functions
	SDKLoaderMapNameToFunction(Init);
	SDKLoaderMapNameToFunction(UnInit);
	SDKLoaderMapNameToFunction(CreateKeyboardEffect);
	SDKLoaderMapNameToFunction(CreateMouseEffect);
	SDKLoaderMapNameToFunction(CreateHeadsetEffect);
	SDKLoaderMapNameToFunction(CreateMousepadEffect);
	SDKLoaderMapNameToFunction(CreateKeypadEffect);
	SDKLoaderMapNameToFunction(SetEffect);
	SDKLoaderMapNameToFunction(DeleteEffect);

	auto res = Init();
	if (res != RZRESULT_SUCCESS) {
		LOG_E("failed with code {0}", res);
		return false;
	}

	// Set everything to off best way to detect support + looks clean!
	for (int devID = KEYBOARD; devID < ALL; devID++) {
		auto type = static_cast<RETCDeviceType>(devID);
		if (playEffect(type, CHROMA_NONE, nullptr) == RZRESULT_SUCCESS) {
			enableSupportFor(type);
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
