// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "RPCReceiver.h"
#include <iostream>

// Global vars
std::unique_ptr<RPCReceiver> rpcReceiver(nullptr);
std::unique_ptr<SDKManager> sdkManager(nullptr);
std::unique_ptr<ConfigManager> CONFIG(nullptr);
std::shared_ptr<spdlog::logger> LOG(nullptr);

// Main
bool userCancelled = false;
std::condition_variable cv;

void cleanup() {
	if (rpcReceiver) {
		//-V547
		rpcReceiver->shutdown();
		rpcReceiver.reset();
	}

	if (sdkManager) {
		//-V547
		sdkManager.reset();
	}

	if (CONFIG) {
		//-V547
		if (!CONFIG->SaveConfig()) {
			LOG->error("Failed to save config file.");
		}
	}
}

extern void requestTermination() {
	userCancelled = true;
	cv.notify_all();
}

BOOL WINAPI consoleHandler(DWORD signal) {
	switch (signal) {
	case CTRL_C_EVENT:
		requestTermination();
		return TRUE;
	case CTRL_CLOSE_EVENT:
		cleanup();
		return TRUE;
	default:
		return FALSE;
	}
}


#if defined(_DEBUG)
#define DEF_LOG_LEVEL spdlog::level::trace
#define DEF_FLUSH_LEVEL spdlog::level::trace
#else
#define DEF_LOG_LEVEL spdlog::level::warn
#define DEF_FLUSH_LEVEL spdlog::level::err
#endif

DWORD WINAPI SVCWorkerThread(LPVOID) {
	CONFIG.reset(new ConfigManager());

	try {
		std::vector<spdlog::sink_ptr> sinks;
		sinks.emplace_back(std::make_shared<spdlog::sinks::wincolor_stdout_sink_st>());
		sinks.emplace_back(std::make_shared<spdlog::sinks::simple_file_sink_st>("server.log"));
		LOG = std::make_shared<spdlog::logger>("logger", begin(sinks), end(sinks));

		LOG->set_pattern(CONFIG->GetAsciiString(L"log", L"format", L"[%d.%m %H:%M:%S.%e][%l] %v"));
		spdlog::set_async_mode(1024);

		LOG->set_level(static_cast<spdlog::level::level_enum>(CONFIG->GetLong(L"log", L"level", DEF_LOG_LEVEL)));
		LOG->flush_on(static_cast<spdlog::level::level_enum>(CONFIG->GetLong(L"log", L"flush", DEF_FLUSH_LEVEL)));

		spdlog::set_error_handler([](const std::string& msg) {
			LOG->error("ERR: {}", msg);
		});
	}
	catch (const spdlog::spdlog_ex& ex) {
		std::cout << "Log init failed: " << ex.what() << std::endl;
		return ERROR_CANTOPEN;
	}

	if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
		LOG_E("Setting the console handler failed");
		return ERROR_INVALID_HANDLE;
	}

	sdkManager.reset(new SDKManager());
	rpcReceiver.reset(new RPCReceiver());
	rpcReceiver->startListening();

	// We dont have stuff to do for now so we will just keep sleeping.
	std::mutex m;
	std::unique_lock<std::mutex> lock(m);
	while (!userCancelled) {
		cv.wait(lock);
	}

	// Dont put code below this just add all your exit code to cleanup()
	cleanup();
	return ERROR_SUCCESS;
}
