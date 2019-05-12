// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once
#include <set>
#include <algorithm>

#include "../rpc-midl/rpc_retc.h"
#include "SDKLoader.h"
#include "commonData.h"
#include "colorTransformation.h"
#include "gammaTransformation.h"

#define SDKLoaderAssignNameToVariable(func) func## _t func;
#define SDKLoaderMapNameToFunction(func) func = (func## _t)m_dllFunctionList[#func];
#define MAX_CONFIG_SECTION_LEN (size_t)64

class LightingSdk {
public:
	LightingSdk() {
		m_sdkLoader = nullptr;
		m_dllInstance = nullptr;
		std::fill(m_supportedDevices, m_supportedDevices + ALL, FALSE);
	}

	virtual ~LightingSdk() {
		unloadDLL();
	}

	virtual bool initialize() = 0;
	virtual void reset() = 0;
	virtual RZRESULT playEffect(RETCDeviceType device, int type, const char data[]) = 0;

	bool init(SdkLoader* sdkLoader) {
		if (m_sdkDll.empty() || m_sdkName.empty()) {
			LOG_W("Skipped an invalid SDK due to missing DLL_NAME.");
			return false;
		}
		
		const auto& sdkConfString = (SDK_SUB_CONFIG_SECTION + m_sdkName);
		std::copy_n(std::begin(sdkConfString), std::min(sdkConfString.length(), MAX_CONFIG_SECTION_LEN), m_sdkConfigSection);

		if (!CONFIG->GetBool(m_sdkConfigSection, L"enabled", true)) {
			LOG_D(L"{0} load aborted disabled by config.", m_sdkName);
			return false;
		}

		if (!sdkLoader->load(m_dllInstance, m_sdkDll, m_dllFunctionList)) {
			LOG_E(L"{0} failed to load, please check if an update is available.", m_sdkName);
			return false;
		}

		m_sdkLoader = sdkLoader;

		if (CONFIG->GetBool(m_sdkConfigSection, L"enable_colortransformations", true)) {
			LOG_D(L"{0} color transformations enabled.", m_sdkName);

			if (!initializeColorTransformations()) {
				LOG_E(L"{0}: Error while trying to initialize the color transformations, continuing with partial init.", m_sdkName);
			}
		}
		

		LOG_I(L"{0} loaded {1} initializing SDK.", m_sdkName, m_sdkDll);
		return initialize();
	}

	bool initializeColorTransformations() {
		// Gamma adjustment
		auto gammaTransformationValues = CONFIG->GetVec3D(m_sdkConfigSection, L"gamma_adjustment", Vec3D());
		if (!gammaTransformationValues.isZero()) { // Skip the transformation if we got the default vector.
			m_activeColorTransformations.insert(std::make_unique<GammaTransformation>(gammaTransformationValues));
		}

		return true;
	}

	// r,g,b used as input and output 
	void TRANSFORM_COLORS(uint8_t&r, uint8_t&g, uint8_t &b) {
		for (const auto &transformation : m_activeColorTransformations)	{
			transformation->apply(r, g, b);
		}
	}

	// r,g,b used as output 
	void CONVERT_AND_TRANSFORM_COLORS(const COLORREF& color, uint8_t &r, uint8_t&g, uint8_t&b) {
		r = GetRValue(color);
		g = GetGValue(color);
		b = GetBValue(color);

		return TRANSFORM_COLORS(r, g, b);
	}

	void setSupportFor(const RETCDeviceType type, const BOOL support) {
		m_supportedDevices[type] = support;
	}

	void enableSupportFor(const RETCDeviceType type) { 
		setSupportFor(type, TRUE);
	}

	void disableSupportFor(const RETCDeviceType type) {
		setSupportFor(type, FALSE);
	}

	bool hasSupportFor(const RETCDeviceType deviceId) const { return m_supportedDevices[deviceId] != FALSE; }

	supportArray_t& getSupportedModes() { return m_supportedDevices; }

	const std::string& getSDKDll() const { return m_sdkDll; }
	const std::wstring& getSDKName() const { return m_sdkName; }

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

		m_sdkLoader->unload(m_sdkDll);
	}

protected:
	function_list m_dllFunctionList;
	std::string m_sdkDll;
	std::wstring m_sdkName;
	wchar_t m_sdkConfigSection[MAX_CONFIG_SECTION_LEN] = {0};

	supportArray_t m_supportedDevices{};

	// Color transformation
	std::set<std::unique_ptr<ColorTransformation>> m_activeColorTransformations;

private:
	SdkLoader* m_sdkLoader;
	HINSTANCE m_dllInstance;
};
