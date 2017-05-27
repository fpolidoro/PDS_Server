#include "IconManager.h"

IconManager::IconManager() {
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

IconManager::~IconManager() {
	GdiplusShutdown(gdiplusToken);
}

int IconManager::Convert(HICON icon, BYTE **buf) {

	if (icon == nullptr)
		return 0;

	LPSTREAM stream;
	CreateStreamOnHGlobal(NULL, false, &stream);
	if (stream == NULL)
		return 0;

	Bitmap bmp(icon);
	CLSID pngClsid;
	CLSIDFromString(L"{557cf406-1a04-11d3-9a73-0000f81ef32e}", &pngClsid);
	bmp.Save(stream, &pngClsid, NULL);
	//bmp.Save(L"prova.png", &pngClsid, NULL);

	STATSTG streamStats = { 0 };
	if (FAILED(stream->Stat(&streamStats, 0)))
		return 0;

	LARGE_INTEGER li;
	li.QuadPart = 0;
	if (FAILED(stream->Seek(li, STREAM_SEEK_SET, NULL)))
		return 0;

	int size = streamStats.cbSize.QuadPart;

	ULONG bytesSaved = 0;

	*buf = (BYTE*)malloc(sizeof(BYTE)*size);

	if (FAILED(stream->Read(*buf, size, &bytesSaved)))
		return 0;

	li.QuadPart = 0;
	stream->Seek(li, STREAM_SEEK_SET, NULL);

	return size;
}
