// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once
#include <set>
#include <algorithm>

#include "../rpc-midl/rpc_retc.h"
#include "SDKLoader.h"
#include "commonData.h"
#include "RzErrors.h"
#include "RzChromaSDKDefines.h"
#include "RzChromaSDKTypes.h"
#include "colorTransformation.h"
#include "gammaTransformation.h"

#define SDKLoaderAssignNameToVariable(func) func## _t func;
#define SDKLoaderMapNameToFunction(func) func = (func## _t)DLL_FUNCTION_LIST[#func];
#define MAX_CONFIG_SECTION_LEN (size_t)64

class LightingSDK {
public:
	LightingSDK::LightingSDK() {
		m_sdkLoader = nullptr;
		m_dllInstance = nullptr;
		std::fill(m_supportedDevices, m_supportedDevices + ALL, FALSE);
	}

	virtual ~LightingSDK() {
		unloadDLL();
	}

	virtual bool initialize() = 0;
	virtual void reset() = 0;
	virtual RZRESULT playEffect(RETCDeviceType device, int type, const char data[]) = 0;

	bool init(SDKLoader* sdkLoader) {
		if (SDK_DLL.empty() || SDK_NAME.empty()) {
			LOG_W("Skipped an invalid SDK due to missing DLL_NAME.");
			return false;
		}
		
		const auto& sdkConfString = (SDK_MAIN_SUB_CONFIG_SECTION + SDK_NAME);
		std::copy_n(std::begin(sdkConfString), std::min(sdkConfString.length(), MAX_CONFIG_SECTION_LEN), SDK_CONFIG_SECTION);

		if (!CONFIG->GetBool(SDK_CONFIG_SECTION, L"enabled", true)) {
			LOG_D(L"{0} load aborted disabled by config.", SDK_NAME);
			return false;
		}

		if (!sdkLoader->load(m_dllInstance, SDK_DLL, DLL_FUNCTION_LIST)) {
			LOG_E(L"{0} failed to load, please check if an update is available.", SDK_NAME);
			return false;
		}

		m_sdkLoader = sdkLoader;

		if (CONFIG->GetBool(SDK_CONFIG_SECTION, L"enable_colortransformations", true)) {
			LOG_D(L"{0} color transformations enabled.", SDK_NAME);

			if (!initializeColorTransformations()) {
				LOG_E(L"{0}: Error while trying to initialize the color transformations, continuing with partial init.", SDK_NAME);
			}
		}
		

		LOG_I(L"{0} loaded {1} initializing SDK.", SDK_NAME, SDK_DLL);
		return initialize();
	}

	bool initializeColorTransformations() {
		// Gamma adjustment
		auto gammaTransformationValues = CONFIG->GetVec3D(SDK_CONFIG_SECTION, L"gamma_adjustment", Vec3D());
		if (!gammaTransformationValues.isZero()) { // Skip the transformation if we got the default vector.
			m_activeColorTransformations.insert(std::make_unique<GammaTransformation>(gammaTransformationValues));
		}

		return true;
	}

	// r,g,b used as input and output 
	void TRANSFORM_COLORS(int &r, int &g, int &b) {
		for (const auto &transformation : m_activeColorTransformations)	{
			transformation->apply(r, g, b);
		}
	}

	// r,g,b used as output 
	void TRANSFORM_COLORS(const COLORREF& color, int &r, int &g, int &b) {
		r = GetRValue(color);
		g = GetGValue(color);
		b = GetBValue(color);

		return TRANSFORM_COLORS(r, g, b);
	}

	void setSupportFor(RETCDeviceType type, BOOL support) {
		m_supportedDevices[type] = support;
	}

	void enableSupportFor(RETCDeviceType type) { 
		setSupportFor(type, TRUE);
	}

	void disableSupportFor(RETCDeviceType type) {
		setSupportFor(type, FALSE);
	}

	bool hasSupportFor(RETCDeviceType deviceID) const { return m_supportedDevices[deviceID] != FALSE; }

	supportArray_t& getSupportedModes() { return m_supportedDevices; }

	const std::string& getSDKDll() const { return SDK_DLL; }
	const std::wstring& getSDKName() const { return SDK_NAME; }

	void disconnect() {
		// Dont call reset because we cant guarantee that the sdk checks for a valid dll.
		if (m_dllInstance != nullptr) {
			reset();
		}

		m_activeColorTransformations.clear();
		unloadDLL();
	}

	void unloadDLL() const {
		if (!m_sdkLoader || m_dllInstance == nullptr) {
			return;
		}

		m_sdkLoader->unload(SDK_DLL);
	}

protected:
	functionList DLL_FUNCTION_LIST;
	std::string SDK_DLL;
	std::wstring SDK_NAME;
	wchar_t SDK_CONFIG_SECTION[MAX_CONFIG_SECTION_LEN] = {0};

	supportArray_t m_supportedDevices;

	// Color transformation
	std::set<std::unique_ptr<ColorTransformation>> m_activeColorTransformations;

private:
	SDKLoader* m_sdkLoader;
	HINSTANCE m_dllInstance;
};
