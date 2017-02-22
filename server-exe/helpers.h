#pragma once

#define mapto(x,y) case x: return y;
#define maptoassign(x,y) case x: { val = y;  break;}
#define clearandshrink(x) x.clear(); x.shrink_to_fit();

#define mkwordhilo(hi,lo) ((hi << 8) | lo); 
#define LOBYTE(w)           ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))


inline const std::string guidToString(REFGUID guid) {
	char szGuid[40] = { 0 };
	sprintf_s(szGuid, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
	return szGuid;
}

inline std::ostream& operator<<(std::ostream& os, REFGUID guid) {

	os << std::uppercase;
	os.width(8);
	os << guidToString(guid).c_str();
	os << std::nouppercase;
	return os;
}

// Custom compare function for GUIDs
struct RZEFFECTIDCmp {
	bool operator()(const GUID& Left, const GUID& Right) const {
		return memcmp(&Left, &Right, sizeof(Right)) < 0;
	}
};