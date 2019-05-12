#pragma once
#include "utils.h"
#include <windows.h>
#include <unordered_map>

using function_list = std::unordered_map<std::string, void*>;
using dll_instances = std::unordered_map<std::string, FARPROC*>;

class SdkLoader {
public:
	SdkLoader() = default;
	~SdkLoader();

	bool load(HINSTANCE& inst, const std::string& name, function_list& fList);
	bool reload(HINSTANCE& inst, const std::string& dllName, function_list& fList);
	bool isLoaded(const std::string& name);
	bool unload(const std::string& name);
	bool unload(HINSTANCE& inst);

private:
	void destroy();

	// Only for internal use
	std::unordered_map<std::string, HINSTANCE> m_dllInstances;
};
