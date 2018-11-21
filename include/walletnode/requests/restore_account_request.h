#pragma once

#include "router.h"
#include "jsonrpc.h"

namespace graft {

class Context;

namespace wnd {

GRAFT_DEFINE_IO_STRUCT(WalletRestoreAccountRequest,
    (std::string, Password),
    (std::string, Seed)
);

GRAFT_DEFINE_JSON_RPC_REQUEST(WalletRestoreAccountRequestJsonRpc, WalletRestoreAccountRequest)

GRAFT_DEFINE_IO_STRUCT_INITED(WalletRestoreAccountResponse,
    (int, Result, 0)
);

GRAFT_DEFINE_JSON_RPC_REQUEST(WalletRestoreAccountResponseJsonRpc, WalletRestoreAccountResponse)

GRAFT_DEFINE_IO_STRUCT(WalletRestoreAccountCallbackRequest,
    (int,         Result),
    (std::string, WalletId),
    (std::string, Address),
    (std::string, ViewKey),
    (std::string, Account),
    (std::string, Seed)
);

GRAFT_DEFINE_JSON_RPC_REQUEST(WalletRestoreAccountCallbackRequestJsonRpc, WalletRestoreAccountCallbackRequest)

}

}
