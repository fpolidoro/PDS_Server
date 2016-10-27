#pragma once
#include <olectl.h>
#pragma comment(lib, "oleaut32.lib")

class IconIO {
public:
	IconIO();
	~IconIO();
	static HRESULT SaveIcon(HICON icon, LPCSTR path);
};

