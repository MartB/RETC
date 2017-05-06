#pragma once
#include "utils.h"
#include "../rpc-midl/rpc_retc.h"


typedef struct internalEffectData {
	int type;
	RETCDeviceType deviceType;
	std::string data;
} internalEffectData;


class EffectManager {
public:
	static bool createUniqueEffectID(RZEFFECTID* guid) { return CoCreateGuid(guid) == S_OK; }

	bool storeEffect(const RETCDeviceType& deviceType, int effectType, RZEFFECTID* pEffectID, unsigned long effectSize, const char effectData[]);
	bool deleteEffect(const RZEFFECTID& pEffectID);
	void clearEffects() { m_effectMap.clear(); }

	const internalEffectData* getEffect(const RZEFFECTID& pEffectID);

private:
	std::unordered_map<RZEFFECTID, internalEffectData> m_effectMap;
};
