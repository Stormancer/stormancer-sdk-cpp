/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#if defined(_WIN32)
#include "WindowsIncludes.h" // Sleep
#else
#include <chrono>
#include <thread>
#endif

#include "RakSleep.h"


#if defined(WINDOWS_PHONE_8) || defined(WINDOWS_STORE_RT)
#include "ThreadEmulation.h"
using namespace ThreadEmulation;
#endif

void RakSleep(unsigned int ms)
{









#ifdef _WIN32
	Sleep(ms);
#else
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}
