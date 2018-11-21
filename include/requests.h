
#pragma once

#include "router.h"

namespace graft::supernode::request {

namespace wnd {

//forwards
class WalletManager;

}

void registerRTARequests(graft::Router &router);
void registerForwardRequests(graft::Router &router);
void registerHealthcheckRequests(graft::Router &router);
void registerDebugRequests(Router &router);

namespace wnd
{

void registerWalletRequests(Router &router, wnd::WalletManager&);

}

}

