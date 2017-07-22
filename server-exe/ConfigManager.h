#pragma once
#define SI_SUPPORT_IOSTREAMS
#include "SimpleIni/SimpleIni.h"

#include <codecvt>
#include <memory>

class ConfigManager {
public:
	~ConfigManager();
	ConfigManager();

	void ReloadConfigFile();

	std::string GetAsciiString(const wchar_t * section, const wchar_t * key, const wchar_t * def);
	std::string GetAsciiString(const wchar_t * section, const wchar_t * key, const std::string &def);

	std::string GetUtf8StdString(const wchar_t * section, const wchar_t * key, const std::string &def);

	const wchar_t * GetWString(const wchar_t * section, const wchar_t * key, const wchar_t * def);
	double GetDouble(const wchar_t * section, const wchar_t * key, double def);
	bool GetBool(const wchar_t * section, const wchar_t * key, const bool def);
	long GetLong(const wchar_t * section, const wchar_t * key, const long def);

	bool SetBool(const wchar_t * section, const wchar_t * key, bool value);
	bool SetLong(const wchar_t * section, const wchar_t * key, const long value);
	bool SetWString(const wchar_t * section, const wchar_t * key, const wchar_t * value);

	bool SetDouble(const wchar_t * section, const wchar_t * key, const double value);

	bool SaveConfig();

private:
	std::unique_ptr<CSimpleIni> simpleIni;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> WSTR_CONVERTER;

};