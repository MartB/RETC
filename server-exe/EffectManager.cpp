// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "EffectManager.h"

bool EffectManager::storeEffect(const RETCDeviceType& deviceType, int effectType, RZEFFECTID* pEffectID, unsigned long effectSize, const char effectData[]) {
	auto effData = internalEffectData();
	effData.type = effectType;
	effData.deviceType = deviceType;
	effData.data = std::string(effectData, effectData + effectSize);

	RZEFFECTID newEffectID;
	if (!createUniqueEffectID(&newEffectID)) {
		return false;
	}

	*pEffectID = newEffectID;

	m_effectMap.insert(std::make_pair(newEffectID, effData));

	return true;
}

bool EffectManager::deleteEffect(const RZEFFECTID& pEffectID) {
	auto it = m_effectMap.find(pEffectID);
	if (it == m_effectMap.end()) {
		return false;
	}

	m_effectMap.erase(it);

	return true;
}

const internalEffectData* EffectManager::getEffect(const RZEFFECTID& pEffectID) {
	const auto& entry = m_effectMap.find(pEffectID);

	if (entry == m_effectMap.end()) {
		return nullptr;
	}

	return &entry->second;
}
