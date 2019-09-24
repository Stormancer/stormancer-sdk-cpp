#pragma once

#include "RakNetSocket2.h"
#include "RakPeer.h"
#include <string>
#include "stormancer/Tasks.h"

pplx::task<void> testP2P(std::string endpoint, std::string account, std::string application, std::string sceneId, int guestsCount = 1);
