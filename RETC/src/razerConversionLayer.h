#pragma once
#include "stdafx.h"

#include <fstream>
#include <sstream>
#include "RzChromaSDKDefines.h"
#include "RzErrors.h"
#include "RzChromaSDKTypes.h"
#include "CUESDK.h"

#include "razerKeysToCorsair.h"

inline const std::string guidToString(REFGUID guid)
{
	char szGuid[40] = { 0 };
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

class razerConversionLayer
{
	// Custom compare function for GUIDs
	struct RZEFFECTIDCmp
	{
		bool operator()(const GUID & Left, const GUID & Right) const {
			return memcmp(&Left, &Right, sizeof(Right)) < 0;
		}
	};

public:
	razerConversionLayer();
	~razerConversionLayer();
	bool connect();
	bool createKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE effectType, PRZPARAM effectData, RZEFFECTID * razerEffectGuid);
	bool setActiveEffect(RZEFFECTID razerEffectGuid);
	bool deleteEffect(RZEFFECTID razerEffectGuid);

private:
	void logError(const std::stringstream &errorMsg);
	const std::string corsairErrorToString(const CorsairError error);
	const RZEFFECTID createUniqueEffectID();

	bool playKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE type, PRZPARAM Effect);
	void destroy();


//Member attributes
private:
	CorsairLedPositions* m_referenceLedMap;
	std::ofstream m_logOutputStream;

	std::map <RZEFFECTID, PRZPARAM, RZEFFECTIDCmp> m_Effects;
	RZEFFECTID m_lastActiveEffect;
};

