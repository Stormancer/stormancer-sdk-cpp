/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#ifndef __SOCKET_DEFINES_H
#define __SOCKET_DEFINES_H

 /// Internal
#ifdef GFWL
#define accept__ XSocketAccept
#define connect__ XSocketConnect
#define closesocket__ XSocketClose
#define socket__ XSocketCreate
#define bind__ XSocketBind
#define getsockname__ XSocketGetSockName
#define getsockopt__ XSocketGetSockOpt
#define inet_addr__ XSocketInet_Addr
#define ioctlsocket__ XSocketIOCTLSocket
#define listen__ XSocketListen
#define recv__ XSocketRecv
#define recvfrom__ XSocketRecvFrom
#define select__ XSocketSelect
#define send__ XSocketSend
#define sendto__ XSocketSendTo
#define setsockopt__ XSocketSetSockOpt
#define shutdown__ XSocketShutdown
#define WSASendTo__ XWSASendTo



























































#elif   defined(WINDOWS_STORE_RT)
#include "WinRTSocketAdapter.h"
#define accept__ WinRTAccept
#define connect__ WinRTConnect
#define closesocket__ WinRTClose
#define socket__ WinRTCreateDatagramSocket
#define bind__ WinRTBind
#define getsockname__ RNS2_WindowsStore8::WinRTGetSockName
#define getsockopt__ WinRTGetSockOpt
#define inet_addr__ RNS2_WindowsStore8::WinRTInet_Addr
#define ioctlsocket__ RNS2_WindowsStore8::WinRTIOCTLSocket
#define listen__ WinRTListen
#define recv__ WinRTRecv
#define recvfrom__ WinRTRecvFrom
#define select__ WinRTSelect
#define send__ WinRTSend
#define sendto__ WinRTSendTo
#define setsockopt__ RNS2_WindowsStore8::WinRTSetSockOpt
#define shutdown__ WinRTShutdown
#define WSASendTo__ WinRTSendTo






























#else



#if defined(_WIN32)
#define closesocket__ closesocket
#define select__ select
#elif defined(__native_client__)
	// namespace RakNet { void CloseSocket(SOCKET s); }
	// #define closesocket__ RakNet::CloseSocket
#define select__ select
#else
#define closesocket__ close
#define select__ select
// On Unix-like systems, sockets are ints (see SocketIncludes.h)
typedef int SOCKET;
#endif
#define accept__ accept
#define connect__ connect



#define socket__ socket

#define bind__ bind
#define getsockname__ getsockname
#define getsockopt__ getsockopt



#define inet_addr__ inet_addr

#define ioctlsocket__ ioctlsocket
#define listen__ listen
#define recv__ recv
#define recvfrom__ recvfrom



#define sendto__ sendto

#define send__ send



#define setsockopt__ setsockopt

#define shutdown__ shutdown
#define WSASendTo__ WSASendTo
#endif

#endif
