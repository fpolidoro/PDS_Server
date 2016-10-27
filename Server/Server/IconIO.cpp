#include "IconIO.h"

HRESULT IconIO::SaveIcon(HICON hIcon, LPCSTR path) {
	// Create the IPicture intrface
	PICTDESC desc = { sizeof(PICTDESC) };
	desc.picType = PICTYPE_ICON;
	desc.icon.hicon = hIcon;
	IPicture* pPicture = 0;
	HRESULT hr = OleCreatePictureIndirect(&desc, IID_IPicture, FALSE, (void**)&pPicture);
	if (FAILED(hr)) return hr;

	// Create a stream and save the image
	IStream* pStream = 0;
	CreateStreamOnHGlobal(0, TRUE, &pStream);
	LONG cbSize = 0;
	hr = pPicture->SaveAsFile(pStream, TRUE, &cbSize);

	// Write the stream content to the file
	if (!FAILED(hr)) {
		HGLOBAL hBuf = 0;
		GetHGlobalFromStream(pStream, &hBuf);
		void* buffer = GlobalLock(hBuf);
		HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		if (!hFile) hr = HRESULT_FROM_WIN32(GetLastError());
		else {
			DWORD written = 0;
			WriteFile(hFile, buffer, cbSize, &written, 0);
			CloseHandle(hFile);
		}
		GlobalUnlock(buffer);
	}
	// Cleanup
	pStream->Release();
	pPicture->Release();
	return hr;
}
