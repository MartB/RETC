#pragma once
#include "utils.h"
#include "../rpc-midl/rpc_retc.h"


typedef struct internalEffectData {
	int type;
	RETCDeviceType deviceType;
	char *data;
} internalEffectData;


class EffectManager {
public:
	static bool createUniqueEffectID(RZEFFECTID* guid) { return CoCreateGuid(guid) == S_OK; }

	~EffectManager();
	bool storeEffect(const RETCDeviceType& deviceType, int effectType, RZEFFECTID* pEffectID, unsigned long effectSize, const char effectData[]);
	bool deleteEffect(const RZEFFECTID& pEffectID);
	void clearEffects();

	const internalEffectData* getEffect(const RZEFFECTID& pEffectID);

private:
	std::unordered_map<RZEFFECTID, internalEffectData> m_effectMap;
};
