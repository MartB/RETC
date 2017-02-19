#pragma once
#include <map>
#include "RzChromaSDKDefines.h"
#include "RzChromaSDKTypes.h"
#include "CUESDK.h"
#include <wingdi.h>

using namespace ChromaSDK::Keyboard;
const std::map<RZKEY, CorsairLedId> customQWERTZKeymap = {
	{ RZKEY_ESC, CLK_Escape },
	{ RZKEY_F1, CLK_F1 },
	{ RZKEY_F2, CLK_F2 },
	{ RZKEY_F3, CLK_F3 },
	{ RZKEY_F4, CLK_F4 },
	{ RZKEY_F5, CLK_F5 },
	{ RZKEY_F6, CLK_F6 },
	{ RZKEY_F7, CLK_F7 },
	{ RZKEY_F8, CLK_F8 },
	{ RZKEY_F9, CLK_F9 },
	{ RZKEY_F10, CLK_F10 },
	{ RZKEY_F11, CLK_F11 },
	{ RZKEY_F12, CLK_F12 },
	{ RZKEY_1, CLK_1 },
	{ RZKEY_2, CLK_2 },
	{ RZKEY_3, CLK_3 },
	{ RZKEY_4, CLK_4 },
	{ RZKEY_5, CLK_5 },
	{ RZKEY_6, CLK_6 },
	{ RZKEY_7, CLK_7 },
	{ RZKEY_8, CLK_8 },
	{ RZKEY_9, CLK_9 },
	{ RZKEY_0, CLK_0 },
	{ RZKEY_A, CLK_A },
	{ RZKEY_B, CLK_B },
	{ RZKEY_C, CLK_C },
	{ RZKEY_D, CLK_D },
	{ RZKEY_E, CLK_E },
	{ RZKEY_F, CLK_F },
	{ RZKEY_G, CLK_G },
	{ RZKEY_H, CLK_H },
	{ RZKEY_I, CLK_I },
	{ RZKEY_J, CLK_J },
	{ RZKEY_K, CLK_K },
	{ RZKEY_L, CLK_L },
	{ RZKEY_M, CLK_M },
	{ RZKEY_N, CLK_N },
	{ RZKEY_O, CLK_O },
	{ RZKEY_P, CLK_P },
	{ RZKEY_Q, CLK_Q },
	{ RZKEY_R, CLK_R },
	{ RZKEY_S, CLK_S },
	{ RZKEY_T, CLK_T },
	{ RZKEY_U, CLK_U },
	{ RZKEY_V, CLK_V },
	{ RZKEY_W, CLK_W },
	{ RZKEY_X, CLK_X },
	{ RZKEY_Y, CLK_Y },
	{ RZKEY_Z, CLK_Z },
	{ RZKEY_NUMLOCK, CLK_NumLock },
	{ RZKEY_NUMPAD0, CLK_Keypad0 },
	{ RZKEY_NUMPAD1, CLK_Keypad1 },
	{ RZKEY_NUMPAD2, CLK_Keypad2 },
	{ RZKEY_NUMPAD3, CLK_Keypad3 },
	{ RZKEY_NUMPAD4, CLK_Keypad4 },
	{ RZKEY_NUMPAD5, CLK_Keypad5 },
	{ RZKEY_NUMPAD6, CLK_Keypad6 },
	{ RZKEY_NUMPAD7, CLK_Keypad7 },
	{ RZKEY_NUMPAD8, CLK_Keypad8 },
	{ RZKEY_NUMPAD9, CLK_Keypad9 },
	{ RZKEY_NUMPAD_DIVIDE, CLK_KeypadSlash },
	{ RZKEY_NUMPAD_MULTIPLY, CLK_KeypadAsterisk },
	{ RZKEY_NUMPAD_SUBTRACT, CLK_KeypadMinus },
	{ RZKEY_NUMPAD_ADD, CLK_KeypadPlus },
	{ RZKEY_NUMPAD_ENTER, CLK_KeypadEnter },
	{ RZKEY_NUMPAD_DECIMAL, CLK_KeypadPeriodAndDelete },
	{ RZKEY_PRINTSCREEN, CLK_PrintScreen },
	{ RZKEY_SCROLL, CLK_ScrollLock },
	{ RZKEY_PAUSE, CLK_PauseBreak },
	{ RZKEY_INSERT, CLK_Insert },
	{ RZKEY_HOME, CLK_Home },
	{ RZKEY_PAGEUP, CLK_PageUp },
	{ RZKEY_DELETE, CLK_Delete },
	{ RZKEY_END, CLK_End },
	{ RZKEY_PAGEDOWN, CLK_PageDown },
	{ RZKEY_UP, CLK_UpArrow },
	{ RZKEY_LEFT, CLK_LeftArrow },
	{ RZKEY_DOWN, CLK_DownArrow },
	{ RZKEY_RIGHT, CLK_RightArrow },
	{ RZKEY_TAB, CLK_Tab },
	{ RZKEY_CAPSLOCK, CLK_CapsLock },
	{ RZKEY_BACKSPACE, CLK_Backspace },
	{ RZKEY_ENTER, CLK_Enter },
	{ RZKEY_LCTRL, CLK_LeftCtrl },
	{ RZKEY_LWIN, CLK_LeftGui },
	{ RZKEY_LALT, CLK_LeftAlt },
	{ RZKEY_SPACE, CLK_Space },
	{ RZKEY_RALT, CLK_RightAlt },
	{ RZKEY_FN, CLK_RightGui },
	{ RZKEY_RMENU, CLK_Application },
	{ RZKEY_RCTRL, CLK_RightCtrl },
	{ RZKEY_LSHIFT, CLK_LeftShift },
	{ RZKEY_RSHIFT, CLK_RightShift },
	{ RZKEY_MACRO1, CLK_G1 },
	{ RZKEY_MACRO2, CLK_G2 },
	{ RZKEY_MACRO3, CLK_G3 },
	{ RZKEY_MACRO4, CLK_G4 },
	{ RZKEY_MACRO5, CLK_G5 },
	{ RZKEY_OEM_1, CLK_GraveAccentAndTilde },
	{ RZKEY_OEM_2, CLK_EqualsAndPlus },
	{ RZKEY_OEM_3, CLK_MinusAndUnderscore },
	{ RZKEY_OEM_4, CLK_BracketLeft },
	{ RZKEY_OEM_5, CLK_BracketRight },
	{ RZKEY_OEM_6, CLK_Backslash },
	{ RZKEY_OEM_7, CLK_SemicolonAndColon },
	{ RZKEY_OEM_8, CLK_ApostropheAndDoubleQuote },
	{ RZKEY_OEM_9, CLK_CommaAndLessThan },
	{ RZKEY_OEM_10,CLK_PeriodAndBiggerThan },
	{ RZKEY_OEM_11,CLK_SlashAndQuestionMark }, //Not sure
	{ RZKEY_EUR_1, CLK_NonUsTilde },
	{ RZKEY_EUR_2, CLK_NonUsBackslash },
	{ RZKEY_JPN_1, CLI_Invalid },
	{ RZKEY_JPN_2, CLI_Invalid },
	{ RZKEY_JPN_3, CLI_Invalid },
	{ RZKEY_JPN_4, CLI_Invalid },
	{ RZKEY_JPN_5, CLI_Invalid },
	{ RZKEY_KOR_1, CLI_Invalid },
	{ RZKEY_KOR_2, CLK_NonUsTilde },
	{ RZKEY_KOR_3, CLI_Invalid },
	{ RZKEY_KOR_4, CLI_Invalid },
	{ RZKEY_KOR_5, CLI_Invalid },
	{ RZKEY_KOR_6, CLI_Invalid },
	{ RZKEY_KOR_7, CLI_Invalid },
	{ RZKEY_INVALID, CLI_Invalid },
};

#define mkwordhilo(hi,lo) ((hi << 8) | lo); 

inline const CorsairLedColor convertLedColor(const COLORREF & color)
{
	CorsairLedColor ledColor;
	ledColor.r = GetRValue(color);
	ledColor.g = GetGValue(color);
	ledColor.b = GetBValue(color);
	ledColor.ledId = CLI_Invalid;

	return ledColor;
}

inline const CorsairLedId findCorrespondingLed(const int row, const int col)
{
	RZKEY key = (RZKEY)mkwordhilo(row, col);
	auto it = customQWERTZKeymap.find(key);
	if (it != customQWERTZKeymap.end())
		return it->second;

	return CLI_Invalid;
}