#pragma once
#include "utils.h"
#include <windows.h>
#include <unordered_map>

typedef std::unordered_map<std::string, void*> functionList;
typedef std::unordered_map<std::string, HINSTANCE> dllInstances;

class SDKLoader {
public:
	SDKLoader();
	~SDKLoader();

	bool load(HINSTANCE& inst, const std::string& name, functionList& fList);
	bool reload(HINSTANCE& inst, const std::string& dllName, functionList& fList);
	bool isLoaded(const std::string& name);
	bool unload(const std::string& name);
	bool unload(HINSTANCE& inst);

private:
	void destroy();

private:
	// Only for internal use
	std::unordered_map<std::string, HINSTANCE> m_dllInstances;
};
