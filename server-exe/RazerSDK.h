#pragma once
#include "LightingSDK.h"
using namespace ChromaSDK;

class RazerSDK : public LightingSdk {
public:
	RazerSDK();

	bool initialize() override;
	void reset() override;
	RZRESULT playEffect(RETCDeviceType device, int type, const char data[]) override;
	static RETCDeviceType razerToRETCDeviceTYPE(DEVICE_INFO_TYPE devType);

private:
	// DLL functions
	typedef RZRESULT (*Init_t)();
	typedef RZRESULT (*UnInit_t)();
	typedef RZRESULT (*CreateKeyboardEffect_t)(Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
	typedef RZRESULT (*CreateMouseEffect_t)(Mouse::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
	typedef RZRESULT (*CreateHeadsetEffect_t)(Headset::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
	typedef RZRESULT (*CreateMousepadEffect_t)(Mousepad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
	typedef RZRESULT (*CreateKeypadEffect_t)(Keypad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
	typedef RZRESULT (*CreateChromaLinkEffect_t)(ChromaLink::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);

	typedef RZRESULT (*SetEffect_t)(RZEFFECTID EffectId);
	typedef RZRESULT (*DeleteEffect_t)(RZEFFECTID EffectId);
	typedef RZRESULT (*QueryDevice_t)(RZDEVICEID EffectId, DEVICE_INFO_TYPE& DeviceInfo);

	SDKLoaderAssignNameToVariable(Init)
	SDKLoaderAssignNameToVariable(UnInit)
	SDKLoaderAssignNameToVariable(CreateKeyboardEffect)
	SDKLoaderAssignNameToVariable(CreateMouseEffect)
	SDKLoaderAssignNameToVariable(CreateHeadsetEffect)
	SDKLoaderAssignNameToVariable(CreateMousepadEffect)
	SDKLoaderAssignNameToVariable(CreateKeypadEffect)
	SDKLoaderAssignNameToVariable(CreateChromaLinkEffect)
	SDKLoaderAssignNameToVariable(SetEffect)
	SDKLoaderAssignNameToVariable(DeleteEffect)
	SDKLoaderAssignNameToVariable(QueryDevice)
};
