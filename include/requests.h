
#pragma once

#include "router.h"

namespace graft::supernode::request {

void registerRTARequests(graft::Router &router);
void registerForwardRequests(graft::Router &router);
void registerHealthcheckRequests(graft::Router &router);
void registerDebugRequests(Router &router);

}

namespace graft {
namespace walletnode {

//forwards
class WalletManager;

namespace request
{

void registerWalletRequests(Router &router, WalletManager&);

}

}

}

