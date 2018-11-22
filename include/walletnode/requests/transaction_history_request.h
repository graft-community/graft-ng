#pragma once

#include "router.h"
#include "jsonrpc.h"

namespace graft {

class Context;

namespace wnd {

GRAFT_DEFINE_IO_STRUCT(WalletTransactionHistoryRequest,
    (std::string, WalletId),
    (std::string, Account),
    (std::string, Password)
);

GRAFT_DEFINE_JSON_RPC_REQUEST(WalletTransactionHistoryRequestJsonRpc, WalletTransactionHistoryRequest)

GRAFT_DEFINE_IO_STRUCT_INITED(WalletTransactionHistoryResponse,
    (int, Result, 0)
);

GRAFT_DEFINE_JSON_RPC_REQUEST(WalletTransactionHistoryResponseJsonRpc, WalletTransactionHistoryResponse)

GRAFT_DEFINE_IO_STRUCT(WalletTransactionHistoryCallbackRequest,
    (int,         Result),
    (std::string, History)
);

GRAFT_DEFINE_JSON_RPC_REQUEST(WalletTransactionHistoryCallbackRequestJsonRpc, WalletTransactionHistoryCallbackRequest)

}

}
