#include <pch.h>


const wchar_t* ttitle = L"ExamAI";
std::shared_ptr<XML3::XML> SettingsX;
std::wstring datafolder;


bool is_light_theme() {
    // based on https://stackoverflow.com/questions/51334674/how-to-detect-windows-10-light-dark-mode-in-win32-application

    // The value is expected to be a REG_DWORD, which is a signed 32-bit little-endian
    char buffer[4];
    DWORD cbData = 4;
    RegGetValueW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"AppsUseLightTheme",
        RRF_RT_REG_DWORD, // expected value type
        nullptr,
        buffer,
        &cbData);

    // convert bytes written to our buffer to an int, assuming little-endian
    auto i = int(buffer[3] << 24 |
        buffer[2] << 16 |
        buffer[1] << 8 |
        buffer[0]);

    return i == 1;
}


CComPtr<IWICImagingFactory> wbfact;

void SaveWic(IWICBitmapSource* src, const wchar_t* out)
{
    if (!wbfact)
        CoCreateInstance(CLSID_WICImagingFactory, 0, CLSCTX_INPROC_SERVER,
            __uuidof(IWICImagingFactory), (void**)&wbfact);

    CComPtr<IWICBitmapEncoder> wicBitmapEncoder;
    wbfact->CreateEncoder(GUID_ContainerFormatPng, 0, &wicBitmapEncoder);
	CComPtr<IWICStream> stream;
	wbfact->CreateStream(&stream);  

	stream->InitializeFromFilename(out, GENERIC_WRITE);
	wicBitmapEncoder->Initialize(stream, WICBitmapEncoderNoCache);
	CComPtr<IWICBitmapFrameEncode> wicFrameEncode;
	wicBitmapEncoder->CreateNewFrame(&wicFrameEncode, 0);
	wicFrameEncode->Initialize(0);
	UINT W = 0, H = 0;
	src->GetSize(&W, &H);
	wicFrameEncode->SetSize(W, H);
    GUID g2 = GUID_WICPixelFormat32bppBGRA;
	wicFrameEncode->SetPixelFormat(&g2);
	wicFrameEncode->WriteSource(src, 0);
	wicFrameEncode->Commit();
	wicBitmapEncoder->Commit();

}
CComPtr<IWICBitmap> LoadWic(const wchar_t* o)
{
	if (!wbfact)
		CoCreateInstance(CLSID_WICImagingFactory, 0, CLSCTX_INPROC_SERVER,
			__uuidof(IWICImagingFactory), (void**)&wbfact);
	CComPtr<IWICBitmapDecoder> pDecoder = NULL;
	CComPtr<IWICBitmapFrameDecode> pSource = NULL;

	auto hr = wbfact->CreateDecoderFromFilename(
		o,
		NULL, GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);
	if (FAILED(hr)) return {};
	hr = pDecoder->GetFrame(0, &pSource);
	if (FAILED(hr)) return {};

    CComPtr<IWICBitmap> bix;
    wbfact->CreateBitmapFromSource(pSource, WICBitmapCacheOnDemand, &bix);
    return bix;
}



int CALLBACK BrowseCallbackProc(
    HWND hwnd,
    UINT uMsg,
    LPARAM,
    LPARAM lpData
)
{
    if (uMsg == BFFM_INITIALIZED)
    {
        ITEMIDLIST* sel = (ITEMIDLIST*)lpData;
        if (!sel)
            return 0;
        SendMessage(hwnd, BFFM_SETSELECTION, false, (LPARAM)sel);
    }
    return 0;
}


std::wstring TempFile3()
{
    std::vector<wchar_t> td(1000);
    GetTempPathW(1000, td.data());

    wcscat_s(td.data(), 1000, L"\\");
    std::vector<wchar_t> x(1000);
    GetTempFileNameW(td.data(), L"tmp", 0, x.data());
    DeleteFile(x.data());

    return x.data();
}


bool BrowseFolder(HWND hh, const TCHAR* tit, const TCHAR* root, const TCHAR* sel, TCHAR* rv)
{
    IShellFolder* sf = 0;
    SHGetDesktopFolder(&sf);
    if (!sf)
        return false;

    BROWSEINFO bi = { 0 };
    bi.hwndOwner = hh;
    if (root && _tcslen(root))
    {
        PIDLIST_RELATIVE pd = 0;
        wchar_t dn[1000] = { 0 };
        wcscpy_s(dn, 1000, root);
        sf->ParseDisplayName(hh, 0, dn, 0, &pd, 0);
        bi.pidlRoot = pd;
    }
    bi.lpszTitle = tit;
    bi.ulFlags = BIF_EDITBOX | BIF_NEWDIALOGSTYLE;
    bi.lpfn = BrowseCallbackProc;
    if (sel && _tcslen(sel))
    {
        PIDLIST_RELATIVE pd = 0;
        wchar_t dn[1000] = { 0 };
        wcscpy_s(dn, 1000, sel);
        sf->ParseDisplayName(hh, 0, dn, 0, &pd, 0);
        bi.lParam = (LPARAM)pd;
    }

    PIDLIST_ABSOLUTE pd = SHBrowseForFolder(&bi);
    if (!pd)
        return false;

    SHGetPathFromIDList(pd, rv);
    CoTaskMemFree(pd);
    return true;
}




#include "english.hpp"


const wchar_t* s(size_t idx)
{
    if (idx > MAX_LANG)
        return L"";
    return z_strings[idx];
}



DWORD Run(const wchar_t* y, bool W, DWORD flg)
{
    PROCESS_INFORMATION pInfo = { 0 };
    STARTUPINFO sInfo = { 0 };

    sInfo.cb = sizeof(sInfo);
    wchar_t yy[1000];
    swprintf_s(yy, 1000, L"%s", y);
    CreateProcess(0, yy, 0, 0, 0, flg, 0, 0, &sInfo, &pInfo);
    SetPriorityClass(pInfo.hProcess, IDLE_PRIORITY_CLASS);
    SetThreadPriority(pInfo.hThread, THREAD_PRIORITY_IDLE);
    if (W)
        WaitForSingleObject(pInfo.hProcess, INFINITE);
    DWORD ec = 0;
    GetExitCodeProcess(pInfo.hProcess, &ec);
    CloseHandle(pInfo.hProcess);
    CloseHandle(pInfo.hThread);
    return ec;
}




void Locate([[maybe_unused]] const wchar_t* fi)
{
    wchar_t buf[1024] = {};
    swprintf_s(buf, 1024, L"/select,\"%s\"", fi);
    ShellExecute(0, L"open", L"explorer.exe", buf, 0, SW_SHOWMAXIMIZED);
}




std::optional<std::vector<std::wstring>> TwFile(HWND hh, const wchar_t* szFolder)
{
    CComPtr<IWiaDevMgr2> d;
    CoCreateInstance(CLSID_WiaDevMgr2, 0, CLSCTX_ALL, __uuidof(IWiaDevMgr2), (void**)&d);
    if (!d)
        return {};

    std::vector<std::wstring> z;

    LONG                nFiles = 0;
    LPWSTR* rFilePaths = NULL;
    CComPtr<IWiaItem2> pItem = NULL;
    //    IWiaItem2* pItemRoot = NULL;
     //   BSTR                devName = NULL;
    auto hr = d->GetImageDlg(0, NULL, hh, _bstr_t(szFolder), _bstr_t(L"scanfile.jpg"), &nFiles, &rFilePaths, &pItem);
    if (FAILED(hr))
        return {};

    for (LONG f = 0; f < nFiles; f++)
    {
        z.push_back(rFilePaths[f]);
        SysFreeString(rFilePaths[f]);
    }

    return z;
}

