// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "CorsairSDK.h"
#include <RzErrors.h>

CorsairSDK::CorsairSDK() {
	this->m_sdkName = L"CorsairCueSDK";
#ifdef _WIN64
	this->m_sdkDll = "CUESDK.x64_2015.dll";
#else
	this->m_sdkDll = "CUESDK_2015.dll";
#endif
	this->m_dllFunctionList = {
		{"CorsairSetLedsColors", nullptr},
		{"CorsairSetLedsColorsAsync", nullptr},
		{"CorsairGetDeviceCount", nullptr},
		{"CorsairGetDeviceInfo", nullptr},
		{"CorsairGetLedPositions", nullptr},
		{"CorsairGetLedIdForKeyName", nullptr},
		{"CorsairRequestControl", nullptr},
		{"CorsairPerformProtocolHandshake", nullptr},
		{"CorsairGetLastError", nullptr},
		{"CorsairGetLedPositionsByDeviceIndex", nullptr}
	};
}

bool CorsairSDK::initialize() {
	// Initialize dll functions
	SDKLoaderMapNameToFunction(CorsairSetLedsColors);
	SDKLoaderMapNameToFunction(CorsairSetLedsColorsAsync);
	SDKLoaderMapNameToFunction(CorsairGetDeviceCount);
	SDKLoaderMapNameToFunction(CorsairGetDeviceInfo);
	SDKLoaderMapNameToFunction(CorsairGetLedPositions);
	SDKLoaderMapNameToFunction(CorsairGetLedIdForKeyName);
	SDKLoaderMapNameToFunction(CorsairRequestControl);
	SDKLoaderMapNameToFunction(CorsairPerformProtocolHandshake);
	SDKLoaderMapNameToFunction(CorsairGetLastError);
	SDKLoaderMapNameToFunction(CorsairGetLedPositionsByDeviceIndex);

	const auto hsDetails = CorsairPerformProtocolHandshake();
	if (hsDetails.breakingChanges) {
		LOG_E("Protocol has breaking changes aborting");
		return false;
	}

	if (hsDetails.serverProtocolVersion == 0 || hsDetails.serverVersion == nullptr) {
		LOG_E("Invalid server protocol, make sure CUE is running and SDK access is enabled!");
		return false;
	}

	const auto devCount = CorsairGetDeviceCount();
	if (devCount == 0) {
		LOG_I("No devices found");
		return false;
	}

	if (CONFIG->GetBool(m_sdkConfigSection, L"exclusivemode", false)) {
		LOG_I("Using exclusive mode due to config parameter");

		if (!CorsairRequestControl(CorsairAccessMode::CAM_ExclusiveLightingControl)) {
			LOG_E("Requesting exclusive mode failed falling back to default.");
		}	
	}

	size_t maxLeds = 0;
	for (auto i = 0; i < devCount; i++) {
		const CorsairDeviceInfo* devInfo = CorsairGetDeviceInfo(i);

		if (devInfo->capsMask != CDC_Lighting) {
			continue;
		}

		auto devType = corsairToRetcDeviceType(devInfo->type);
		if (devType == ESIZE) {
			continue;
		}

		auto& ledVector = m_availableLeds[devType];

		switch (devType) {
		case MOUSE: {
			std::string modelName = devInfo->model;
			// The following transform is only working for ASCII characters, be careful!
			std::transform(modelName.begin(), modelName.end(), modelName.begin(), ::tolower);

			const auto numberOfKeys = devInfo->ledsCount;
			ledVector.reserve(numberOfKeys);

			/* #HACK workaround for the api limitation on the glaive.
			 * It is misreporting as 3 key but has CLM3 mapped to CLM5.
			*/
			if (modelName.find("glaive") != std::string::npos && numberOfKeys < 4) {
				ledVector.emplace_back(CLM_1);
				ledVector.emplace_back(CLM_2);
				ledVector.emplace_back(CLM_5);
				break;
			}

			const auto maxSequentialIds = CLM_4 - CLM_1;
			for (auto key = 0; key < numberOfKeys; key++) {
				auto ledId = CLM_1;
				if (key > maxSequentialIds) {
					ledId = static_cast<CorsairLedId>(CLM_5 + ((key -1) - maxSequentialIds));
				}
				else {
					ledId = static_cast<CorsairLedId>(CLM_1 + key);
				}

				ledVector.emplace_back(ledId);
			}

			
			break;
		}
		case HEADSET: {
			ledVector.reserve(2);
			ledVector.emplace_back(CLH_LeftLogo);
			ledVector.emplace_back(CLH_RightLogo);
			break;
		}
		case KEYBOARD:
		case MOUSEPAD:
		case KEYPAD: 
		case HEADSET_STAND: {
			auto const ledPositions = CorsairGetLedPositionsByDeviceIndex(i);
			if (!ledPositions) {
				LOG_E("Cue SDK call: CorsairGetLedPositionsByDeviceIndex failed, aborted.");
				return false;
			}

			auto const ledCount = static_cast<size_t>(ledPositions->numberOfLed);

			if (ledCount == 0) {
				continue;
			}

			ledVector.reserve(ledCount);

			const auto& ledData = ledPositions->pLedPosition;
			for (size_t key = 0; key < ledCount; key++) {
				auto ledId = ledData[key].ledId;
				ledVector.emplace_back(ledId);
			}
			break;
		}
		default:
			LOG_W("Skipping unsupported device type {0}", devType);
			continue;
		}

		// Get the max number of leds so we can later reserve the output color vector.
		const auto ledCount = ledVector.size();
		if (ledCount > maxLeds) {
			maxLeds = ledCount;
		}

		enableSupportFor(devType);
	}

	m_outputColorVector.reserve(maxLeds);
	return true;
}

void CorsairSDK::reset() {
	for (int devId = KEYBOARD; devId < ALL; devId++) {
		m_availableLeds[devId].clear();
	}

	m_outputColorVector.clear();
}

RZRESULT CorsairSDK::playEffect(const RETCDeviceType deviceType, const int effectType, const char data[]) {
	RZRESULT res;
	switch (deviceType) {
	case KEYBOARD:
		res = prepareKeyboardEffect(effectType, data);
		break;
	case MOUSE:
		res = prepareMouseEffect(effectType, data);
		break;
	case HEADSET:
		res = prepareHeadsetEffect(effectType, data);
		break;
	case MOUSEPAD:
		res = prepareMousePadEffect(effectType, data);
		break;
	case KEYPAD:
		res = RZRESULT_NOT_SUPPORTED;
		break;
	case SYSTEM: // Chroma link not supported...
		res = RZRESULT_NOT_SUPPORTED;
		break;
	case HEADSET_STAND:
		res = prepareHeadsetStandEffect(effectType, data);
		break;
	default:
		res = RZRESULT_INVALID;
		break;
	}

	if (res != RZRESULT_SUCCESS) {
		return res;
	}

	CorsairSetLedsColors(static_cast<int>(m_outputColorVector.size()), m_outputColorVector.data());
	m_outputColorVector.clear(); // resize(0) not needed anymore standard states capacity remains unchanged.

	if (const auto error = CorsairGetLastError()) {
		LOG_D("{0}", errToString(error));
		return RZRESULT_NOT_SUPPORTED;
	}

	return RZRESULT_SUCCESS;
}

RZRESULT CorsairSDK::prepareKeyboardEffect(const int type, const char effectData[]) {
	using namespace ChromaSDK::Keyboard;
	const auto& ledVector = m_availableLeds[KEYBOARD];
	// High led count we better reserve.
	m_outputColorVector.reserve(ledVector.size());
	int row, col;

	if (type == CHROMA_NONE) {
		auto ledColor = convertLedColor(0);

		for (const auto ledId : ledVector) {
			ledColor.ledId = ledId;
			m_outputColorVector.emplace_back(ledColor);
		}
	}
	else if (type == CHROMA_STATIC) {
		const auto& custEffect = reinterpret_cast<const STATIC_EFFECT_TYPE*>(effectData);
		auto ledColor = convertLedColor(custEffect->Color);

		for (const auto ledId : ledVector) {
			if (!findKeyboardLed(ledId, &row, &col)) {
				continue;
			}

			ledColor.ledId = ledId;
			m_outputColorVector.emplace_back(ledColor);
		}
	}
	else if (type == CHROMA_CUSTOM) {
		const auto& custEffect = reinterpret_cast<const CUSTOM_EFFECT_TYPE*>(effectData);
		for (const auto ledId : ledVector) {
			if (!findKeyboardLed(ledId, &row, &col)) {
				continue;
			}

			auto ledColor = convertLedColor(custEffect->Color[row][col]);
			ledColor.ledId = ledId;
			m_outputColorVector.emplace_back(ledColor);
		}
	}
	else if (type == CHROMA_CUSTOM_KEY) {
		const auto& custEffect = reinterpret_cast<const CUSTOM_KEY_EFFECT_TYPE*>(effectData);
		for (const auto ledId : ledVector) {
			if (!findKeyboardLed(ledId, &row, &col)) {
				continue;
			}

			// Check if the key has a specific override
			auto origColor = custEffect->Key[row][col];
			if (origColor == NULL) {
				origColor = custEffect->Color[row][col];
			}

			auto ledColor = convertLedColor(origColor);
			ledColor.ledId = ledId;
			m_outputColorVector.emplace_back(ledColor);
		}
	}
	else {
		return RZRESULT_NOT_SUPPORTED;
	}

	return RZRESULT_SUCCESS;
}


RZRESULT CorsairSDK::prepareMouseEffect(const int type, const char effectData[]) {
	using namespace ChromaSDK::Mouse;

	int row, col;
	const auto& ledVector = m_availableLeds[MOUSE];

	if (type == CHROMA_NONE) {
		auto ledColor = convertLedColor(0);

		for (const auto ledId : ledVector) {
			ledColor.ledId = ledId;
			m_outputColorVector.emplace_back(ledColor);
		}
	}
	else if (type == CHROMA_STATIC) {
		const auto& custEffect = reinterpret_cast<const STATIC_EFFECT_TYPE*>(effectData);

		auto ledColor = convertLedColor(custEffect->Color);

		if (custEffect->LEDId == RZLED_ALL) {
			for (const auto ledId : ledVector) {
				ledColor.ledId = ledId;
				m_outputColorVector.emplace_back(ledColor);
			}
		}
		else {
			ledColor.ledId = findMouseLedForRzled(custEffect->LEDId);

			if (ledColor.ledId != CLI_Invalid) {
				m_outputColorVector.emplace_back(ledColor);
			}
		}
	}
	else if (type == CHROMA_CUSTOM) {
		const auto& custEffect = reinterpret_cast<const CUSTOM_EFFECT_TYPE*>(effectData);


		for (const auto ledId : ledVector) {
			const auto val = findMouseLedForCld(ledId);
			if (val == RZLED_NONE) {
				continue;
			}

			auto ledColor = convertLedColor(custEffect->Color[val]);
			ledColor.ledId = ledId;
			m_outputColorVector.emplace_back(ledColor);
		}
	}
	else if (type == CHROMA_CUSTOM2) {
		const auto& custEffect = reinterpret_cast<const CUSTOM_EFFECT_TYPE2*>(effectData);

		for (const auto ledId : ledVector) {
			if (!findMouseLed(ledId, &row, &col)) {
				continue;
			}

			auto ledColor = convertLedColor(custEffect->Color[row][col]);
			ledColor.ledId = ledId;

			m_outputColorVector.emplace_back(ledColor);
		}
	}
	else {
		return RZRESULT_NOT_SUPPORTED;
	}

	return RZRESULT_SUCCESS;
}

RZRESULT CorsairSDK::prepareMousePadEffect(const int type, const char effectData[]) {
	using namespace ChromaSDK::Mousepad;

	const auto& ledVector = m_availableLeds[MOUSEPAD];

	if (type == CHROMA_NONE) {
		auto ledColor = convertLedColor(0);

		for (const auto ledId : ledVector) {
			ledColor.ledId = ledId;
			m_outputColorVector.emplace_back(ledColor);
		}
	}
	else if (type == CHROMA_STATIC) {
		const auto& custEffect = reinterpret_cast<const STATIC_EFFECT_TYPE*>(effectData);

		auto ledColor = convertLedColor(custEffect->Color);

		for (const auto led : ledVector) {
			ledColor.ledId = led;
			m_outputColorVector.emplace_back(ledColor);
		}
	}
	else if (type == CHROMA_CUSTOM) {
		const auto& custEffect = reinterpret_cast<const CUSTOM_EFFECT_TYPE*>(effectData);

		for (const auto led : ledVector) {
			// Razer mousepad leds start with 0 on the right but 0 is left on corsair so re-map accordingly.
			auto ledColor = convertLedColor(custEffect->Color[MAX_LEDS - (led - CLMM_Zone1) - 1]); 
			ledColor.ledId = led;
			m_outputColorVector.emplace_back(ledColor);
		}
	}

	return RZRESULT_SUCCESS;
}

RZRESULT CorsairSDK::prepareHeadsetEffect(const int type, const char effectData[]) {
	using namespace ChromaSDK::Headset;

	const auto& ledVector = m_availableLeds[HEADSET];

	if (type == CHROMA_NONE) {
		auto ledColor = convertLedColor(0);

		for (const auto ledId : ledVector) {
			ledColor.ledId = ledId;
			m_outputColorVector.emplace_back(ledColor);
		}
	}
	else if (type == CHROMA_STATIC) {
		const auto& custEffect = reinterpret_cast<const STATIC_EFFECT_TYPE*>(effectData);
		auto ledColor = convertLedColor(custEffect->Color);

		for (const auto led : ledVector) {
			ledColor.ledId = led;
			m_outputColorVector.emplace_back(ledColor);
		}
	}
	else if (type == CHROMA_CUSTOM) {
		const auto& custEffect = reinterpret_cast<const CUSTOM_EFFECT_TYPE*>(effectData);

		for (const auto led : ledVector) {
			auto ledColor = convertLedColor(custEffect->Color[(led - CLH_LeftLogo)]);
			ledColor.ledId = led;
			m_outputColorVector.emplace_back(ledColor);
		}
	}

	return RZRESULT_SUCCESS;
}

/*
	**!WORKAROUND_ALERT!**
	This contains a custom led mapping logic, it will work good enough for most
	cases but it will look a bit off on "fluid" color travelling animations.

	Reasons:
	- ChromaLedCount = 15 > CorsairLedCount = 9
	- Mapping is different
	- Only 1 bottom LED on corsair.
	- No top led on Razer
	- Some missing side leds on corsair.

	The code tries to paint the following picture:
	(Each line is a corsair LED Position, read counterclockwise)
	- (TOP+LOGO) zone1, rzid = 0
	- (TOP) zone2, rzids 0
	- (R) zone3, rzid 1,2
	- (R) zone4, rzid 3
	- (R) zone5, rzid 4
	- (BOTTOM) zone6, rzid 5,6,7,8,9
	- (L) zone7, rzid 10,11
	- (L) zone8, rzid 12,13
	- (L) zone9, rzid 14
*/
RZRESULT CorsairSDK::prepareHeadsetStandEffect(const int type, const char effectData[]) {
	using namespace ChromaSDK::Mousepad;

	const auto& ledVector = m_availableLeds[HEADSET_STAND];

	if (type == CHROMA_NONE) {
		auto ledColor = convertLedColor(0);

		for (const auto ledId : ledVector) {
			ledColor.ledId = ledId;
			m_outputColorVector.emplace_back(ledColor);
		}
	}
	else if (type == CHROMA_STATIC) {
		const auto& custEffect = reinterpret_cast<const STATIC_EFFECT_TYPE*>(effectData);
		auto ledColor = convertLedColor(custEffect->Color);

		for (const auto led : ledVector) {
			ledColor.ledId = led;
			m_outputColorVector.emplace_back(ledColor);
		}
	}
	else if (type == CHROMA_CUSTOM) {
		const auto& custEffect = reinterpret_cast<const CUSTOM_EFFECT_TYPE*>(effectData);

		for (const auto led : ledVector) {
			COLORREF color = 0;
			if (led == CLHSS_Zone6) {
				color = custEffect->Color[5];
				for (auto idx = 6; idx <= 9 && color == 0; idx++) {
					color = custEffect->Color[idx];
				}
			}
			else if (led == CLHSS_Zone3) {
				color = custEffect->Color[1];
				if (color == 0) {
					color = custEffect->Color[2];
				}
			}
			else if (led == CLHSS_Zone7) {
				color = custEffect->Color[10];
				if (color == 0) {
					color = custEffect->Color[11];
				}
			}
			else if (led == CLHSS_Zone8) {
				color = custEffect->Color[12];
				if (color == 0) {
					color = custEffect->Color[13];
				}
			}
			else {
				const auto cIdx = findHeadsetStandLed(led);
				if (cIdx == -1) {
					return RZRESULT_NOT_SUPPORTED;
				}

				color = custEffect->Color[cIdx];
			}

			auto ledColor = convertLedColor(color);
			ledColor.ledId = led;
			m_outputColorVector.emplace_back(ledColor);
		}
	}

	return RZRESULT_SUCCESS;
}

CorsairLedColor CorsairSDK::convertLedColor(const COLORREF& color) {
	uint8_t r, g, b;
	CONVERT_AND_TRANSFORM_COLORS(color, r, g, b);

	CorsairLedColor ledColor;
	ledColor.r = r;
	ledColor.g = g;
	ledColor.b = b;
	ledColor.ledId = CLI_Invalid;

	return ledColor;
}

#define MAPTO(x,y) case x: return y;
#define MAPTOASSIGN(x,y) case x: { *row = HIBYTE(y); *col = LOBYTE(y); return true;}
#define CUSTOMKEYASSIGN(x,y,z) case x: { *row = y; *col = z;  break;}

RETCDeviceType CorsairSDK::corsairToRetcDeviceType(const CorsairDeviceType type) {
	switch (type) {
		MAPTO(CDT_Unknown, RETCDeviceType::ESIZE);
		MAPTO(CDT_Mouse, RETCDeviceType::MOUSE);
		MAPTO(CDT_Keyboard, RETCDeviceType::KEYBOARD);
		MAPTO(CDT_Headset, RETCDeviceType::HEADSET);
		MAPTO(CDT_MouseMat, RETCDeviceType::MOUSEPAD);
		MAPTO(CDT_HeadsetStand, RETCDeviceType::HEADSET_STAND);
	default:
		return ESIZE;
	}
}

bool CorsairSDK::findKeyboardLed(const CorsairLedId ledid, int* row, int* col) {
	using namespace ChromaSDK::Keyboard;
	switch (ledid) {
		MAPTOASSIGN(CLK_Escape, RZKEY_ESC);
		MAPTOASSIGN(CLK_F1, RZKEY_F1);
		MAPTOASSIGN(CLK_F2, RZKEY_F2);
		MAPTOASSIGN(CLK_F3, RZKEY_F3);
		MAPTOASSIGN(CLK_F4, RZKEY_F4);
		MAPTOASSIGN(CLK_F5, RZKEY_F5);
		MAPTOASSIGN(CLK_F6, RZKEY_F6);
		MAPTOASSIGN(CLK_F7, RZKEY_F7);
		MAPTOASSIGN(CLK_F8, RZKEY_F8);
		MAPTOASSIGN(CLK_F9, RZKEY_F9);
		MAPTOASSIGN(CLK_F10, RZKEY_F10);
		MAPTOASSIGN(CLK_F11, RZKEY_F11);
		MAPTOASSIGN(CLK_F12, RZKEY_F12);
		MAPTOASSIGN(CLK_1, RZKEY_1);
		MAPTOASSIGN(CLK_2, RZKEY_2);
		MAPTOASSIGN(CLK_3, RZKEY_3);
		MAPTOASSIGN(CLK_4, RZKEY_4);
		MAPTOASSIGN(CLK_5, RZKEY_5);
		MAPTOASSIGN(CLK_6, RZKEY_6);
		MAPTOASSIGN(CLK_7, RZKEY_7);
		MAPTOASSIGN(CLK_8, RZKEY_8);
		MAPTOASSIGN(CLK_9, RZKEY_9);
		MAPTOASSIGN(CLK_0, RZKEY_0);
		MAPTOASSIGN(CLK_A, RZKEY_A);
		MAPTOASSIGN(CLK_B, RZKEY_B);
		MAPTOASSIGN(CLK_C, RZKEY_C);
		MAPTOASSIGN(CLK_D, RZKEY_D);
		MAPTOASSIGN(CLK_E, RZKEY_E);
		MAPTOASSIGN(CLK_F, RZKEY_F);
		MAPTOASSIGN(CLK_G, RZKEY_G);
		MAPTOASSIGN(CLK_H, RZKEY_H);
		MAPTOASSIGN(CLK_I, RZKEY_I);
		MAPTOASSIGN(CLK_J, RZKEY_J);
		MAPTOASSIGN(CLK_K, RZKEY_K);
		MAPTOASSIGN(CLK_L, RZKEY_L);
		MAPTOASSIGN(CLK_M, RZKEY_M);
		MAPTOASSIGN(CLK_N, RZKEY_N);
		MAPTOASSIGN(CLK_O, RZKEY_O);
		MAPTOASSIGN(CLK_P, RZKEY_P);
		MAPTOASSIGN(CLK_Q, RZKEY_Q);
		MAPTOASSIGN(CLK_R, RZKEY_R);
		MAPTOASSIGN(CLK_S, RZKEY_S);
		MAPTOASSIGN(CLK_T, RZKEY_T);
		MAPTOASSIGN(CLK_U, RZKEY_U);
		MAPTOASSIGN(CLK_V, RZKEY_V);
		MAPTOASSIGN(CLK_W, RZKEY_W);
		MAPTOASSIGN(CLK_X, RZKEY_X);
		MAPTOASSIGN(CLK_Y, RZKEY_Y);
		MAPTOASSIGN(CLK_Z, RZKEY_Z);
		MAPTOASSIGN(CLK_NumLock, RZKEY_NUMLOCK);
		MAPTOASSIGN(CLK_Keypad0, RZKEY_NUMPAD0);
		MAPTOASSIGN(CLK_Keypad1, RZKEY_NUMPAD1);
		MAPTOASSIGN(CLK_Keypad2, RZKEY_NUMPAD2);
		MAPTOASSIGN(CLK_Keypad3, RZKEY_NUMPAD3);
		MAPTOASSIGN(CLK_Keypad4, RZKEY_NUMPAD4);
		MAPTOASSIGN(CLK_Keypad5, RZKEY_NUMPAD5);
		MAPTOASSIGN(CLK_Keypad6, RZKEY_NUMPAD6);
		MAPTOASSIGN(CLK_Keypad7, RZKEY_NUMPAD7);
		MAPTOASSIGN(CLK_Keypad8, RZKEY_NUMPAD8);
		MAPTOASSIGN(CLK_Keypad9, RZKEY_NUMPAD9);
		MAPTOASSIGN(CLK_KeypadSlash, RZKEY_NUMPAD_DIVIDE);
		MAPTOASSIGN(CLK_KeypadAsterisk, RZKEY_NUMPAD_MULTIPLY);
		MAPTOASSIGN(CLK_KeypadMinus, RZKEY_NUMPAD_SUBTRACT);
		MAPTOASSIGN(CLK_KeypadPlus, RZKEY_NUMPAD_ADD);
		MAPTOASSIGN(CLK_KeypadEnter, RZKEY_NUMPAD_ENTER);
		MAPTOASSIGN(CLK_KeypadPeriodAndDelete, RZKEY_NUMPAD_DECIMAL);
		MAPTOASSIGN(CLK_PrintScreen, RZKEY_PRINTSCREEN);
		MAPTOASSIGN(CLK_ScrollLock, RZKEY_SCROLL);
		MAPTOASSIGN(CLK_PauseBreak, RZKEY_PAUSE);
		MAPTOASSIGN(CLK_Insert, RZKEY_INSERT);
		MAPTOASSIGN(CLK_Home, RZKEY_HOME);
		MAPTOASSIGN(CLK_PageUp, RZKEY_PAGEUP);
		MAPTOASSIGN(CLK_Delete, RZKEY_DELETE);
		MAPTOASSIGN(CLK_End, RZKEY_END);
		MAPTOASSIGN(CLK_PageDown, RZKEY_PAGEDOWN);
		MAPTOASSIGN(CLK_UpArrow, RZKEY_UP);
		MAPTOASSIGN(CLK_LeftArrow, RZKEY_LEFT);
		MAPTOASSIGN(CLK_DownArrow, RZKEY_DOWN);
		MAPTOASSIGN(CLK_RightArrow, RZKEY_RIGHT);
		MAPTOASSIGN(CLK_Tab, RZKEY_TAB);
		MAPTOASSIGN(CLK_CapsLock, RZKEY_CAPSLOCK);
		MAPTOASSIGN(CLK_Backspace, RZKEY_BACKSPACE);
		MAPTOASSIGN(CLK_Enter, RZKEY_ENTER);
		MAPTOASSIGN(CLK_LeftCtrl, RZKEY_LCTRL);
		MAPTOASSIGN(CLK_LeftGui, RZKEY_LWIN);
		MAPTOASSIGN(CLK_LeftAlt, RZKEY_LALT);
		MAPTOASSIGN(CLK_Space, RZKEY_SPACE);
		MAPTOASSIGN(CLK_RightAlt, RZKEY_RALT);
		MAPTOASSIGN(CLK_RightGui, RZKEY_FN); 
		MAPTOASSIGN(CLK_Fn, RZKEY_FN); // This is actually the same key as RightGui on some keyboard variants
		MAPTOASSIGN(CLK_Application, RZKEY_RMENU);
		MAPTOASSIGN(CLK_RightCtrl, RZKEY_RCTRL);
		MAPTOASSIGN(CLK_LeftShift, RZKEY_LSHIFT);
		MAPTOASSIGN(CLK_RightShift, RZKEY_RSHIFT);
		MAPTOASSIGN(CLK_GraveAccentAndTilde, RZKEY_OEM_1);
		MAPTOASSIGN(CLK_EqualsAndPlus, RZKEY_OEM_3);
		MAPTOASSIGN(CLK_MinusAndUnderscore, RZKEY_OEM_2);
		MAPTOASSIGN(CLK_BracketLeft, RZKEY_OEM_4);
		MAPTOASSIGN(CLK_BracketRight, RZKEY_OEM_5);
		MAPTOASSIGN(CLK_Backslash, RZKEY_OEM_6);
		MAPTOASSIGN(CLK_SemicolonAndColon, RZKEY_OEM_7);
		MAPTOASSIGN(CLK_ApostropheAndDoubleQuote, RZKEY_OEM_8);
		MAPTOASSIGN(CLK_CommaAndLessThan, RZKEY_OEM_9);
		MAPTOASSIGN(CLK_PeriodAndBiggerThan, RZKEY_OEM_10);
		MAPTOASSIGN(CLK_SlashAndQuestionMark, RZKEY_OEM_11);
		MAPTOASSIGN(CLK_NonUsTilde, RZKEY_EUR_1);
		MAPTOASSIGN(CLK_NonUsBackslash, RZKEY_EUR_2);
		MAPTOASSIGN(CLK_Logo, RZLED_LOGO);
		MAPTOASSIGN(CLI_Invalid, RZKEY_INVALID);
	default: {
		return false;
	}
	}
}

bool CorsairSDK::findMouseLed(const CorsairLedId ledid, int* row, int* col) {
	using namespace ChromaSDK::Mouse;

	switch (ledid) {
		MAPTOASSIGN(CLI_Invalid, RZLED_NONE);
		MAPTOASSIGN(CLM_1, RZLED2_LOGO);
		MAPTOASSIGN(CLM_2, RZLED2_SCROLLWHEEL);
		MAPTOASSIGN(CLM_3, RZLED2_BACKLIGHT);
		MAPTOASSIGN(CLM_4, RZLED2_LEFT_SIDE1);
		MAPTOASSIGN(CLM_5, RZLED2_BACKLIGHT);
		MAPTOASSIGN(CLM_6, RZLED2_BOTTOM1);
	default:
		return false;
	}
}

// This is deprecated
CorsairLedId CorsairSDK::findMouseLedForRzled(const ChromaSDK::Mouse::RZLED led) {
	using namespace ChromaSDK::Mouse;

	switch (led) {
		MAPTO(RZLED_NONE, CLI_Invalid);
		MAPTO(RZLED_LOGO, CLM_1);
		MAPTO(RZLED_SCROLLWHEEL, CLM_2);
		MAPTO(RZLED_BACKLIGHT, CLM_3);
		MAPTO(RZLED_SIDE_STRIP1, CLM_4);
		MAPTO(RZLED_SIDE_STRIP2, CLM_5);
		MAPTO(RZLED_SIDE_STRIP3, CLM_6);
	default:
		return CLI_Invalid;
	}
}

ChromaSDK::Mouse::RZLED CorsairSDK::findMouseLedForCld(const CorsairLedId led) {
	using namespace ChromaSDK::Mouse;

	switch (led) {
		MAPTO(CLI_Invalid, RZLED_NONE);
		MAPTO(CLM_1, RZLED_LOGO);
		MAPTO(CLM_2, RZLED_SCROLLWHEEL);
		MAPTO(CLM_3, RZLED_BACKLIGHT);
		MAPTO(CLM_4, RZLED_SIDE_STRIP1);
		MAPTO(CLM_5, RZLED_SIDE_STRIP2);
		MAPTO(CLM_6, RZLED_SIDE_STRIP3);

	default:
		return RZLED_NONE;
	}
}

int CorsairSDK::findHeadsetStandLed(const CorsairLedId ledid) {
	using namespace ChromaSDK::Keyboard;
	switch (ledid) {
		MAPTO(CLHSS_Zone1, 0); // Logo just like on razer (right side there)

		MAPTO(CLHSS_Zone2, 0);
		MAPTO(CLHSS_Zone4, 3);
		MAPTO(CLHSS_Zone5, 4);

		MAPTO(CLHSS_Zone9, 14);
		default: 
			return -1;
	}
}

std::string CorsairSDK::errToString(const CorsairError& error) {
	switch (error) {
	case CE_Success:
		return "CE_Success";
	case CE_ServerNotFound:
		return "CE_ServerNotFound";
	case CE_NoControl:
		return "CE_NoControl";
	case CE_ProtocolHandshakeMissing:
		return "CE_ProtocolHandshakeMissing";
	case CE_IncompatibleProtocol:
		return "CE_IncompatibleProtocol";
	case CE_InvalidArguments:
		return "CE_InvalidArguments";
	default:
		return "unknown error";
	}
}
