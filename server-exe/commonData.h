#pragma once
#include <RzChromaSDKTypes.h>
#include "../rpc-midl/rpc_retc.h"

namespace LookupArrays {
	using namespace ChromaSDK;
	const int genericEffectType[MAX][CHROMA_RESERVED] = {
		{Keyboard::CHROMA_NONE ,Keyboard::CHROMA_WAVE, Keyboard::CHROMA_SPECTRUMCYCLING, Keyboard::CHROMA_BREATHING, -1 , Keyboard::CHROMA_REACTIVE, Keyboard::CHROMA_STATIC, Keyboard::CHROMA_CUSTOM},
		{Mouse::CHROMA_NONE, Mouse::CHROMA_WAVE, Mouse::CHROMA_SPECTRUMCYCLING, Mouse::CHROMA_BREATHING, Mouse::CHROMA_BLINKING, Mouse::CHROMA_REACTIVE, Mouse::CHROMA_STATIC, Mouse::CHROMA_CUSTOM2},
		{Headset::CHROMA_NONE, -1, Headset::CHROMA_SPECTRUMCYCLING, Headset::CHROMA_BREATHING, -1, -1, Headset::CHROMA_STATIC, Headset::CHROMA_CUSTOM}, // Headset
		{Mousepad::CHROMA_NONE, Mousepad::CHROMA_WAVE, Mousepad::CHROMA_SPECTRUMCYCLING, Mousepad::CHROMA_BREATHING, -1, -1, Mousepad::CHROMA_STATIC, Mousepad::CHROMA_CUSTOM}, // Mousepad
		{Keypad::CHROMA_NONE, Keypad::CHROMA_WAVE, Keypad::CHROMA_SPECTRUMCYCLING, Keypad::CHROMA_BREATHING, -1, Keypad::CHROMA_REACTIVE, Keypad::CHROMA_STATIC, Keypad::CHROMA_CUSTOM} // Keypad
	};

	const unsigned long effectSize[MAX][CHROMA_RESERVED] = {
		{sizeof(WAVE_EFFECT_TYPE), sizeof(SPECTRUMCYCLING_EFFECT_TYPE), sizeof(BREATHING_EFFECT_TYPE), sizeof(BLINKING_EFFECT_TYPE), sizeof(REACTIVE_EFFECT_TYPE), sizeof(STATIC_EFFECT_TYPE), sizeof(CUSTOM_EFFECT_TYPE), 0},
		{sizeof(Keyboard::BREATHING_EFFECT_TYPE), sizeof(Keyboard::CUSTOM_EFFECT_TYPE), sizeof(Keyboard::REACTIVE_EFFECT_TYPE), sizeof(Keyboard::STATIC_EFFECT_TYPE), sizeof(SPECTRUMCYCLING_EFFECT_TYPE), sizeof(WAVE_EFFECT_TYPE), 0, sizeof(Keyboard::CUSTOM_KEY_EFFECT_TYPE)},
		{sizeof(Mouse::BLINKING_EFFECT_TYPE), sizeof(Keyboard::BREATHING_EFFECT_TYPE), sizeof(Keyboard::CUSTOM_EFFECT_TYPE), sizeof(Keyboard::REACTIVE_EFFECT_TYPE), sizeof(SPECTRUMCYCLING_EFFECT_TYPE), sizeof(STATIC_EFFECT_TYPE), sizeof(WAVE_EFFECT_TYPE), sizeof(Mouse::CUSTOM_EFFECT_TYPE2)},
		{sizeof(Headset::STATIC_EFFECT_TYPE), sizeof(Headset::BREATHING_EFFECT_TYPE), sizeof(SPECTRUMCYCLING_EFFECT_TYPE), sizeof(Headset::CUSTOM_EFFECT_TYPE), 0, 0, 0, 0},
		{sizeof(Mousepad::BREATHING_EFFECT_TYPE), sizeof(Mousepad::CUSTOM_EFFECT_TYPE), sizeof(SPECTRUMCYCLING_EFFECT_TYPE), sizeof(Mousepad::STATIC_EFFECT_TYPE), sizeof(Mousepad::WAVE_EFFECT_TYPE), 0, 0, 0},
		{sizeof(Keypad::BREATHING_EFFECT_TYPE), sizeof(Keypad::CUSTOM_EFFECT_TYPE), sizeof(Keypad::REACTIVE_EFFECT_TYPE), sizeof(SPECTRUMCYCLING_EFFECT_TYPE), sizeof(Keypad::STATIC_EFFECT_TYPE), sizeof(Keypad::WAVE_EFFECT_TYPE), 0, 0}
	};

}
