/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "SignaledEvent.h"
#include "RakAssert.h"
#include "RakSleep.h"
#include <thread>
#include <chrono>


#if defined(__GNUC__) 
#include <sys/time.h>
#include <unistd.h>
#endif

using namespace RakNet;





SignaledEvent::SignaledEvent()
{

}
SignaledEvent::~SignaledEvent()
{
	// Intentionally do not close event, so it doesn't close twice on linux
}

void SignaledEvent::InitEvent(void)
{

}

void SignaledEvent::CloseEvent(void)
{

}

void SignaledEvent::SetEvent(void)
{
	condition.notify_all();
}

void SignaledEvent::WaitOnEvent(int timeoutMs)
{
	std::unique_lock<std::mutex> lock(this->_signalLock);
	condition.wait_for(lock, std::chrono::milliseconds(timeoutMs));
}
