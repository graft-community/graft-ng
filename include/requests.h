
#pragma once

#include "router.h"

namespace graft::supernode::request {

//forwards
class WalletManager;

void registerRTARequests(graft::Router &router);
void registerForwardRequests(graft::Router &router);
void registerHealthcheckRequests(graft::Router &router);
void registerDebugRequests(Router &router);
void registerWalletRequests(Router &router, WalletManager&);

}

