// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "SDKLoader.h"

SDKLoader::SDKLoader() {

}

SDKLoader::~SDKLoader() {
	destroy();
}

void SDKLoader::destroy() {
	for (auto inst : m_dllInstances) {
		FreeLibrary(inst.second);
	}

	m_dllInstances.clear();
}

bool SDKLoader::load(HINSTANCE& inst, const std::string& dllName, functionList& fList) {
	// Force user to call reload.
	if (isLoaded(dllName)) {
		return false;
	}

	inst = LoadLibraryA(dllName.c_str());

	if (!inst) {
		return false;
	}

	for (auto& func : fList) {
		void* fptr = GetProcAddress(inst, func.first.c_str());
		if (!fptr) {
			FreeLibrary(inst);
			return false;
		}

		func.second = fptr;
	}

	m_dllInstances.insert(make_pair(dllName, inst));
	return true;
}

bool SDKLoader::reload(HINSTANCE& inst, const std::string& dllName, functionList& fList) {
	auto res = unload(dllName);
	if (!res) {
		res = unload(inst);
	}

	if (!res) {
		return false;
	}

	return load(inst, dllName, fList);
}

bool SDKLoader::isLoaded(const std::string& name) {
	return m_dllInstances.find(name) != m_dllInstances.end();
}

bool SDKLoader::unload(const std::string& name) {
	auto elem = m_dllInstances.find(name);

	if (elem == m_dllInstances.end()) {
		return false;
	}

	FreeLibrary(elem->second);
	m_dllInstances.erase(elem);

	return true;
}

bool SDKLoader::unload(HINSTANCE& inst) {
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
