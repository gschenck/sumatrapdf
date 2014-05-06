// mh@tin-pot.net

#include "BaseUtil.h"
#include "BaseEngine.h"

#include "WinUtil.h"
#include "FileUtil.h"
#include "Version.h"

// #include "SumatraPDF.h" // MessageBoxWarning()
#include "cJSON.h"  // Does extern "C".

namespace {
    LPWSTR UserTocFilename(const BYTE digest[16]);
    BOOL ReadTocFile(LPWSTR tocFilename, cJSON** pRoot, BOOL bComplain);
    BOOL WriteTocFile(LPWSTR tocFilename, cJSON *pRoot);
}

struct BaseEngine::BaseData {
    BaseData() : json(NULL), bDidMsg(FALSE) {}
    ~BaseData() { cJSON_Delete(json); }
    cJSON* json;
    BOOL bDidMsg;
};

cJSON* BaseEngine::GetJSON() const
{
    return data->json;
}

BaseEngine::BaseEngine()
:   data(new BaseData)
{
    ZeroMemory(md5digest, sizeof md5digest);
}

BaseEngine::~BaseEngine()
{
    delete data;
}

bool BaseEngine::HasTocTree() const
{
    if (data->json == NULL) {
        static const BYTE zeros[sizeof md5digest] = { 0U };

        if (memcmp(md5digest, zeros, sizeof md5digest) == 0) {
            return false;	// Nothing loaded.
        }

        // Get ToC pathname from MD5 hash of doc content.
        ScopedMem<WCHAR> tocFilename(UserTocFilename(md5digest));
        
        if (tocFilename.Get()) {
	    BOOL complain = !data->bDidMsg;
	    BOOL success = ReadTocFile(tocFilename, &data->json, complain);
	    data->bDidMsg = TRUE;
	}
    }

    return data->json != NULL;
}

DocTocItem *BaseEngine::GetTocTree()
{
    return NULL; // DocTocItem is abstract.
}


namespace {

LPWSTR GenTocFilename(LPWSTR fileName)
{
    ScopedMem<WCHAR> path;
    /* Use %APPDATA% */
    path.Set(GetSpecialFolder(CSIDL_APPDATA, true));
    if (path) {
        path.Set(path::Join(path, APP_NAME_STR));
        if (path && !dir::Create(path))
            path.Set(NULL);
    }

    if (!path || !fileName)
        return NULL;

    return path::Join(path, fileName);
}

LPWSTR UserTocFilename(const BYTE digest[16])
{
    ScopedMem<CHAR> fingerPrint(str::MemToHex(digest, 16));

    const size_t size = strlen(fingerPrint) + sizeof ".json";
    ScopedMem<CHAR>  fileName(new CHAR[size]);
    ScopedMem<WCHAR> wFileName(new WCHAR[size]);
    sprintf_s(fileName, size, "%s%s", fingerPrint.Get(), ".json");

    INT res = MultiByteToWideChar(
	CP_UTF8,
	MB_ERR_INVALID_CHARS,
	fileName,
	-1,
	wFileName,
	(int)size);

    if (res == 0) { // Failure.
	return NULL;
    } else {
        return GenTocFilename(wFileName.StealData());
    }
}


BOOL ReadTocFile(LPWSTR tocFilename, cJSON** pRoot, BOOL bComplain)
{
    BOOL bRet = TRUE;
    DWORD cbSize, cbSizeHi;
    DWORD cbRead;
    const DWORD cbSizeMax = 1U << 20;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    CHAR *lpszContent = NULL;
    cJSON *json = NULL;

    hFile = CreateFile(tocFilename,
	FILE_READ_DATA, 
	FILE_SHARE_READ,
	NULL,
	OPEN_EXISTING,
	FILE_FLAG_SEQUENTIAL_SCAN,
	NULL);

    if (hFile == INVALID_HANDLE_VALUE)
	goto done;

    cbSize = GetFileSize(hFile, &cbSizeHi);
    if (cbSizeHi > 0 || cbSize == 0 || cbSize > cbSizeMax)
        goto done;

    lpszContent = new CHAR[cbSize+1];
    bRet = ReadFile(hFile, lpszContent, cbSize, &cbRead, NULL);
    if (!bRet)
	goto done;

    lpszContent[cbSize] = '\0';
    json = cJSON_Parse(lpszContent);
    if (json == 0 && bComplain) {
	// Parse error - give a hint.
	const char *jsonErrorPtr = cJSON_GetErrorPtr();

	if (jsonErrorPtr == 0) {
	    static const char lastGasp[] = "No error information from cJSON ...";
	    jsonErrorPtr = lastGasp;
	}

	size_t len = wcslen(tocFilename) + 80U + 50U;
        WCHAR* msg = new WCHAR[len];
	wsprintf(msg, L"JSON parse error in file:\n\'%s\'\nAt:\n\"%.80S...\"",
	    tocFilename, jsonErrorPtr);
	WCHAR title[] = L"SumatraPDF - JSON Table of Content";
	HWND hwnd = NULL; // No parent window.
        UINT type =  MB_OK | MB_ICONEXCLAMATION;
        MessageBox(hwnd, msg, title, type);
	delete[] msg;
    }

done:
    if (lpszContent != NULL) delete[] lpszContent;
    if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
    *pRoot = json;
    return json != NULL;
}


BOOL WriteTocFile(LPWSTR tocFilename, cJSON *root)
{
    BOOL success = FALSE;
    return success;
}

} // namespace
