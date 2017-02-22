#pragma once
#include "RzChromaSDKDefines.h"
#include "RzChromaSDKTypes.h"
#include "CUESDK.h"
#include "helpers.h"

inline const CorsairLedColor convertLedColor(const COLORREF & color)
{
	CorsairLedColor ledColor;
	ledColor.r = GetRValue(color);
	ledColor.g = GetGValue(color);
	ledColor.b = GetBValue(color);
	ledColor.ledId = CLI_Invalid;

	return ledColor;
}

inline const bool findKeyboardLed(CorsairLedId ledid, int *row, int *col) {
	using namespace ChromaSDK::Keyboard;
	int val;
	switch (ledid) {
			maptoassign(CLK_Escape, RZKEY_ESC)
			maptoassign(CLK_F1, RZKEY_F1)
			maptoassign(CLK_F2, RZKEY_F2)
			maptoassign(CLK_F3, RZKEY_F3)
			maptoassign(CLK_F4, RZKEY_F4)
			maptoassign(CLK_F5, RZKEY_F5)
			maptoassign(CLK_F6, RZKEY_F6)
			maptoassign(CLK_F7, RZKEY_F7)
			maptoassign(CLK_F8, RZKEY_F8)
			maptoassign(CLK_F9, RZKEY_F9)
			maptoassign(CLK_F10, RZKEY_F10)
			maptoassign(CLK_F11, RZKEY_F11)
			maptoassign(CLK_F12, RZKEY_F12)
			maptoassign(CLK_1, RZKEY_1)
			maptoassign(CLK_2, RZKEY_2)
			maptoassign(CLK_3, RZKEY_3)
			maptoassign(CLK_4, RZKEY_4)
			maptoassign(CLK_5, RZKEY_5)
			maptoassign(CLK_6, RZKEY_6)
			maptoassign(CLK_7, RZKEY_7)
			maptoassign(CLK_8, RZKEY_8)
			maptoassign(CLK_9, RZKEY_9)
			maptoassign(CLK_0, RZKEY_0)
			maptoassign(CLK_A, RZKEY_A)
			maptoassign(CLK_B, RZKEY_B)
			maptoassign(CLK_C, RZKEY_C)
			maptoassign(CLK_D, RZKEY_D)
			maptoassign(CLK_E, RZKEY_E)
			maptoassign(CLK_F, RZKEY_F)
			maptoassign(CLK_G, RZKEY_G)
			maptoassign(CLK_H, RZKEY_H)
			maptoassign(CLK_I, RZKEY_I)
			maptoassign(CLK_J, RZKEY_J)
			maptoassign(CLK_K, RZKEY_K)
			maptoassign(CLK_L, RZKEY_L)
			maptoassign(CLK_M, RZKEY_M)
			maptoassign(CLK_N, RZKEY_N)
			maptoassign(CLK_O, RZKEY_O)
			maptoassign(CLK_P, RZKEY_P)
			maptoassign(CLK_Q, RZKEY_Q)
			maptoassign(CLK_R, RZKEY_R)
			maptoassign(CLK_S, RZKEY_S)
			maptoassign(CLK_T, RZKEY_T)
			maptoassign(CLK_U, RZKEY_U)
			maptoassign(CLK_V, RZKEY_V)
			maptoassign(CLK_W, RZKEY_W)
			maptoassign(CLK_X, RZKEY_X)
			maptoassign(CLK_Y, RZKEY_Y)
			maptoassign(CLK_Z, RZKEY_Z)
			maptoassign(CLK_NumLock, RZKEY_NUMLOCK)
			maptoassign(CLK_Keypad0, RZKEY_NUMPAD0)
			maptoassign(CLK_Keypad1, RZKEY_NUMPAD1)
			maptoassign(CLK_Keypad2, RZKEY_NUMPAD2)
			maptoassign(CLK_Keypad3, RZKEY_NUMPAD3)
			maptoassign(CLK_Keypad4, RZKEY_NUMPAD4)
			maptoassign(CLK_Keypad5, RZKEY_NUMPAD5)
			maptoassign(CLK_Keypad6, RZKEY_NUMPAD6)
			maptoassign(CLK_Keypad7, RZKEY_NUMPAD7)
			maptoassign(CLK_Keypad8, RZKEY_NUMPAD8)
			maptoassign(CLK_Keypad9, RZKEY_NUMPAD9)
			maptoassign(CLK_KeypadSlash, RZKEY_NUMPAD_DIVIDE)
			maptoassign(CLK_KeypadAsterisk, RZKEY_NUMPAD_MULTIPLY)
			maptoassign(CLK_KeypadMinus, RZKEY_NUMPAD_SUBTRACT)
			maptoassign(CLK_KeypadPlus, RZKEY_NUMPAD_ADD)
			maptoassign(CLK_KeypadEnter, RZKEY_NUMPAD_ENTER)
			maptoassign(CLK_KeypadPeriodAndDelete, RZKEY_NUMPAD_DECIMAL)
			maptoassign(CLK_PrintScreen, RZKEY_PRINTSCREEN)
			maptoassign(CLK_ScrollLock, RZKEY_SCROLL)
			maptoassign(CLK_PauseBreak, RZKEY_PAUSE)
			maptoassign(CLK_Insert, RZKEY_INSERT)
			maptoassign(CLK_Home, RZKEY_HOME)
			maptoassign(CLK_PageUp, RZKEY_PAGEUP)
			maptoassign(CLK_Delete, RZKEY_DELETE)
			maptoassign(CLK_End, RZKEY_END)
			maptoassign(CLK_PageDown, RZKEY_PAGEDOWN)
			maptoassign(CLK_UpArrow, RZKEY_UP)
			maptoassign(CLK_LeftArrow, RZKEY_LEFT)
			maptoassign(CLK_DownArrow, RZKEY_DOWN)
			maptoassign(CLK_RightArrow, RZKEY_RIGHT)
			maptoassign(CLK_Tab, RZKEY_TAB)
			maptoassign(CLK_CapsLock, RZKEY_CAPSLOCK)
			maptoassign(CLK_Backspace, RZKEY_BACKSPACE)
			maptoassign(CLK_Enter, RZKEY_ENTER)
			maptoassign(CLK_LeftCtrl, RZKEY_LCTRL)
			maptoassign(CLK_LeftGui, RZKEY_LWIN)
			maptoassign(CLK_LeftAlt, RZKEY_LALT)
			maptoassign(CLK_Space, RZKEY_SPACE)
			maptoassign(CLK_RightAlt, RZKEY_RALT)
			maptoassign(CLK_RightGui, RZKEY_FN)
			maptoassign(CLK_Application, RZKEY_RMENU)
			maptoassign(CLK_RightCtrl, RZKEY_RCTRL)
			maptoassign(CLK_LeftShift, RZKEY_LSHIFT)
			maptoassign(CLK_RightShift, RZKEY_RSHIFT)
			maptoassign(CLK_G1, RZKEY_MACRO1)
			maptoassign(CLK_G2, RZKEY_MACRO2)
			maptoassign(CLK_G3, RZKEY_MACRO3)
			maptoassign(CLK_G4, RZKEY_MACRO4)
			maptoassign(CLK_G5, RZKEY_MACRO5)
			maptoassign(CLK_GraveAccentAndTilde, RZKEY_OEM_1)
			maptoassign(CLK_EqualsAndPlus, RZKEY_OEM_2)
			maptoassign(CLK_MinusAndUnderscore, RZKEY_OEM_3)
			maptoassign(CLK_BracketLeft, RZKEY_OEM_4)
			maptoassign(CLK_BracketRight, RZKEY_OEM_5)
			maptoassign(CLK_Backslash, RZKEY_OEM_6)
			maptoassign(CLK_SemicolonAndColon, RZKEY_OEM_7)
			maptoassign(CLK_ApostropheAndDoubleQuote, RZKEY_OEM_8)
			maptoassign(CLK_CommaAndLessThan, RZKEY_OEM_9)
			maptoassign(CLK_PeriodAndBiggerThan, RZKEY_OEM_10)
			maptoassign(CLK_SlashAndQuestionMark, RZKEY_OEM_11) //Not sure
			maptoassign(CLK_NonUsTilde, RZKEY_EUR_1)
			maptoassign(CLK_NonUsBackslash, RZKEY_EUR_2)
			maptoassign(CLK_Logo, RZLED_LOGO)
			maptoassign(CLI_Invalid, RZKEY_INVALID)
	default:
		return false;
	}

	*row = HIBYTE(val);
	*col = LOBYTE(val);

	return true;
}

inline const bool findMouseLed(const CorsairLedId ledid, int *row, int *col) 
{
	using namespace ChromaSDK::Mouse;
	int val;

	switch (ledid) {
		maptoassign(CLI_Invalid, RZLED_NONE)
		maptoassign(CLM_1, RZLED2_LOGO)
		maptoassign(CLM_2, RZLED2_BOTTOM1) // Sloppy
		maptoassign(CLM_3, RZLED2_SCROLLWHEEL)
		maptoassign(CLM_4, RZLED2_BACKLIGHT)
	default:
		return false;
	}

	*row = HIBYTE(val);
	*col = LOBYTE(val);

	return true;
}

inline const int findMouseLed(const CorsairLedId ledid)
{
	using namespace ChromaSDK::Mouse;

	switch (ledid) {
			mapto(CLI_Invalid, RZLED_NONE)
			mapto(CLM_1, RZLED_LOGO)
			mapto(CLM_2, RZLED_SIDE_STRIP1) // Sloppy
			mapto(CLM_3, RZLED_SCROLLWHEEL)
			mapto(CLM_4, RZLED_BACKLIGHT)
	default:
		return -1;
	}
}

inline const CorsairLedId findMouseLed(ChromaSDK::Mouse::RZLED led)
{
	using namespace ChromaSDK::Mouse;

	switch (led) {
		mapto(RZLED_NONE, CLI_Invalid)
			mapto(RZLED_LOGO, CLM_1)
			mapto(RZLED_SIDE_STRIP1, CLM_2) // Sloppy
			mapto(RZLED_SCROLLWHEEL, CLM_3)
			mapto(RZLED_BACKLIGHT, CLM_4)
	default:
		return CLI_Invalid;
	}
}