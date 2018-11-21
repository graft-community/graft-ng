#pragma once

#include "router.h"
#include "jsonrpc.h"

namespace graft {

class Context;

namespace wnd {

GRAFT_DEFINE_IO_STRUCT(WalletCreateAccountRequest,
    (std::string, Password),
    (std::string, Language)
);

GRAFT_DEFINE_JSON_RPC_REQUEST(WalletCreateAccountRequestJsonRpc, WalletCreateAccountRequest)

GRAFT_DEFINE_IO_STRUCT_INITED(WalletCreateAccountResponse,
    (int, Result, 0)
);

GRAFT_DEFINE_JSON_RPC_REQUEST(WalletCreateAccountResponseJsonRpc, WalletCreateAccountResponse)

}

}
