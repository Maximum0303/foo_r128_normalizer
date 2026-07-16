#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <winsock2.h>
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>

#include <objbase.h>
#include <objidl.h>
#include <ole2.h>
#include <shlobj.h>
#include <mmsystem.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cwchar>
#include <deque>
#include <iterator>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

#include "../SDK/foobar2000.h"
#include "../SDK/modeless_dialog.h"
#include "../SDK/coreDarkMode.h"

#include "resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
