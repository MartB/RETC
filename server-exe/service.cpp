// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <Windows.h>
#include <tchar.h>
#include <thread>

#define SVC_NAME  _T("RETC")
#define SVC_FNAME _T("RETC Service")
#define SVC_DESC  _T("Allows programs that support the Razer Chroma SDK to use Corsair RGB devices. Visit https://github.com/MartB/RETC for more info.")

// Forward declarations
extern void requestTermination();

void InstallService();
void RemoveService();
VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceController(DWORD);
int SVCWorkerThread();


SERVICE_STATUS_HANDLE g_StatusHandle = nullptr;
SERVICE_STATUS g_ServiceStatus = {0};
SERVICE_TABLE_ENTRY ServiceTableEntry[] =
{
	{SVC_NAME, static_cast<LPSERVICE_MAIN_FUNCTION>(ServiceMain)},
	{nullptr, nullptr}
};

int _tmain(const int argc, TCHAR* argv[]) {
	if (argc > 1) {
		if (wcscmp(argv[1], _T("install_service")) == 0) {
			InstallService();
			return EXIT_SUCCESS;
		}

		if (wcscmp(argv[1], _T("remove_service")) == 0) {
			RemoveService();
			return EXIT_SUCCESS;
		}
	}

	if (StartServiceCtrlDispatcher(ServiceTableEntry) == FALSE) {
		OutputDebugString(_T("Service start failed, falling back to normal launch."));
		SVCWorkerThread();
		return GetLastError();
	}

	return EXIT_SUCCESS;
}

VOID WINAPI ServiceMain(DWORD, LPTSTR*) {
	g_StatusHandle = RegisterServiceCtrlHandler(SVC_NAME, ServiceController);

	if (g_StatusHandle == nullptr) {
		OutputDebugString(_T("RegisterServiceCtrlHandler failed"));
		return;
	}

	ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

	// Start service thread
	auto serviceThread = std::thread([=] {
		SVCWorkerThread();
	});

	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 1;

	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

	// Wait until the server terminates
	serviceThread.join();

	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}

VOID WINAPI ServiceController(DWORD ctrlCode) {
	switch (ctrlCode) {
	case SERVICE_CONTROL_STOP:
		if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 0;

		SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

		// Call termination function in server.cpp
		requestTermination();
		break;

	default:
		break;
	}
}

void InstallService() {
	wchar_t szPath[MAX_PATH];
	if (GetModuleFileName(nullptr, szPath, ARRAYSIZE(szPath)) == 0) {
		wprintf(L"GetModuleFileName err 0x%08lx\n", GetLastError());
		return;
	}

	const auto svcManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
	if (svcManager == nullptr) {
		wprintf(L"OpenSCManager err 0x%08lx\n", GetLastError());
		return;
	}

	const auto svc = CreateService(
		svcManager,
		SVC_NAME,
		_T("RETC Service"),
		SERVICE_CHANGE_CONFIG | SERVICE_START,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START,
		SERVICE_ERROR_NORMAL,
		szPath,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	);

	if (svc == nullptr) {
		CloseServiceHandle(svcManager);

		const auto lastErrorCode = GetLastError();
		if (lastErrorCode == ERROR_SERVICE_EXISTS) {
			RemoveService();
			InstallService();
			return;
		}

		wprintf(L"CreateService err 0x%08lx\n", GetLastError());
		return;
	}

	wprintf(L"Installed %s service.\n", SVC_NAME);

	SERVICE_FAILURE_ACTIONS servFailActions;
	SC_ACTION failActions[3];
	failActions[0].Type = SC_ACTION_RESTART;
	failActions[0].Delay = 2000;
	failActions[1].Type = SC_ACTION_RESTART;
	failActions[1].Delay = 2000;
	failActions[2].Type = SC_ACTION_RESTART;
	failActions[2].Delay = 2000;

	servFailActions.dwResetPeriod = 60 * 60 * 24;
	servFailActions.lpCommand = nullptr;
	servFailActions.lpRebootMsg = nullptr;
	servFailActions.cActions = 3;
	servFailActions.lpsaActions = failActions;

	ChangeServiceConfig2(svc, SERVICE_CONFIG_FAILURE_ACTIONS, &servFailActions);

	SERVICE_DESCRIPTION description = {
		SVC_DESC
	};

	ChangeServiceConfig2(svc, SERVICE_CONFIG_DESCRIPTION, &description);

	// Start service
	StartService(svc, 0, nullptr);

	CloseServiceHandle(svcManager);
	CloseServiceHandle(svc);
}


void RemoveService() {

	// Open the local default service control manager database
	const auto svcManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
	if (svcManager == nullptr) {
		wprintf(L"OpenSCManager err 0x%08lx\n", GetLastError());
		return;
	}

	// Open the service with delete, stop, and query status permissions
	const auto svc = OpenService(svcManager, SVC_NAME, SERVICE_STOP |
	                             SERVICE_QUERY_STATUS | DELETE);
	if (svc == nullptr) {
		wprintf(L"OpenService failed w/err 0x%08lx\n", GetLastError());
		CloseServiceHandle(svcManager);
		return;
	}

	SERVICE_STATUS ssSvcStatus = {};
	if (ControlService(svc, SERVICE_CONTROL_STOP, &ssSvcStatus)) {
		wprintf(L"Stopping %s.", SVC_NAME);
		Sleep(500);

		while (QueryServiceStatus(svc, &ssSvcStatus)) {
			if (ssSvcStatus.dwCurrentState == SERVICE_STOP_PENDING) {
				Sleep(500);
			}
			else break;
		}
#ifdef _DEBUG
		if (ssSvcStatus.dwCurrentState == SERVICE_STOPPED) {
			wprintf(L"service stopped.\n");
		}
		else {
			wprintf(L"failed to stop service.\n");
		}
#endif
	}

	if (!DeleteService(svc)) {
		wprintf(L"DeleteService err 0x%08lx\n", GetLastError());
		CloseServiceHandle(svcManager);
		CloseServiceHandle(svc);
		return;
	}

	wprintf(L"Removed %s service.\n", SVC_NAME);

	CloseServiceHandle(svcManager);
	CloseServiceHandle(svc);
}
