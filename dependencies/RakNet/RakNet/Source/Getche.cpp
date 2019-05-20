/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#if defined(_XBOX) || defined(_XBOX_720_COMPILE_AS_WINDOWS) || defined(X360)

#elif defined(_WIN32) && !defined(_XBOX) && !defined(_XBOX_720_COMPILE_AS_WINDOWS) && !defined(X360)
#include <conio.h> /* getche() */
#elif defined(__S3E__)

#else

#include "Getche.h"

char getche()
{


	struct termios oldt,
		newt;
	char            ch;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;

}
#endif
