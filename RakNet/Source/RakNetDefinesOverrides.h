/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#if !defined(ANDROID)
#define RAKNET_SUPPORT_IPV6 1
#else
#define RAKNET_SUPPORT_IPV6 0
#endif

#if !defined(RAKNET_DEBUG_PRINTF) && !defined(_DEBUG)
#define RAKNET_DEBUG_PRINTF printf
#endif

// USER EDITABLE FILE

