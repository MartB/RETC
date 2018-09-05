// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "SDKManager.h"
#include "RzErrors.h"
#include "LightingSDK.h"

SDKManager::SDKManager() {
	m_sdkLoader = std::make_shared<SDKLoader>();
	m_effectManager = std::make_unique<EffectManager>();

	/**
	 * Register your sdk here, make sure the RazerSDK is always at the beginning.
	 */
	m_availableSDKs.insert(std::make_shared<RazerSDK>());
	m_availableSDKs.insert(std::make_shared<CorsairSDK>());

	m_clientConfig = new RETCClientConfig;
	reset();
}

void SDKManager::reset() {
	for (auto &sdkList : m_selectedSDKs) {
		sdkList.clear();
	}

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
		// Razers api does not do HEADSET_STAND support properly, so skip this for now.
		if (idx == HEADSET_STAND) {
			continue;
		}

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

void SDKManager::checkAvailability() {
	bool hasAnySDK = false;
	for (auto&& sdk : m_availableSDKs) {
		if (!sdk->init(m_sdkLoader.get())) {
			LOG_T(L"an sdk failed to initialize {0}", sdk->getSDKName());
			continue;
		}

		auto& supportedDevices = sdk->getSupportedModes();
		for (int devID = KEYBOARD; devID < ALL; devID++) {
			// Check if the device is not supported
			if (supportedDevices[devID] == FALSE) {
				continue;
			}

			m_selectedSDKs[devID].insert(sdk);
			m_clientConfig->supportedDeviceTypes[devID] = TRUE;
			hasAnySDK = true;
		}
	}

	// Force enable the mousepad support if a headset_stand is connected, this does not assign a sdk!
	if (m_clientConfig->supportedDeviceTypes[MOUSEPAD] == FALSE && m_clientConfig->supportedDeviceTypes[HEADSET_STAND] != FALSE) {
		m_clientConfig->supportedDeviceTypes[MOUSEPAD] = TRUE;
	}

	if (hasAnySDK) {
		m_clientConfig->supportedDeviceTypes[ALL] = TRUE;
	}
}

RZRESULT SDKManager::playEffectOnAllSDKs(int effectType, const char effectData[]) const {
	RZRESULT res = RZRESULT_NOT_SUPPORTED;
	for (int devID = KEYBOARD; devID < ALL; devID++) {
		for (const auto& sdk : m_selectedSDKs[devID]) {
			res = sdk->playEffect(static_cast<RETCDeviceType>(devID), effectType, effectData);
			if (res != RZRESULT_SUCCESS) {
				LOG_D(L"SDK: {0} failed to play an effect.", sdk->getSDKName());
				continue; // dont hard fail if one sdk failed.
			}
		}
	}

	return res;
}

RZRESULT SDKManager::playbackEffect(const RETCDeviceType& devType, int effectType, const char effectData[]) {
	if (devType == ALL) {
		return playEffectOnAllSDKs(effectType, effectData);
	}

	RZRESULT res = RZRESULT_NOT_FOUND;

	for (auto&& sdk : m_selectedSDKs[devType]) {
		res = sdk->playEffect(devType, effectType, effectData);
	}

	// *WORKAROUND ALERT!* Send fake data to headset_stands based on the mousepad data.
	if (devType == MOUSEPAD) {
		for (auto&& sdk : m_selectedSDKs[HEADSET_STAND]) {
			res = sdk->playEffect(HEADSET_STAND, effectType, effectData);
		}
	}

	return res;
}

RZRESULT SDKManager::playEffect(const RETCDeviceType& devType, int effectType, RZEFFECTID* pEffectId, efsize_t size, const char effectData[]) {
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
