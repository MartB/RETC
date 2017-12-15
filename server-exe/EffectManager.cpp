// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "EffectManager.h"

bool EffectManager::storeEffect(const RETCDeviceType& deviceType, int effectType, RZEFFECTID* pEffectID, unsigned long effectSize, const char effectData[]) {
	internalEffectData effData;
	effData.type = effectType;
	effData.deviceType = deviceType;
	effData.data = new char[effectSize];
	std::copy(effectData, effectData + effectSize, effData.data);

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

	delete[] it->second.data;
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
