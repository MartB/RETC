#pragma once
#include "../rpc-midl/rpc_retc.h"
#include "SDKManager.h"
#include <thread>

#define STATUS_OK(x) ((x) == RPC_S_OK)

class RPCReceiver {
public:
	RPCReceiver();
	RPC_STATUS startListening();
	void reset();

	bool shutdown();

private:
	void setStatus(const RPC_STATUS stat) { m_RpcStatus = stat; }
	RPC_STATUS getStatus() const { return m_RpcStatus; }

	RPC_STATUS m_RpcStatus;
	std::thread m_listenThread;
};
