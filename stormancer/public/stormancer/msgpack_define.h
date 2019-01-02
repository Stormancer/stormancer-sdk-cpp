#pragma once

#if !defined(_NO_MSGPACK)

#include "stormancer/headers.h"

#else

#define MSGPACK_DEFINE(...)
#define MSGPACK_DEFINE_MAP(...)
#define MSGPACK_DEFINE_ARRAY(...)
#define MSGPACK_ADD_ENUM(...)

#endif
