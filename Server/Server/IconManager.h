#pragma once
#include <Windows.h>
#include <gdiplus.h>

//#pragma comment (lib,"Gdiplus.lib")

using namespace Gdiplus;

class IconManager {
public:
	IconManager();
	~IconManager();
	int Convert(HICON icon, BYTE **buf);
private:
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
};

