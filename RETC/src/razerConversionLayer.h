#pragma once
#include "stdafx.h"

#include <fstream>
#include "RzChromaSDKTypes.h"
#include "CUESDK.h"
#include <sstream>

#include "razerKeysToCorsair.h"

#ifdef _DEBUG
#define LOGE2(x,y) std::cout << "ERRO : " << "[" << __FILE__ << "][" << __FUNCTION__ << "][Line " << __LINE__ << "] " << x <<":"<< y <<std::endl;
#define LOGI2(x,y) std::cout << "INFO : " << "[" << __FILE__ << "][" << __FUNCTION__ << "][Line " << __LINE__ << "] " << x <<":"<< y << std::endl;
#define LOGD2(x,y) std::cout << "DEBG : " << "[" << __FILE__ << "][" << __FUNCTION__ << "][Line " << __LINE__ << "] " << x <<":"<< y << std::endl;
#define LOGE(x) std::cout << "ERRO : " << "[" << __FILE__ << "][" << __FUNCTION__ << "][Line " << __LINE__ << "] " << x << std::endl;
#define LOGI(x) std::cout << "INFO : " << "[" << __FILE__ << "][" << __FUNCTION__ << "][Line " << __LINE__ << "] " << x << std::endl;
#define LOGD(x) std::cout << "DEBG : " << "[" << __FILE__ << "][" << __FUNCTION__ << "][Line " << __LINE__ << "] " << x << std::endl;
#else
#define LOGE2(x,y) NULL
#define LOGI2(x,y) NULL
#define LOGD2(x,y) NULL
#define LOGE(x) NULL
#define LOGI(x) NULL
#define LOGD(x) NULL
#endif

struct out_redir {
	explicit out_redir(std::streambuf* original_buf) : buf_(original_buf) { }

	~out_redir() {
		std::cerr.rdbuf(buf_);
	}

private:
	std::streambuf* buf_;
};

inline const std::string guidToString(REFGUID guid)
{
	char szGuid[40] = { 0 };
	sprintf_s(szGuid, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
	return szGuid;
}

template <typename Type>
inline const std::string ptrStr(Type ptr) {
	std::ostringstream adr;
	adr << (void const *)ptr;
	return adr.str();
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
	bool connect();
	bool createKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE effectType, PRZPARAM effectData, RZEFFECTID * razerEffectGuid);
	bool setActiveEffect(RZEFFECTID razerEffectGuid);
	bool deleteEffect(RZEFFECTID razerEffectGuid);

private:
	void logError(const std::stringstream &errorMsg);
	static std::string corsairErrorToString(const CorsairError error);
	static bool createUniqueEffectID(RZEFFECTID *guid);


	bool playKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE type, PRZPARAM Effect);
public:
	void destroy();


//Member attributes
private:
	//CorsairLedPositions* m_referenceLedMap;
	std::ofstream m_logOutputStream;

	std::map <RZEFFECTID, PRZPARAM, RZEFFECTIDCmp> m_Effects;
	RZEFFECTID m_lastActiveEffect;
};

