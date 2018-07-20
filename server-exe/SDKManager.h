#pragma once

#include "utils.h"
#include "SDKLoader.h"
#include "EffectManager.h"

#include <set>

#include "CorsairSDK.h"
#include "RazerSDK.h"

class SDKManager {
public:
	SDKManager();
	~SDKManager();

	void reset();
	void disconnect();

public:
	bool initialize();
	void reloadEmulatedDevices();
	LightingSDK* getSDKForDeviceType(RETCDeviceType type);
	RETCClientConfig* getClientConfig() const { return m_clientConfig; }

public:
	RZRESULT deleteEffect(const RZEFFECTID& effID) const;
	RZRESULT setEffect(const RZEFFECTID& effID);
	RZRESULT playEffect(const RETCDeviceType& devType, int effectType, RZEFFECTID* pEffectId, efsize_t size, const char effectData[]);

private:
	void checkAvailability();
	RZRESULT playEffectOnAllSDKs(int effectType, const char effectData[]) const;
	RZRESULT playbackEffect(const RETCDeviceType& devType, int effectType, const char effectData[]);

	std::shared_ptr<SDKLoader> m_sdkLoader;

	std::unique_ptr<EffectManager> m_effectManager;
	std::set<std::unique_ptr<LightingSDK>> m_availableSDKs;

	RETCClientConfig* m_clientConfig;

	LightingSDK* m_selectedSDKs[ALL];

	bool m_bIsInitialized;
};
