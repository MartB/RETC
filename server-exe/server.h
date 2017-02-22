#pragma once
typedef struct effectData {
	int id;
	DEVICE_TYPE_RETC deviceType;
	unsigned long size;
	char* data;
} effectData;

namespace std {
	template<> struct hash<GUID>
	{
		size_t operator()(const GUID& guid) const noexcept {
			const std::uint64_t* p = reinterpret_cast<const std::uint64_t*>(&guid);
			std::hash<std::uint64_t> hash;
			return hash(p[0]) ^ hash(p[1]);
		}
	};
}

int effectTypeLookupArray[DEVICE_TYPE_RETC::KEYPAD + 1][ChromaSDK::CHROMA_CUSTOM + 1] = {
	{ ChromaSDK::Keyboard::CHROMA_NONE ,ChromaSDK::Keyboard::CHROMA_WAVE, ChromaSDK::Keyboard::CHROMA_SPECTRUMCYCLING, ChromaSDK::Keyboard::CHROMA_BREATHING, ChromaSDK::Keyboard::CHROMA_INVALID, ChromaSDK::Keyboard::CHROMA_REACTIVE, ChromaSDK::Keyboard::CHROMA_STATIC, ChromaSDK::Keyboard::CHROMA_CUSTOM },
	{ ChromaSDK::Mouse::CHROMA_NONE, ChromaSDK::Mouse::CHROMA_WAVE, ChromaSDK::Mouse::CHROMA_SPECTRUMCYCLING, ChromaSDK::Mouse::CHROMA_BREATHING, ChromaSDK::Mouse::CHROMA_BLINKING, ChromaSDK::Mouse::CHROMA_REACTIVE,	ChromaSDK::Mouse::CHROMA_STATIC, ChromaSDK::Mouse::CHROMA_CUSTOM2 },
	{ ChromaSDK::Headset::CHROMA_NONE, -1, ChromaSDK::Headset::CHROMA_SPECTRUMCYCLING, ChromaSDK::Headset::CHROMA_BREATHING, -1, -1, ChromaSDK::Headset::CHROMA_STATIC, ChromaSDK::Headset::CHROMA_CUSTOM }, // Headset
	{ -1 }, // Mousepad
	{ -1 } // Keypad
};
