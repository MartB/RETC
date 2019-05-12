#pragma once
#define SI_SUPPORT_IOSTREAMS
#include "SimpleIni/SimpleIni.h"

#include <codecvt>
#include <memory>

struct Vec3D {
	double x, y, z;
	Vec3D() : x(0.0f), y(0.0f), z(0.0f) {}

	bool isZero() const {
		return x == 0.0f && y == 0.0f && z == 0.0f; //-V550
	}
};

class ConfigManager {
public:
	~ConfigManager();
	ConfigManager();

	void ReloadConfigFile() const;

	std::string GetAsciiString(const wchar_t * section, const wchar_t * key, const wchar_t * def) const;
	std::string GetAsciiString(const wchar_t * section, const wchar_t * key, const std::string &def) const;

	std::string GetUtf8StdString(const wchar_t * section, const wchar_t * key, const std::string &def);

	const wchar_t * GetWString(const wchar_t * section, const wchar_t * key, const wchar_t * def) const;
	double GetDouble(const wchar_t * section, const wchar_t * key, double def) const;
	bool GetBool(const wchar_t * section, const wchar_t * key, bool def);
	long GetLong(const wchar_t * section, const wchar_t * key, long def);
	Vec3D GetVec3D(const wchar_t * section, const wchar_t *key, const Vec3D &def) const;

	bool SetBool(const wchar_t * section, const wchar_t * key, bool value) const;
	bool SetLong(const wchar_t * section, const wchar_t * key, long value) const;
	bool SetWString(const wchar_t * section, const wchar_t * key, const wchar_t * value) const;

	bool SetDouble(const wchar_t * section, const wchar_t * key, double value) const;
	bool SetVec3D(const wchar_t* section, const wchar_t* key, const Vec3D &value) const;

	bool SaveConfig() const;

private:
	std::unique_ptr<CSimpleIni> simpleIni;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> WSTR_CONVERTER;

};