/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/// \file
///



#include "SimpleMutex.h"
#include "RakAssert.h"

using namespace RakNet;




































SimpleMutex::SimpleMutex() //: isInitialized(false)
{







	// Prior implementation of Initializing in Lock() was not threadsafe
	Init();
}

SimpleMutex::~SimpleMutex()
{


}

#ifdef _WIN32
#ifdef _DEBUG
#include <stdio.h>
#endif
#endif

void SimpleMutex::Lock(void)
{
// 	if (isInitialized==false)
// 		Init();
	this->hMutex.lock();

}

void SimpleMutex::Unlock(void)
{
	this->hMutex.unlock();
}

void SimpleMutex::Init(void)
{

//	isInitialized=true;
}
