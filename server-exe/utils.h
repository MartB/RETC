#pragma once
#include "spdlog/spdlog.h"

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

#define mkwordhilo(hi,lo) ((hi << 8) | lo);
#define LOBYTE(w) ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w) ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))

extern std::shared_ptr<spdlog::logger> LOG;
#define LOG_E(...) LOG->error("[" __FUNCTION__ "] " __VA_ARGS__)
#define LOG_W(...) LOG->warn("[" __FUNCTION__"] " __VA_ARGS__)
#define LOG_I(...) LOG->info("[" __FUNCTION__"] " __VA_ARGS__)
#define LOG_T(...) SPDLOG_TRACE(LOG, __VA_ARGS__);
