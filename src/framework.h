#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN

// WinApi
#include <windows.h>

// Files and strings
#include <string>
#include <sstream>

// Collections and algorithms
#include <vector>
#include <algorithm>

// COM
#include <objidl.h>
#include <shobjidl.h> 

// GDI+
#include <gdiplus.h>
#pragma comment (lib, "Gdiplus.lib")

// Namespaces
using namespace Gdiplus;
using namespace std;

typedef vector<vector<byte>> Mat;
typedef vector<vector<vector<Rect>>> Bounds;
