// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "CorsairSDK.h"

CorsairSDK::CorsairSDK() {
	this->SDK_NAME = L"CorsairCueSDK";
#ifdef _WIN64
	this->SDK_DLL = "CUESDK.x64_2015.dll";
#else
	this->SDK_DLL = "CUESDK_2015.dll";
#endif
	this->DLL_FUNCTION_LIST = {
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

	auto hsDetails = CorsairPerformProtocolHandshake();
	if (hsDetails.breakingChanges == true) {
		LOG_E("Protocol has breaking changes aborting");
		return false;
	}

	if (hsDetails.serverProtocolVersion == 0 || hsDetails.serverVersion == nullptr) {
		LOG_E("Invalid server protocol");
		return false;
	}

	auto devCount = CorsairGetDeviceCount();
	if (devCount == 0) {
		LOG_I("No devices found");
		return false;
	}

	if (CONFIG->GetBool(SDK_CONFIG_SECTION, L"exclusivemode", false)) {
		LOG_I("Using exclusive mode due to config parameter");

		if (!CorsairRequestControl(CorsairAccessMode::CAM_ExclusiveLightingControl)) {
			LOG_E("Requesting exclusive mode failed falling back to default.");
		}	
	}

	for (auto i = 0; i < devCount; i++) {
		const CorsairDeviceInfo* devInfo = CorsairGetDeviceInfo(i);

		if (devInfo->capsMask != CDC_Lighting) {
			continue;
		}

		auto devType = corsairToRETCDeviceTYPE(devInfo->type);
		if (devType == ESIZE) {
			continue;
		}

		auto& ledVector = m_availableLeds[devType];

		switch (devType) {
		case MOUSE: {
			const auto& numberOfKeys = static_cast<size_t>(devInfo->physicalLayout - CPL_Zones1 + 1);
			ledVector.reserve(numberOfKeys);
			for (size_t key = 0; key < numberOfKeys; key++) {
				auto ledId = static_cast<CorsairLedId>(CLM_1 + key);
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
		case KEYPAD: {
			auto const ledPositions = CorsairGetLedPositionsByDeviceIndex(i);
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

		enableSupportFor(devType);
	}

	return true;
}

void CorsairSDK::reset() {
	for (int devID = KEYBOARD; devID < ALL; devID++) {
		m_availableLeds[devID].clear();
	}

	m_outputColorVector.clear();
}

RZRESULT CorsairSDK::playEffect(RETCDeviceType deviceType, int effectType, const char data[]) {
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
	default:
		res = RZRESULT_INVALID;
		break;
	}

	if (res != RZRESULT_SUCCESS) {
		return res;
	}

	CorsairSetLedsColors(static_cast<int>(m_outputColorVector.size()), m_outputColorVector.data());
	m_outputColorVector.resize(0);

	if (const auto error = CorsairGetLastError()) {
		LOG_D("{0}", errToString(error));
		return RZRESULT_NOT_SUPPORTED;
	}

	return RZRESULT_SUCCESS;
}

RZRESULT CorsairSDK::prepareKeyboardEffect(int type, const char effectData[]) {
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


RZRESULT CorsairSDK::prepareMouseEffect(int type, const char effectData[]) {
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
				if (!findMouseLed(ledId, &row, &col)) {
					continue;
				}

				ledColor.ledId = ledId;
				m_outputColorVector.emplace_back(ledColor);
			}
		}
		else {
			ledColor.ledId = findMouseLed(custEffect->LEDId);

			if (ledColor.ledId != CLI_Invalid) {
				m_outputColorVector.emplace_back(ledColor);
			}
		}
	}
	else if (type == CHROMA_CUSTOM) {
		const auto& custEffect = reinterpret_cast<const CUSTOM_EFFECT_TYPE*>(effectData);


		for (const auto ledId : ledVector) {
			RZLED val = findMouseLed(ledId);
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

RZRESULT CorsairSDK::prepareMousePadEffect(int type, const char effectData[]) {
	using namespace ChromaSDK::Mousepad;

	const ledIDVector& ledVector = m_availableLeds[MOUSEPAD];

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
			auto ledColor = convertLedColor(custEffect->Color[(led - CLMM_Zone1)]);
			ledColor.ledId = led;
			m_outputColorVector.emplace_back(ledColor);
		}
	}

	return RZRESULT_SUCCESS;
}

RZRESULT CorsairSDK::prepareHeadsetEffect(int type, const char effectData[]) {
	using namespace ChromaSDK::Headset;

	const ledIDVector& ledVector = m_availableLeds[HEADSET];

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
			auto ledColor = convertLedColor(custEffect->Color[findMousepadLed(led)]);
			ledColor.ledId = led;
			m_outputColorVector.emplace_back(ledColor);
		}
	}

	return RZRESULT_SUCCESS;
}

CorsairLedColor CorsairSDK::convertLedColor(const COLORREF& color) {
	CorsairLedColor ledColor;
	ledColor.r = GetRValue(color);
	ledColor.g = GetGValue(color);
	ledColor.b = GetBValue(color);
	ledColor.ledId = CLI_Invalid;

	return ledColor;
}

#define mapto(x,y) case x: return y;
#define maptoassign(x,y) case x: { *row = HIBYTE(y); *col = LOBYTE(y); return true;}
#define customkeyassign(x,y,z) case x: { *row = y; *col = z;  break;}

RETCDeviceType CorsairSDK::corsairToRETCDeviceTYPE(CorsairDeviceType type) {
	switch (type) {
		mapto(CDT_Unknown, RETCDeviceType::ESIZE);
		mapto(CDT_Mouse, RETCDeviceType::MOUSE);
		mapto(CDT_Keyboard, RETCDeviceType::KEYBOARD);
		mapto(CDT_Headset, RETCDeviceType::HEADSET);
		mapto(CDT_MouseMat, RETCDeviceType::MOUSEPAD);
	default:
		return ESIZE;
	}
}

bool CorsairSDK::findKeyboardLed(CorsairLedId ledid, int* row, int* col) {
	using namespace ChromaSDK::Keyboard;
	switch (ledid) {
		maptoassign(CLK_Escape, RZKEY_ESC);
		maptoassign(CLK_F1, RZKEY_F1);
		maptoassign(CLK_F2, RZKEY_F2);
		maptoassign(CLK_F3, RZKEY_F3);
		maptoassign(CLK_F4, RZKEY_F4);
		maptoassign(CLK_F5, RZKEY_F5);
		maptoassign(CLK_F6, RZKEY_F6);
		maptoassign(CLK_F7, RZKEY_F7);
		maptoassign(CLK_F8, RZKEY_F8);
		maptoassign(CLK_F9, RZKEY_F9);
		maptoassign(CLK_F10, RZKEY_F10);
		maptoassign(CLK_F11, RZKEY_F11);
		maptoassign(CLK_F12, RZKEY_F12);
		maptoassign(CLK_1, RZKEY_1);
		maptoassign(CLK_2, RZKEY_2);
		maptoassign(CLK_3, RZKEY_3);
		maptoassign(CLK_4, RZKEY_4);
		maptoassign(CLK_5, RZKEY_5);
		maptoassign(CLK_6, RZKEY_6);
		maptoassign(CLK_7, RZKEY_7);
		maptoassign(CLK_8, RZKEY_8);
		maptoassign(CLK_9, RZKEY_9);
		maptoassign(CLK_0, RZKEY_0);
		maptoassign(CLK_A, RZKEY_A);
		maptoassign(CLK_B, RZKEY_B);
		maptoassign(CLK_C, RZKEY_C);
		maptoassign(CLK_D, RZKEY_D);
		maptoassign(CLK_E, RZKEY_E);
		maptoassign(CLK_F, RZKEY_F);
		maptoassign(CLK_G, RZKEY_G);
		maptoassign(CLK_H, RZKEY_H);
		maptoassign(CLK_I, RZKEY_I);
		maptoassign(CLK_J, RZKEY_J);
		maptoassign(CLK_K, RZKEY_K);
		maptoassign(CLK_L, RZKEY_L);
		maptoassign(CLK_M, RZKEY_M);
		maptoassign(CLK_N, RZKEY_N);
		maptoassign(CLK_O, RZKEY_O);
		maptoassign(CLK_P, RZKEY_P);
		maptoassign(CLK_Q, RZKEY_Q);
		maptoassign(CLK_R, RZKEY_R);
		maptoassign(CLK_S, RZKEY_S);
		maptoassign(CLK_T, RZKEY_T);
		maptoassign(CLK_U, RZKEY_U);
		maptoassign(CLK_V, RZKEY_V);
		maptoassign(CLK_W, RZKEY_W);
		maptoassign(CLK_X, RZKEY_X);
		maptoassign(CLK_Y, RZKEY_Y);
		maptoassign(CLK_Z, RZKEY_Z);
		maptoassign(CLK_NumLock, RZKEY_NUMLOCK);
		maptoassign(CLK_Keypad0, RZKEY_NUMPAD0);
		maptoassign(CLK_Keypad1, RZKEY_NUMPAD1);
		maptoassign(CLK_Keypad2, RZKEY_NUMPAD2);
		maptoassign(CLK_Keypad3, RZKEY_NUMPAD3);
		maptoassign(CLK_Keypad4, RZKEY_NUMPAD4);
		maptoassign(CLK_Keypad5, RZKEY_NUMPAD5);
		maptoassign(CLK_Keypad6, RZKEY_NUMPAD6);
		maptoassign(CLK_Keypad7, RZKEY_NUMPAD7);
		maptoassign(CLK_Keypad8, RZKEY_NUMPAD8);
		maptoassign(CLK_Keypad9, RZKEY_NUMPAD9);
		maptoassign(CLK_KeypadSlash, RZKEY_NUMPAD_DIVIDE);
		maptoassign(CLK_KeypadAsterisk, RZKEY_NUMPAD_MULTIPLY);
		maptoassign(CLK_KeypadMinus, RZKEY_NUMPAD_SUBTRACT);
		maptoassign(CLK_KeypadPlus, RZKEY_NUMPAD_ADD);
		maptoassign(CLK_KeypadEnter, RZKEY_NUMPAD_ENTER);
		maptoassign(CLK_KeypadPeriodAndDelete, RZKEY_NUMPAD_DECIMAL);
		maptoassign(CLK_PrintScreen, RZKEY_PRINTSCREEN);
		maptoassign(CLK_ScrollLock, RZKEY_SCROLL);
		maptoassign(CLK_PauseBreak, RZKEY_PAUSE);
		maptoassign(CLK_Insert, RZKEY_INSERT);
		maptoassign(CLK_Home, RZKEY_HOME);
		maptoassign(CLK_PageUp, RZKEY_PAGEUP);
		maptoassign(CLK_Delete, RZKEY_DELETE);
		maptoassign(CLK_End, RZKEY_END);
		maptoassign(CLK_PageDown, RZKEY_PAGEDOWN);
		maptoassign(CLK_UpArrow, RZKEY_UP);
		maptoassign(CLK_LeftArrow, RZKEY_LEFT);
		maptoassign(CLK_DownArrow, RZKEY_DOWN);
		maptoassign(CLK_RightArrow, RZKEY_RIGHT);
		maptoassign(CLK_Tab, RZKEY_TAB);
		maptoassign(CLK_CapsLock, RZKEY_CAPSLOCK);
		maptoassign(CLK_Backspace, RZKEY_BACKSPACE);
		maptoassign(CLK_Enter, RZKEY_ENTER);
		maptoassign(CLK_LeftCtrl, RZKEY_LCTRL);
		maptoassign(CLK_LeftGui, RZKEY_LWIN);
		maptoassign(CLK_LeftAlt, RZKEY_LALT);
		maptoassign(CLK_Space, RZKEY_SPACE);
		maptoassign(CLK_RightAlt, RZKEY_RALT);
		maptoassign(CLK_RightGui, RZKEY_FN); 
		maptoassign(CLK_Fn, RZKEY_FN); // This is actually the same key as RightGui on some keyboard variants
		maptoassign(CLK_Application, RZKEY_RMENU);
		maptoassign(CLK_RightCtrl, RZKEY_RCTRL);
		maptoassign(CLK_LeftShift, RZKEY_LSHIFT);
		maptoassign(CLK_RightShift, RZKEY_RSHIFT);
		maptoassign(CLK_GraveAccentAndTilde, RZKEY_OEM_1);
		maptoassign(CLK_EqualsAndPlus, RZKEY_OEM_3);
		maptoassign(CLK_MinusAndUnderscore, RZKEY_OEM_2);
		maptoassign(CLK_BracketLeft, RZKEY_OEM_4);
		maptoassign(CLK_BracketRight, RZKEY_OEM_5);
		maptoassign(CLK_Backslash, RZKEY_OEM_6);
		maptoassign(CLK_SemicolonAndColon, RZKEY_OEM_7);
		maptoassign(CLK_ApostropheAndDoubleQuote, RZKEY_OEM_8);
		maptoassign(CLK_CommaAndLessThan, RZKEY_OEM_9);
		maptoassign(CLK_PeriodAndBiggerThan, RZKEY_OEM_10);
		maptoassign(CLK_SlashAndQuestionMark, RZKEY_OEM_11);
		maptoassign(CLK_NonUsTilde, RZKEY_EUR_1);
		maptoassign(CLK_NonUsBackslash, RZKEY_EUR_2);
		maptoassign(CLK_Logo, RZLED_LOGO);
		maptoassign(CLI_Invalid, RZKEY_INVALID);
	default: {
		return false;
	}
	}
}

bool CorsairSDK::findMouseLed(const CorsairLedId ledid, int* row, int* col) {
	using namespace ChromaSDK::Mouse;

	switch (ledid) {
		maptoassign(CLI_Invalid, RZLED_NONE);
		maptoassign(CLM_1, RZLED2_LOGO);
		maptoassign(CLM_2, RZLED2_BOTTOM1);
		maptoassign(CLM_3, RZLED2_SCROLLWHEEL);
		maptoassign(CLM_4, RZLED2_BACKLIGHT);
	default:
		return false;
	}
}

ChromaSDK::Mouse::RZLED CorsairSDK::findMouseLed(const CorsairLedId ledid) {
	using namespace ChromaSDK::Mouse;

	switch (ledid) {
		mapto(CLI_Invalid, RZLED_NONE);
		mapto(CLM_1, RZLED_LOGO);
		mapto(CLM_2, RZLED_SIDE_STRIP1);
		mapto(CLM_3, RZLED_SCROLLWHEEL);
		mapto(CLM_4, RZLED_BACKLIGHT);
	default:
		return RZLED_NONE;
	}
}

CorsairLedId CorsairSDK::findMouseLed(ChromaSDK::Mouse::RZLED led) {
	using namespace ChromaSDK::Mouse;

	switch (led) {
		mapto(RZLED_NONE, CLI_Invalid);
		mapto(RZLED_LOGO, CLM_1);
		mapto(RZLED_SIDE_STRIP1, CLM_2);
		mapto(RZLED_SCROLLWHEEL, CLM_3);
		mapto(RZLED_BACKLIGHT, CLM_4);
	default:
		return CLI_Invalid;
	}
}


int CorsairSDK::findMousepadLed(const CorsairLedId ledid) {
	using namespace ChromaSDK::Mousepad;
	return (ledid - CLMM_Zone1);
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
