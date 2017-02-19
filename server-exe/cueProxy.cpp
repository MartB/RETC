#include "cueProxy.h"

bool cueProxy::Load() {
	if (this->isLoaded())
		return true;

	this->hInstance = LoadLibraryA(CORSAIR_DLL_NAME);

	if (!this->hInstance)
		return false;

	this->CorsairSetLedsColors = (CORSAIRSETLEDSCOLORS)GetProcAddress(this->hInstance, CORSAIR_DLL_SETLEDSCOLORS);
	this->CorsairSetLedsColorsAsync = (CORSAIRSETLEDSCOLORSASYNC)GetProcAddress(this->hInstance, CORSAIR_DLL_SETLEDSCOLORSASYNC);
	this->CorsairGetDeviceCount = (CORSAIRGETDEVICECOUNT)GetProcAddress(this->hInstance, CORSAIR_DLL_GETDEVICECOUNT);
	this->CorsairGetDeviceInfo = (CORSAIRGETDEVICEINFO)GetProcAddress(this->hInstance, CORSAIR_DLL_GETDEVICEINFO);
	this->CorsairGetLedPositions = (CORSAIRGETLEDPOSITIONS)GetProcAddress(this->hInstance, CORSAIR_DLL_GETLEDPOSITIONS);
	this->CorsairGetLedIdForKeyName = (CORSAIRGETLEDIDFORKEYNAME)GetProcAddress(this->hInstance, CORSAIR_DLL_GETLEDIDFORKEYNAME);
	this->CorsairRequestControl = (CORSAIRREQUESTCONTROL)GetProcAddress(this->hInstance, CORSAIR_DLL_REQUESTCONTROL);
	this->CorsairPerformProtocolHandshake = (CORSAIRPERFORMPROTOCOLHANDSHAKE)GetProcAddress(this->hInstance, CORSAIR_DLL_PERFORMPROTOCOLHANDSHAKE);
	this->CorsairGetLastError = (CORSAIRGETLASTERROR)GetProcAddress(this->hInstance, CORSAIR_DLL_GETLASTERROR);

	return true;
}

bool cueProxy::Unload() {
	if (!this->isLoaded())
		return false;

	bool result = this->ReleaseLibrary();
	return result;
}

std::string cueProxy::corsairErrorToString(const CorsairError error) {
	switch (error)
	{
	case CE_Success:
		return "CE_Success";
	case CE_ServerNotFound:
		return "CE_ServerNotFound";
	case CE_NoControl:
		return "CE_NoControl";
	case CE_ProtocolHandshakeMissing:
		return "CE_ProtocolHandshakeMissing";
	case CE_IncompatibleProtocol:
		return "CE_IncompatibleProtocol";
	case CE_InvalidArguments:
		return "CE_InvalidArguments";
	default:
		return "unknown error";
	}
}

bool cueProxy::ReleaseLibrary() {
	if (!this->hInstance)
		return true;

	bool result = FreeLibrary(this->hInstance) == TRUE;
	this->hInstance = nullptr;
	return result;
}
