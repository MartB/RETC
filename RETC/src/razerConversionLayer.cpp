#include "razerConversionLayer.h"

#define mkwordhilo(hi,lo) ((hi << 8) | lo); 

std::string razerConversionLayer::corsairErrorToString(const CorsairError error)
{
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

razerConversionLayer::razerConversionLayer()
{
	m_logOutputStream = std::ofstream("RzToCue.log", std::ofstream::out);
#ifdef _DEBUG
	out_redir red(std::cerr.rdbuf());
	std::cout.rdbuf(m_logOutputStream.rdbuf());  // Redirect cout to log
	std::cerr.rdbuf(m_logOutputStream.rdbuf());  // Redirect cerr to log
#endif

	m_lastActiveEffect = GUID();
}

void razerConversionLayer::logError(const std::stringstream &errorMsg)
{
	// Fail silently bcs we have no filestream open
	if (!m_logOutputStream.is_open())
		return;

	m_logOutputStream << errorMsg.str() << std::endl;
	m_logOutputStream.flush();
}

bool razerConversionLayer::createUniqueEffectID(RZEFFECTID *guid)
{
	return CoCreateGuid(guid) == S_OK;
}

bool razerConversionLayer::connect()
{
	std::stringstream errorMsg;
	CorsairPerformProtocolHandshake();

	if (const auto error = CorsairGetLastError()) {
		errorMsg << "CorsairPerformProtocolHandshake failed: " << corsairErrorToString(error).c_str();
		logError(errorMsg);
		return false;
	}

	bool gotExclusiveControl = CorsairRequestControl(CAM_ExclusiveLightingControl);
	if (!gotExclusiveControl)
		return false;

	/*m_referenceLedMap = CorsairGetLedPositions();

	if (!m_referenceLedMap)	{
		errorMsg << "CorsairGetLedPositions failed: " << corsairErrorToString(CorsairGetLastError()).c_str();
		logError(errorMsg);
		return false;
	}*/

	return true;
}


bool razerConversionLayer::createKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE effectType, PRZPARAM effectData, RZEFFECTID * razerEffectGuid)
{
	std::stringstream errorMsg;

	bool bStoreEffect = (razerEffectGuid == nullptr) ? false : true;
	bool bPlayEffect = (razerEffectGuid == nullptr) ? true : false;

	PRZPARAM* pParam2 = nullptr;
	size_t iSize = 0;

	switch (effectType)
	{
	case ChromaSDK::Keyboard::CHROMA_STATIC:
		iSize = sizeof(ChromaSDK::Keyboard::STATIC_EFFECT_TYPE);
		pParam2 = (bStoreEffect) ? (PRZPARAM*)new ChromaSDK::Keyboard::STATIC_EFFECT_TYPE : 0;
		break;
	case ChromaSDK::Keyboard::CHROMA_NONE:
		bPlayEffect = true;
		break;

	case ChromaSDK::Keyboard::CHROMA_CUSTOM:
		iSize = sizeof(ChromaSDK::Keyboard::CUSTOM_EFFECT_TYPE);
		pParam2 = (bStoreEffect) ? (PRZPARAM*)new ChromaSDK::Keyboard::CUSTOM_EFFECT_TYPE : 0;
		break;
	case ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY:
		bPlayEffect = true;
		bStoreEffect = false;
		break;
	default:
		errorMsg << "Invalid Keyboard effect Type:" << effectType << " not implemented.";
		logError(errorMsg);
		delete pParam2;
		return false;
	}


	if (bPlayEffect)
		playKeyboardEffect(effectType, effectData);

	if (!bStoreEffect)
		return true;

	RZEFFECTID newEffectID;
	createUniqueEffectID(&newEffectID);

	if (effectData != nullptr)
		memcpy(pParam2, effectData, iSize);

	m_Effects.insert(std::make_pair(newEffectID, pParam2));
	*razerEffectGuid = newEffectID;

	return true;
}


bool razerConversionLayer::setActiveEffect(RZEFFECTID razerEffectGuid)
{
	if (razerEffectGuid == m_lastActiveEffect)
		return true;

	for (auto &effect : m_Effects) {
		if (effect.first == razerEffectGuid) {
			if (razerEffectGuid != GUID()) {
				m_lastActiveEffect = razerEffectGuid;
			}
			return playKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE::CHROMA_CUSTOM, effect.second);
		}
	}

	return false;
}

bool razerConversionLayer::deleteEffect(RZEFFECTID razerEffectGuid)
{
	auto itr = m_Effects.begin();
	while (itr != m_Effects.end()) {
		if (itr->first != razerEffectGuid) {
			delete itr->second;
			itr = m_Effects.erase(itr);
			return true;
		}
		
		++itr;
	}

	return false;
}

bool razerConversionLayer::playKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE type, PRZPARAM Effect)
{
	std::vector<CorsairLedColor> vec;
	vec.reserve(MAX_COLUMN * MAX_ROW);

	/************************************************************************/
	/* Missing effects for keyboards: CHROMA_BREATHING, CHROMA_REACTIVE,	*/
	/* CHROMA_SPECTRUMCYCLING, CHROMA_WAVE, CHROMA_STARLIGHT(WIP ?)			*/
	/************************************************************************/

	if (type == ChromaSDK::Keyboard::CHROMA_STATIC)
	{
		ChromaSDK::Keyboard::STATIC_EFFECT_TYPE custEffect = (Effect) ? *(struct ChromaSDK::Keyboard::STATIC_EFFECT_TYPE*)Effect : ChromaSDK::Keyboard::STATIC_EFFECT_TYPE{ 0 };
		COLORREF origColor = custEffect.Color;
		if (origColor == NULL)
			origColor = custEffect.Color;

		CorsairLedColor ledColor = convertLedColor(origColor);
		for (int row = 0; row < MAX_ROW; row++)
			for (int col = 0; col < MAX_COLUMN; col++) {
				ledColor.ledId = findCorrespondingLed(row, col);
				if (ledColor.ledId != CLI_Invalid)
					vec.push_back(ledColor);
			}
	}
	else if (type == ChromaSDK::Keyboard::CHROMA_CUSTOM_KEY)
	{
		ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE custEffect = (Effect) ? *(struct ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE*)Effect : ChromaSDK::Keyboard::CUSTOM_KEY_EFFECT_TYPE{ 0 };

		for (int row = 0; row < MAX_ROW; row++)
			for (int col = 0; col < MAX_COLUMN; col++) {
				//////////////////////////////////////////////////////////////////////////
				// It is possible that we do not have a key value in this struct.       //
				//////////////////////////////////////////////////////////////////////////
				COLORREF origColor = custEffect.Key[row][col];
				if (origColor == NULL)
					origColor = custEffect.Color[row][col];

				CorsairLedColor ledColor = convertLedColor(origColor);
				ledColor.ledId = findCorrespondingLed(row, col);

				if (ledColor.ledId != CLI_Invalid)
					vec.push_back(ledColor);
			}
	}
	else if (type == ChromaSDK::Keyboard::CHROMA_CUSTOM || type == ChromaSDK::Keyboard::CHROMA_NONE)
	{
		ChromaSDK::Keyboard::CUSTOM_EFFECT_TYPE custEffect = (Effect) ? *(struct ChromaSDK::Keyboard::CUSTOM_EFFECT_TYPE*)Effect : ChromaSDK::Keyboard::CUSTOM_EFFECT_TYPE{ 0 };

		for (int row = 0; row < MAX_ROW; row++)
			for (int col = 0; col < MAX_COLUMN; col++) {
				const COLORREF origColor = custEffect.Color[row][col];
				CorsairLedColor ledColor = convertLedColor(origColor);
				ledColor.ledId = findCorrespondingLed(row, col);

				if (ledColor.ledId != CLI_Invalid)
					vec.push_back(ledColor);
			}
	}

	CorsairSetLedsColorsAsync(vec.size(), vec.data(),nullptr,nullptr);

	if (const auto error = CorsairGetLastError()) {
		std::stringstream errorMsg;
		errorMsg << "playKeyboardEffect failed: " << corsairErrorToString(error);
		logError(errorMsg);
		return false;
	}

	return true;
}

void razerConversionLayer::destroy()
{
	auto itr = m_Effects.begin();
	while (itr != m_Effects.end()) {
		delete itr->second;
		itr = m_Effects.erase(itr);
	}

	m_logOutputStream.flush();
	m_logOutputStream.close();

	CorsairReleaseControl(CAM_ExclusiveLightingControl);
}
