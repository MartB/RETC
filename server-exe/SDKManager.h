#pragma once

#include "utils.h"
#include "SDKLoader.h"
#include "EffectManager.h"

#include <set>
#include "LightingSDK.h"

using sdk_list = std::set<std::shared_ptr<LightingSdk>>;

class SdkManager {
public:
	SdkManager();
	~SdkManager() = default;

	void reset();
	void disconnect();

	bool initialize();
	void reloadEmulatedDevices() const;

	std::shared_ptr<RETCClientConfig> getClientConfig() const { return m_clientConfig; }

	RZRESULT deleteEffect(const RZEFFECTID& effId) const;
	RZRESULT setEffect(const RZEFFECTID& effId);
	RZRESULT playEffect(const RETCDeviceType& devType, int effectType, RZEFFECTID* pEffectId, efsize_t size, const char effectData[]);

private:
	void checkAvailability();
	RZRESULT playEffectOnAllSDKs(int effectType, const char effectData[]) const;
	RZRESULT playbackEffect(const RETCDeviceType& devType, int effectType, const char effectData[]);

	std::shared_ptr<SdkLoader> m_sdkLoader;

	std::unique_ptr<EffectManager> m_effectManager;
	sdk_list m_availableSDKs;

	std::shared_ptr<RETCClientConfig> m_clientConfig;

	std::array<sdk_list, ALL> m_selectedSDKs;
	bool m_bIsInitialized{};
};
