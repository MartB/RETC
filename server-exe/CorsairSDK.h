#pragma once
#include "LightingSDK.h"
#include "CUESDK.h"
#include <vector>

class CorsairSDK : public LightingSDK {
public:
	CorsairSDK();

	bool initialize() override;
	void reset() override;
	RZRESULT playEffect(RETCDeviceType device, int type, const char data[]) override;

private:
	RZRESULT prepareKeyboardEffect(int type, const char effectData[]);
	RZRESULT prepareMouseEffect(int type, const char effectData[]);
	RZRESULT prepareMousePadEffect(int type, const char effectData[]);
	RZRESULT prepareHeadsetEffect(int type, const char effectData[]);
	RZRESULT prepareHeadsetStandEffect(int type, const char effectData[]);

	// Helper functions
	CorsairLedColor convertLedColor(const COLORREF& color);
	static RETCDeviceType corsairToRETCDeviceTYPE(CorsairDeviceType type);

	static bool findKeyboardLed(CorsairLedId ledid, int* row, int* col);
	static bool findMouseLed(const CorsairLedId ledid, int* row, int* col);
	static CorsairLedId findMouseLedForRZLED(ChromaSDK::Mouse::RZLED led);
	static ChromaSDK::Mouse::RZLED findMouseLedForCLD(CorsairLedId led);
	static int findHeadsetStandLed(CorsairLedId ledid);

	static std::string errToString(const CorsairError& error);

	typedef std::vector<CorsairLedId> ledIDVector;
	typedef std::vector<CorsairLedColor> ledColorVector;

	ledIDVector m_availableLeds[ALL];
	ledColorVector m_outputColorVector;

	// DLL functions
	typedef bool (*CorsairSetLedsColors_t)(int, CorsairLedColor*);
	typedef bool (*CorsairSetLedsColorsAsync_t)(int, CorsairLedColor*, void (*CallbackType)(void*, bool, CorsairError), void*);
	typedef int (*CorsairGetDeviceCount_t)();
	typedef CorsairDeviceInfo*(*CorsairGetDeviceInfo_t)(int);
	typedef CorsairLedPositions*(*CorsairGetLedPositions_t)();
	typedef CorsairLedId (*CorsairGetLedIdForKeyName_t)(char);
	typedef bool (*CorsairRequestControl_t)(CorsairAccessMode);
	typedef CorsairProtocolDetails (*CorsairPerformProtocolHandshake_t)();
	typedef CorsairError (*CorsairGetLastError_t)();
	typedef CorsairLedPositions*(*CorsairGetLedPositionsByDeviceIndex_t)(int);

	SDKLoaderAssignNameToVariable(CorsairSetLedsColors)
	SDKLoaderAssignNameToVariable(CorsairSetLedsColorsAsync)
	SDKLoaderAssignNameToVariable(CorsairGetDeviceCount)
	SDKLoaderAssignNameToVariable(CorsairGetDeviceInfo)
	SDKLoaderAssignNameToVariable(CorsairGetLedPositions)
	SDKLoaderAssignNameToVariable(CorsairGetLedIdForKeyName)
	SDKLoaderAssignNameToVariable(CorsairRequestControl)
	SDKLoaderAssignNameToVariable(CorsairPerformProtocolHandshake)
	SDKLoaderAssignNameToVariable(CorsairGetLastError)
	SDKLoaderAssignNameToVariable(CorsairGetLedPositionsByDeviceIndex)
};
