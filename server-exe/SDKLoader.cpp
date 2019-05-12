// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "SDKLoader.h"

SdkLoader::~SdkLoader() {
	destroy();
}

void SdkLoader::destroy() {
	for (const auto& inst : m_dllInstances) {
		FreeLibrary(inst.second);
	}

	m_dllInstances.clear();
}

bool SdkLoader::load(HINSTANCE& inst, const std::string& name, function_list& fList) {
	// Force user to call reload.
	if (isLoaded(name)) {
		LOG_D("DLL already loaded you should call reload!");
		return false;
	}

	inst = LoadLibraryA(name.c_str());

	if (!inst) {
		LOG_E("LoadLibraryA({0}) failed with code {1}", name, GetLastError());
		return false;
	}

	for (auto& func : fList) {
		const auto funcPtrName = func.first.c_str();
		const auto fptr = reinterpret_cast<FARPROC *>(GetProcAddress(inst, funcPtrName));
		if (!fptr) {
			LOG_E("Could not find required method {0}, please check {1}.", funcPtrName, name);
			FreeLibrary(inst);
			return false;
		}
		func.second = fptr;
	}

	m_dllInstances.insert(make_pair(name, inst));
	return true;
}

bool SdkLoader::reload(HINSTANCE& inst, const std::string& dllName, function_list& fList) {
	auto res = unload(dllName);
	if (!res) {
		res = unload(inst);
	}

	if (!res) {
		return false;
	}

	return load(inst, dllName, fList);
}

bool SdkLoader::isLoaded(const std::string& name) {
	return m_dllInstances.find(name) != m_dllInstances.end();
}

bool SdkLoader::unload(const std::string& name) {
	const auto elem = m_dllInstances.find(name);

	if (elem == m_dllInstances.end()) {
		return false;
	}

	FreeLibrary(elem->second);
	m_dllInstances.erase(elem);

	return true;
}

bool SdkLoader::unload(HINSTANCE& inst) {
	for (auto& func : m_dllInstances) {
		if (func.second != inst) {
			continue;
		}

		FreeLibrary(func.second);
		m_dllInstances.erase(func.first);
		return true;
	}

	return false;
}
