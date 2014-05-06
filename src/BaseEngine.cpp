// mh@tin-pot.net

#include "BaseUtil.h"
#include "BaseEngine.h"

#include "WinUtil.h"
#include "FileUtil.h"
#include "Version.h"

#include "cJSON.h"  // Does extern "C".

namespace {
    LPWSTR UserTocFileName(const BYTE digest[16]);
    BOOL ReadTocFile(LPWSTR tocFileName, cJSON** rootPtr, BOOL doComplain);
    BOOL WriteTocFile(LPWSTR tocFileName, cJSON *root);
}

struct BaseEngine::BaseData {
    BaseData() : json(NULL), didMessage(FALSE) {}
    ~BaseData() { cJSON_Delete(json); }
    cJSON* json;
    BOOL didMessage;
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
        ScopedMem<WCHAR> tocFileName(UserTocFileName(md5digest));

        if (tocFileName.Get()) {
            BOOL complain = !data->didMessage;
            BOOL success = ReadTocFile(tocFileName, &data->json, complain);
            data->didMessage = TRUE;
        }
    }

    return data->json != NULL;
}

DocTocItem *BaseEngine::GetTocTree()
{
    return NULL; // DocTocItem is abstract.
}


namespace {

LPWSTR GenTocFileName(LPWSTR fileName)
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

LPWSTR UserTocFileName(const BYTE digest[16])
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
        return GenTocFileName(wFileName.StealData());
    }
}


BOOL ReadTocFile(LPWSTR tocFileName, cJSON** rootPtr, BOOL doComplain)
{
    cJSON* json = 0;

    HANDLE hFile = CreateFile(tocFileName,
        FILE_READ_DATA, 
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        goto done;

    const DWORD cbSizeMax = 1U << 20;
    DWORD cbSizeHi;
    const DWORD cbSize = GetFileSize(hFile, &cbSizeHi);
    if (cbSizeHi > 0 || cbSize == 0 || cbSize > cbSizeMax) {
        goto done;
    } else {
        ScopedMem<CHAR> lpszContent(new CHAR[cbSize+1]);
        DWORD cbRead;
        BOOL bRet = ReadFile(hFile, lpszContent, cbSize, &cbRead, NULL);
        if (!bRet)
            goto done;

        lpszContent[cbSize] = '\0';
        json = cJSON_Parse(lpszContent);

        if (json == 0 && doComplain) {
            // Parse error - get and give a hint.
            const char *jsonErrorPtr = cJSON_GetErrorPtr();

            if (jsonErrorPtr == 0) {
                static const char lastGasp[] = "No error information from cJSON ...";
                jsonErrorPtr = lastGasp;
            }

            // Set up a message box.
            const size_t len = wcslen(tocFileName) + 80U + 50U;
            ScopedMem<WCHAR> msg(new WCHAR[len]);
            wsprintf(msg.Get(),
                L"JSON parse error in file:\n\'%s\'\nAt:\n\"%.80S...\"",
                tocFileName, jsonErrorPtr);
            const WCHAR title[] = L"SumatraPDF - JSON Table of Content";
            const HWND hwnd = NULL; // No parent window.
            const UINT type =  MB_OK | MB_ICONEXCLAMATION;

            MessageBox(hwnd, msg, title, type);
        }
    }

done:
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    *rootPtr = json;
    return json != NULL;
}


BOOL WriteTocFile(LPWSTR tocFileName, cJSON *root)
{
    return FALSE; // Not implemented.
}

} // namespace
