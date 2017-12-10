// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "ConfigManager.h"
#define CONFIG_FILENAME L"config.ini"

ConfigManager::ConfigManager() {
	simpleIni = std::make_unique<CSimpleIniW>();
	simpleIni->SetUnicode();

	ReloadConfigFile();
}

#define SUCCESS(cond) cond >= SI_OK

void ConfigManager::ReloadConfigFile() {
	simpleIni->Reset();

	// It does not matter if this fails
	simpleIni->LoadFile(CONFIG_FILENAME);
}

// Please do not use these if you expect special characters > 7 bit to work
std::string ConfigManager::GetAsciiString(const wchar_t* section, const wchar_t* key, const wchar_t* def) {
	const std::wstring res = GetWString(section, key, def);
	return std::string(res.begin(), res.end());
}

std::string ConfigManager::GetAsciiString(const wchar_t* section, const wchar_t* key, const std::string &def) {
	std::wstring defConv(def.begin(), def.end());
	return GetAsciiString(section, key, defConv.c_str());
}

// Please do not use this if you rely on utf8, utf16. Use wstring instead
std::string ConfigManager::GetUtf8StdString(const wchar_t* section, const wchar_t* key, const std::string &def) {
	return WSTR_CONVERTER.to_bytes(GetWString(section, key, WSTR_CONVERTER.from_bytes(def).c_str()));
}

const wchar_t* ConfigManager::GetWString(const wchar_t* section, const wchar_t* key, const wchar_t* def) {
	return simpleIni->GetValue(section, key, def);
}

double ConfigManager::GetDouble(const wchar_t* section, const wchar_t* key, double def) {
	return simpleIni->GetDoubleValue(section, key, def);
}

bool ConfigManager::GetBool(const wchar_t* section, const wchar_t* key, const bool def) {
	return simpleIni->GetBoolValue(section, key, def);
}

long ConfigManager::GetLong(const wchar_t* section, const wchar_t* key, const long def) {
	return simpleIni->GetLongValue(section, key, def);
}

bool ConfigManager::SetBool(const wchar_t* section, const wchar_t* key, bool value) {
	return SUCCESS(simpleIni->SetBoolValue(section, key, value));
}

bool ConfigManager::SetLong(const wchar_t* section, const wchar_t* key, const long value) {
	return SUCCESS(simpleIni->SetLongValue(section, key, value));
}

bool ConfigManager::SetWString(const wchar_t* section, const wchar_t* key, const wchar_t* value) {
	return SUCCESS(simpleIni->SetValue(section, key, value));
}

bool ConfigManager::SetDouble(const wchar_t* section, const wchar_t* key, const double value) {
	return SUCCESS(simpleIni->SetDoubleValue(section, key, value));
}

bool ConfigManager::SaveConfig() {
	return SUCCESS(simpleIni->SaveFile(CONFIG_FILENAME));
}

ConfigManager::~ConfigManager()
{
	simpleIni->Reset();
}

