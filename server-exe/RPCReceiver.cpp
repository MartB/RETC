// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "RPCReceiver.h"

long PROCESSING_DELAY = 0;

void* __RPC_USER midl_user_allocate(size_t size) {
	return malloc(size);
}

void __RPC_USER midl_user_free(void* p) {
	free(p);
}

RPCReceiver::RPCReceiver() {
	m_RpcStatus = RPC_S_OK;
	PROCESSING_DELAY = CONFIG->GetLong(L"rpc", L"processingdelay", 0);
}

void RpcServerListenThreadProc() {
	RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, FALSE);
}

RPC_STATUS RPCReceiver::startListening() {
	m_RpcStatus = RpcServerUseProtseqEp(RPC_WSTR(L"ncalrpc"), 0, RPC_WSTR(L"[retc-rpc]"), nullptr);

	if (!STATUS_OK(m_RpcStatus)) {
		return m_RpcStatus;
	}

	m_RpcStatus = RpcServerRegisterIf2(rpc_retc_v2_5_s_ifspec, nullptr, nullptr, RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH, RPC_C_LISTEN_MAX_CALLS_DEFAULT, static_cast<unsigned>(-1), nullptr);

	if (!STATUS_OK(m_RpcStatus)) {
		return m_RpcStatus;
	}

	m_listenThread = std::thread(RpcServerListenThreadProc);

	return m_RpcStatus;
}

bool RPCReceiver::shutdown() {
	RpcMgmtStopServerListening(nullptr);
	if (m_listenThread.joinable()) {
		m_listenThread.join();
	}
	return true;
}

void RPCReceiver::reset() {
	m_RpcStatus = RPC_S_OK;
}

void __RPC_USER CONTEXT_HANDLE_rundown(CONTEXT_HANDLE hContext) {
	disconnect(&hContext);
}

extern std::unique_ptr<SdkManager> sdk_manager;

CONTEXT_HANDLE initialize(handle_t /*hBinding*/, RETCClientConfig * config) {
	if (!sdk_manager->initialize()) {
		return nullptr;
	}

	*config = *sdk_manager->getClientConfig().get();
	return config; // just return something, we dont need to uniquely identify clients
}

RZRESULT playEffect(const RETCDeviceType deviceType, const int type, RZEFFECTID* pEffectID, const efsize_t effectSize, char* effectData, CONTEXT_HANDLE) {
	if (PROCESSING_DELAY > 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(PROCESSING_DELAY));
	}

	return sdk_manager->playEffect(deviceType, type, pEffectID, effectSize, effectData);
}

RZRESULT setEffect(const RZEFFECTID effID, CONTEXT_HANDLE) { //-V813 c code
	return sdk_manager->setEffect(effID);
}

RZRESULT deleteEffect(const RZEFFECTID effID, CONTEXT_HANDLE) { //-V813 c code
	return sdk_manager->deleteEffect(effID);
}

void disconnect(CONTEXT_HANDLE* phContext) {
	sdk_manager->disconnect();
	*phContext = nullptr;
}
