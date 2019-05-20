#pragma once

#include "stormancer/BuildConfig.h"



#if defined(_WIN32)
#include "stormancer/Windows/AES/AES_Windows.h"




#else
#include "stormancer/Linux/AES/AES_Linux.h"
#endif
