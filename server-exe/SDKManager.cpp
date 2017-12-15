// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "SDKManager.h"
#include "RzErrors.h"

SDKManager::SDKManager() {
	m_sdkLoader = std::make_shared<SDKLoader>();
	m_effectManager = std::make_unique<EffectManager>();

	/**
	 * Register your sdk here, make sure the RazerSDK is always at the beginning.
	 */
	m_availableSDKs.insert(std::make_unique<RazerSDK>());
	m_availableSDKs.insert(std::make_unique<CorsairSDK>());

	m_clientConfig = new RETCClientConfig;
	reset();
}

void SDKManager::reset() {
	std::fill(m_selectedSDKs, m_selectedSDKs + ALL, nullptr);

	std::fill(m_clientConfig->supportedDeviceTypes, m_clientConfig->supportedDeviceTypes + ESIZE, FALSE);
	std::fill(m_clientConfig->emulatedDeviceIDS, m_clientConfig->emulatedDeviceIDS + ALL, GUID_NULL);

	m_bIsInitialized = false;
}

SDKManager::~SDKManager() {
	delete m_clientConfig;
}

void SDKManager::disconnect() {
	LOG_I("Disconnected cleaning up session.");
	for (auto&& sdk : m_availableSDKs) {
		sdk->disconnect();
	}

	reset();
	m_effectManager->clearEffects();
}

bool SDKManager::initialize() {
	if (m_bIsInitialized) {
		LOG_E("SDKManager already initialized.");
		LOG_I("Hint: Only one concurrent session supported");
		return false;
	}

	checkAvailability();

	reloadEmulatedDevices();
	m_bIsInitialized = true;
	return true;
}

void SDKManager::reloadEmulatedDevices() {
	using namespace LookupMaps;
	using namespace LookupArrays;

	std::wstring lastConfigValue;
	for (int idx = KEYBOARD; idx < ALL; idx++) {
		lastConfigValue = CONFIG->GetWString(SDK_MAIN_CONFIG_SECTION, EM_KEYS[idx], EM_VALS[idx]);
		auto it = LookupMaps::razerStringToDevID.find(lastConfigValue);

		if (it != LookupMaps::razerStringToDevID.end()) {
			m_clientConfig->emulatedDeviceIDS[idx] = it->second;
			LOG->debug(L"{0} set to {1}", EM_KEYS[idx], lastConfigValue);
		}
		else {
			LOG->error(L"Invalid {0} value: {1} using default: {2}", EM_KEYS[idx], lastConfigValue, EM_VALS[idx]);
			lastConfigValue = EM_VALS[idx];
			m_clientConfig->emulatedDeviceIDS[idx] = LookupMaps::razerStringToDevID.at(lastConfigValue);
		}
	}
}

LightingSDK* SDKManager::getSDKForDeviceType(RETCDeviceType type) {
	if (type >= ALL || type < KEYBOARD) {
		return nullptr;
	}

	return m_selectedSDKs[type];
}

void SDKManager::checkAvailability() {
	bool hasAnySDK = false;;
	for (auto&& sdk : m_availableSDKs) {
		if (!sdk->init(m_sdkLoader.get())) {
			LOG_T(L"an sdk failed to initialize {0}", sdk->getSDKName());
			continue;
		}

		auto& supportedDevices = sdk->getSupportedModes();
		for (int devID = KEYBOARD; devID < ALL; devID++) {
			if (supportedDevices[devID] == FALSE) {
				continue;
			}

			m_selectedSDKs[devID] = sdk.get();
			m_clientConfig->supportedDeviceTypes[devID] = TRUE;
			hasAnySDK = true;
		}
	}

	if (hasAnySDK) {
		m_clientConfig->supportedDeviceTypes[ALL] = TRUE;
	}
}

RZRESULT SDKManager::playEffectOnAllSDKs(int effectType, const char effectData[]) const {
	RZRESULT res = RZRESULT_NOT_SUPPORTED;
	for (int devID = KEYBOARD; devID < ALL; devID++) {
		const auto& sdk = m_selectedSDKs[devID];
		if (!sdk) {
			continue;
		}

		res = sdk->playEffect(static_cast<RETCDeviceType>(devID), effectType, effectData);
		if (res != RZRESULT_SUCCESS) {
			break;
		}
	}

	return res;
}

RZRESULT SDKManager::playbackEffect(const RETCDeviceType& devType, int effectType, const char effectData[]) {
	if (devType == ALL) {
		return playEffectOnAllSDKs(effectType, effectData);
	}

	auto sdk = getSDKForDeviceType(devType);
	return (sdk) ? sdk->playEffect(devType, effectType, effectData) : RZRESULT_NOT_FOUND;
}

RZRESULT SDKManager::playEffect(const RETCDeviceType& devType, int effectType, RZEFFECTID* pEffectId, unsigned long size, const char effectData[]) {
	// We need to store the effect
	if (pEffectId != nullptr) {
		return m_effectManager->storeEffect(devType, effectType, pEffectId, size, effectData) ? RZRESULT_SUCCESS : RZRESULT_FAILED;
	}

	return playbackEffect(devType, effectType, effectData);
}

RZRESULT SDKManager::deleteEffect(const RZEFFECTID& effID) const {
	return m_effectManager->deleteEffect(effID) ? RZRESULT_SUCCESS : RZRESULT_NOT_FOUND;
}

RZRESULT SDKManager::setEffect(const RZEFFECTID& effID) {
	const auto& effectEntry = m_effectManager->getEffect(effID);

	if (!effectEntry) {
		return RZRESULT_NOT_FOUND;
	}

	return playbackEffect(effectEntry->deviceType, effectEntry->type, effectEntry->data);
}
