#pragma once
#include "spdlog/spdlog.h"
#include "ConfigManager.h"
#include <memory>

namespace std {
	template <>
	struct hash<GUID> {
		size_t operator()(const GUID& guid) const noexcept {
			auto p = reinterpret_cast<const uint64_t*>(&guid);
			hash<uint64_t> hash;
			return hash(p[0]) ^ hash(p[1]);
		}
	};
}

inline const std::string guidToString(REFGUID guid) {
	char szGuid[40] = { 0 };
	sprintf_s(szGuid, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
	return szGuid;
}

#define mkwordhilo(hi,lo) ((hi << 8) | lo);
#define LOBYTE(w) ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w) ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))

extern std::unique_ptr<ConfigManager> CONFIG;
extern std::shared_ptr<spdlog::logger> LOG;
#define LOG_E(...) LOG->error("[" __FUNCTION__ "] " __VA_ARGS__)
#define LOG_W(...) LOG->warn("[" __FUNCTION__"] " __VA_ARGS__)
#define LOG_I(...) LOG->info("[" __FUNCTION__"] " __VA_ARGS__)
#define LOG_D(...) LOG->debug("[" __FUNCTION__"] " __VA_ARGS__)
#define LOG_T(...) SPDLOG_TRACE(LOG, __VA_ARGS__);
