#pragma once
#include "../rpc-midl/rpc_retc.h"
#include "SDKManager.h"
#include <thread>

#pragma once
class RPCReceiver {
public:
	RPCReceiver();
	bool startListening();
	void reset();

	bool shutdown();

private:
	void setStatus(RPC_STATUS stat) { m_RpcStatus = stat; }
	RPC_STATUS getStatus() const { return m_RpcStatus; }

	RPC_STATUS m_RpcStatus;
	std::thread m_listenThread;
};
