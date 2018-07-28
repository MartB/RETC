// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once
#include "../rpc-midl/rpc_retc.h"
#include "SDKLoader.h"
#include "commonData.h"
#include "RzErrors.h"
#include "RzChromaSDKDefines.h"
#include "RzChromaSDKTypes.h"
#include <algorithm>

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

		LOG_I(L"{0} loaded {1} initializing SDK.", SDK_NAME, SDK_DLL);
		return initialize();
	}

	void enableSupportFor(RETCDeviceType type) { m_supportedDevices[type] = TRUE; }
	void disableSupportFor(RETCDeviceType type) { m_supportedDevices[type] = FALSE; }
	bool hasSupportFor(RETCDeviceType deviceID) const { return m_supportedDevices[deviceID] != FALSE; }

	supportArray_t& getSupportedModes() { return m_supportedDevices; }

	const std::string& getSDKDll() const { return SDK_DLL; }
	const std::wstring& getSDKName() const { return SDK_NAME; }

	void disconnect() {
		// Dont call reset because we cant guarantee that the sdk checks for a valid dll.
		if (m_dllInstance != nullptr) {
			reset();
		}
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

private:
	SDKLoader* m_sdkLoader;
	HINSTANCE m_dllInstance;
};
