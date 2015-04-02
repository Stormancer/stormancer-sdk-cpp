#pragma once

// standart libs
#include <stdio.h>
#include <tchar.h>
#include <stdint.h>
#include <streambuf>
#include <iostream>
#include <string>
#include <map>
#include <list>
#include <vector>
#include <sstream>
#include <ctime>
#include <functional>
#include <iomanip>
#include <memory>
#include <future>
#include <regex>
#include <cstdarg>
#include <clocale>
#include <locale>
#include <codecvt>
#include <cctype>
#include <algorithm>
#include <typeinfo>

// custom libs

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <ppltasks.h>
#include <pplx/pplxtasks.h>
#include <rx.hpp>

namespace rx = rxcpp;
namespace rxu = rxcpp::util;
namespace rxsc = rxcpp::schedulers;
namespace rxsub = rxcpp::subjects;

// packed libs
#include <MsgPack.h>

// custom types
#include "typedef.h"
