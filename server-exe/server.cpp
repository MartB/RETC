// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "RPCReceiver.h"
#include <iostream>

// Global vars
std::unique_ptr<RPCReceiver> rpcReceiver(nullptr);
std::unique_ptr<SDKManager> sdkManager(nullptr);
std::shared_ptr<spdlog::logger> LOG(nullptr);

// Main
bool userCancelled = false;

void cleanup() {
	if (rpcReceiver) {
		rpcReceiver->shutdown();
		rpcReceiver.reset();
	}
	if (sdkManager) {
		sdkManager.reset();
	}
}

BOOL WINAPI consoleHandler(DWORD signal) {
	switch (signal) {
	case CTRL_C_EVENT:
		userCancelled = true;
		return TRUE;
	case CTRL_CLOSE_EVENT:
		cleanup();
		return TRUE;
	default:
		return FALSE;
	}
}

int main() {
	try {
		std::vector<spdlog::sink_ptr> sinks;
		sinks.push_back(std::make_shared<spdlog::sinks::wincolor_stdout_sink_st>());
		sinks.push_back(std::make_shared<spdlog::sinks::simple_file_sink_st>("server.log"));
		LOG = std::make_shared<spdlog::logger>("logger", begin(sinks), end(sinks));
		spdlog::set_pattern("[%d-%m-%Y %H:%M:%S.%e %z][%l] %v");
		spdlog::set_async_mode(1024);

#if defined(_DEBUG)
		LOG->set_level(spdlog::level::trace);
		LOG->flush_on(spdlog::level::trace);
#else
		LOG->set_level(spdlog::level::warn);
		LOG->flush_on(spdlog::level::err);
#endif

		spdlog::set_error_handler([](const std::string& msg) {
			std::cerr << "ERR: " << msg << std::endl;
		});
	}
	catch (const spdlog::spdlog_ex& ex) {
		std::cout << "Log init failed: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
		LOG_E("Setting the console handler failed");
		return EXIT_FAILURE;
	}

	sdkManager.reset(new SDKManager());
	rpcReceiver.reset(new RPCReceiver());
	rpcReceiver->startListening();

	// We dont have stuff to do for now so we will just keep sleeping.
	while (!userCancelled) {
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(1ms);
	}

	cleanup();

	return EXIT_SUCCESS;
}
