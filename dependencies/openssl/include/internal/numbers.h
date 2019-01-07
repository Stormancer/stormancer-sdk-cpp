/*
 * Copyright 2015-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef HEADER_NUMBERS_H
# define HEADER_NUMBERS_H

# include <limits.h>



#  define __MAXUINT__(T) ((T) -1)
#  define __MAXINT__(T) ((T) ((((T) 1) << ((sizeof(T) * CHAR_BIT) - 1)) ^ __MAXUINT__(T)))
#  define __MININT__(T) (-__MAXINT__(T) - 1)



















# ifndef INT8_MAX
#  define INT8_MIN __MININT__(int8_t)
#  define INT8_MAX __MAXINT__(int8_t)
#  define UINT8_MAX __MAXUINT__(uint8_t)
# endif

# ifndef INT16_MAX
#  define INT16_MIN __MININT__(int16_t)
#  define INT16_MAX __MAXINT__(int16_t)
#  define UINT16_MAX __MAXUINT__(uint16_t)
# endif

# ifndef INT32_MAX
#  define INT32_MIN __MININT__(int32_t)
#  define INT32_MAX __MAXINT__(int32_t)
#  define UINT32_MAX __MAXUINT__(uint32_t)
# endif

# ifndef INT64_MAX
#  define INT64_MIN __MININT__(int64_t)
#  define INT64_MAX __MAXINT__(int64_t)
#  define UINT64_MAX __MAXUINT__(uint64_t)
# endif

# ifndef SIZE_MAX
#  define SIZE_MAX __MAXUINT__(size_t)
# endif

#endif

