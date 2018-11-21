#pragma once

#include "router.h"
#include "jsonrpc.h"

namespace graft {

class Context;

namespace wnd {

GRAFT_DEFINE_IO_STRUCT(WalletTransferDestination,
                       (std::string, Address),
                       (std::string, Amount)
                       );

GRAFT_DEFINE_IO_STRUCT(WalletPrepareTransferRequest,
    (std::string,                            WalletId),
    (std::string,                            Account),
    (std::string,                            Password),
    (std::vector<WalletTransferDestination>, Destinations)
);

GRAFT_DEFINE_JSON_RPC_REQUEST(WalletPrepareTransferRequestJsonRpc, WalletPrepareTransferRequest)

GRAFT_DEFINE_IO_STRUCT_INITED(WalletPrepareTransferResponse,
    (int, Result, 0)
);

GRAFT_DEFINE_JSON_RPC_REQUEST(WalletPrepareTransferJsonRpc, WalletPrepareTransferResponse)

}

}
