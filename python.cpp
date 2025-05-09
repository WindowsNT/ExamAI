#include "pch.h"


std::wstring PythonFolder()
{
	auto pyf = SettingsX->GetRootElement().vv("pyf").GetWideValue();
	if (pyf.empty())
	{
		return datafolder + L"\\python";
	}
	return pyf;
}



#pragma warning(disable:4100)

#pragma warning(disable:4244)
#pragma warning(disable:4267)
#pragma warning(disable:4334)
#include "zip.hpp"
#include "zip.h"
#include "zip.c"

#include <filesystem>


std::wstring& SwapSlash(std::wstring& te)
{
	for (auto& t : te)
	{
		if (t == '\\')
			t = '/';
	}

	return te;
}

extern HICON hIcon1;
std::wstring TempFile3();



ZZIP::ZZIP(void* b, size_t bs)
{
	ystring tf = TempFile3();
	std::vector<char> x(bs);
	memcpy(x.data(), b, bs);
	PutFile(tf.c_str(), x);
	z = zip_open(tf.a_str(), ZIP_DEFAULT_COMPRESSION_LEVEL, 'r');
}

ZZIP::ZZIP(const char* fn, int w)
{
	if (w == 0)
		z = zip_open(fn, ZIP_DEFAULT_COMPRESSION_LEVEL, 'r');
	else
		z = zip_open(fn, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
}

ZZIP::~ZZIP()
{
	if (z)
		zip_close(z);
	z = 0;
}


HRESULT ZZIP::Add(const char* n, void* b, size_t sz)
{
	if (!z)
		return E_NOINTERFACE;

	if (zip_entry_open(z, n) < 0)
		return E_FAIL;
	if (zip_entry_write(z, b, sz) < 0)
	{
		zip_entry_close(z);
		return E_FAIL;
	}
	zip_entry_close(z);
	return S_OK;
}


void ZZIP::Enum(std::vector< ZZIP_ENTRY>& ze)
{
	auto n = zip_entries_total(z);
	ze.resize(n);
	for (long long i = 0; i < n; ++i) {
		zip_entry_openbyindex(z, i);
		{
			const char* name = zip_entry_name(z);
			int isdir = zip_entry_isdir(z);
			unsigned long long size = zip_entry_size(z);
			unsigned int crc32e = zip_entry_crc32(z);

			ze[i].name = name;
			ze[i].isdir = isdir;
			ze[i].size = size;
			ze[i].xcrc32 = crc32e;
		}
		zip_entry_close(z);
	}
}


HRESULT ZZIP::Extract(const char* n, std::vector<char>& d)
{
	if (zip_entry_open(z, n) < 0)
		return E_NOINTERFACE;
	unsigned long long size = zip_entry_size(z);
	d.resize(size);
	if (zip_entry_noallocread(z, d.data(), size) < 0)
		return E_FAIL;
	zip_entry_close(z);
	return S_OK;
}

HRESULT ZZIP::ExtractFile(const char* n, const wchar_t* put)
{
	std::vector<char> d;
	auto hr = Extract(n, d);
	if (FAILED(hr))
		return hr;
	PutFile(put, d, true);
	return hr;
}


#pragma warning(default:4244)
#pragma warning(default:4267)



std::function<void()> _wait;
static std::vector<char> f;
static std::string _TheLink;
struct MODULEX
{
	static std::vector<char> Fetch(const char* TheLink, HWND hTask)
	{
		std::vector<char> aaaa;
		auto fu = [&](const char* TheLink, std::vector<char>* aaaa)
			{
				// Create thread that will show download progress
				DWORD Size;
				unsigned long bfs = 1000;
				TCHAR ss[1000];
				DWORD TotalTransferred = 0;


				int err = 1;

				HINTERNET hI = 0, hRead = 0;

				hI = InternetOpen(L"NN", INTERNET_OPEN_TYPE_DIRECT, 0, 0, 0);
				if (!hI)
					goto finish;
				hRead = InternetOpenUrlA(hI, TheLink, 0, 0, INTERNET_FLAG_NO_CACHE_WRITE, 0);
				if (!hRead)
					goto finish;

				if (!HttpQueryInfo(hRead, HTTP_QUERY_CONTENT_LENGTH, ss, &bfs, 0))
					Size = (DWORD)-1;
				else
				{
					Size = _ttoi(ss);
					if (hTask)
					{
						SendMessage(hTask, TDM_SET_PROGRESS_BAR_RANGE, 0, MAKELPARAM(0, 100));
					}
				}


				for (;;)
				{
					DWORD n;
					char Buff[100010] = { 0 };



					memset(Buff, 0, 100010);
					BOOL  F = InternetReadFile(hRead, Buff, 100000, &n);
					if (F == false)
					{
						err = 2;
						break;
					}
					if (n == 0)
					{
						// End of file !
						err = 0;
						break;
					}
					TotalTransferred += n;

					if (hTask)
					{
						unsigned long long NewPos = 0;
						if (Size != -1)
							NewPos = (100 * (unsigned long long)TotalTransferred) / (unsigned long long)Size;
						SendMessage(hTask, TDM_SET_PROGRESS_BAR_POS, (WPARAM)NewPos, 0);
					}

					//Write to File !
					//char xx = Buff[n];
					size_t olds = aaaa->size();
					aaaa->resize(olds + n);
					memcpy(aaaa->data() + olds, Buff, n);

					int NewPos = 0;
					if (Size != -1)
						NewPos = (100 * TotalTransferred) / Size;
				}

			finish:
				InternetCloseHandle(hRead);
				InternetCloseHandle(hI);
			};

		fu(TheLink, &aaaa);
		return aaaa;
	}

	std::vector<char> FetchShow(const char* TheLink)
	{
		_TheLink = TheLink;
		TASKDIALOGCONFIG tc = { 0 };
		tc.cbSize = sizeof(tc);
		tc.hwndParent = 0;
		tc.pszWindowTitle = L"Turbo Play";
		tc.hMainIcon = hIcon1;
		tc.dwFlags = TDF_ENABLE_HYPERLINKS | TDF_CAN_BE_MINIMIZED | TDF_USE_HICON_MAIN | TDF_SHOW_PROGRESS_BAR;
		tc.pszMainInstruction = L"Installing...";
		tc.pszContent = L"Please wait...";
		tc.dwCommonButtons = TDCBF_CANCEL_BUTTON;
		tc.pfCallback = [](_In_ HWND hwnd, _In_ UINT msg, _In_ WPARAM wParam, _In_ LPARAM lpx, _In_ LONG_PTR lp) ->HRESULT
			{
				if (msg == TDN_CREATED)
				{
					SendMessage(hwnd, TDM_ENABLE_BUTTON, IDCANCEL, 0);

					auto thr = std::thread([&](HWND hwnd)
						{
							f = Fetch(_TheLink.c_str(), hwnd);
							SendMessage(hwnd, TDM_ENABLE_BUTTON, IDCANCEL, 1);
							SendMessage(hwnd, TDM_CLICK_BUTTON, IDCANCEL, 0);
						}, hwnd);
					thr.detach();
				}
				return S_OK;
			};

		int rv = 0;
		BOOL ch = 0;
		f.clear();
		TaskDialogIndirect(&tc, &rv, 0, &ch);
		return f;
	}

	std::vector<char> WaitShow(std::function<void()> w)
	{
		_wait = w;
		TASKDIALOGCONFIG tc = { 0 };
		tc.cbSize = sizeof(tc);
		tc.hwndParent = 0;
		tc.pszWindowTitle = L"NN";
		tc.hMainIcon = hIcon1;
		tc.dwFlags = TDF_ENABLE_HYPERLINKS | TDF_CAN_BE_MINIMIZED | TDF_USE_HICON_MAIN | TDF_SHOW_MARQUEE_PROGRESS_BAR;
		tc.pszMainInstruction = s(22);
		tc.pszContent = s(22);
		tc.dwCommonButtons = TDCBF_CANCEL_BUTTON;
		tc.pfCallback = [](_In_ HWND hwnd, _In_ UINT msg, _In_ WPARAM wParam, _In_ LPARAM lpx, _In_ LONG_PTR lp) ->HRESULT
			{
				if (msg == TDN_CREATED)
				{
					SendMessage(hwnd, TDM_ENABLE_BUTTON, IDCANCEL, 0);
					auto thr = std::thread([&](HWND hwnd)
						{
							if (_wait)
								_wait();
							SendMessage(hwnd, TDM_ENABLE_BUTTON, IDCANCEL, 1);
							SendMessage(hwnd, TDM_CLICK_BUTTON, IDCANCEL, 0);
						}, hwnd);
					thr.detach();
				}
				return S_OK;
			};

		int rv = 0;
		BOOL ch = 0;
		TaskDialogIndirect(&tc, &rv, 0, &ch);
		return f;
	}

	MODULEX(const wchar_t* fo, const wchar_t* _77, const wchar_t* ffmp)
	{
		_folder = fo;
		_7z = _77;
		if (ffmp)
			FFmpegBinary = ffmp;
	}

	std::wstring FFmpegBinary;
	std::wstring _folder;
	std::wstring _7z;
	std::wstring folder()
	{
		return _folder;
	}

	bool RunP(const wchar_t* fu, const wchar_t* par)
	{
		SHELLEXECUTEINFO sei = { 0 };
		sei.cbSize = sizeof(sei);
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;
		sei.lpFile = fu;
		sei.lpParameters = par;
		ShellExecuteEx(&sei);
		WaitShow([&]()
			{
				WaitForSingleObject(sei.hProcess, INFINITE);
				CloseHandle(sei.hProcess);
			});
		return true;
	}

	bool InstallPythonPackage(const char* force_url)
	{
		ystring url;
		url.Format(L"https://www.turbo-play.com/redist/python3116.7z");
		if (force_url && strlen(force_url) > 5)
			url.Format(L"%S", force_url);
		auto F = FetchShow(url.a_str());
		auto mf2 = folder();
		mf2 += L"\\python.7z";
		if (1)
		{
			PutFile(mf2.c_str(), F);
			if (mf2.empty())
				return 0;
		}
		std::vector<wchar_t> p7(1000);
		swprintf_s(p7.data(), 1000, L"x -y -o\"%s\" \"%s\"", folder().c_str(), mf2.c_str());
		// f:\tp2\aed\7zr e -y -or:\fd 1_1_1_0.sf2
		SHELLEXECUTEINFO sei = { 0 };
		sei.cbSize = sizeof(sei);
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;
		sei.lpFile = _7z.c_str();
		sei.lpParameters = p7.data();
		ShellExecuteEx(&sei);
		WaitShow([&]()
			{
				WaitForSingleObject(sei.hProcess, INFINITE);
				CloseHandle(sei.hProcess);
			});
		DeleteFile(mf2.c_str());
		return 1;
	}

	void PipInstall(const wchar_t* packages)
	{
		PROCESS_INFORMATION pInfo = { 0 };
		STARTUPINFO sInfo = { 0 };
		sInfo.cb = sizeof(sInfo);
		wchar_t yy[1000] = {};
		swprintf_s(yy, 1000, L"\"%s\\scripts\\pip.exe\" install %s", folder().c_str(), packages);
		CreateProcess(0, yy, 0, 0, 0, CREATE_NEW_CONSOLE, 0, 0, &sInfo, &pInfo);
		SetPriorityClass(pInfo.hProcess, IDLE_PRIORITY_CLASS);
		SetThreadPriority(pInfo.hThread, THREAD_PRIORITY_IDLE);
		WaitForSingleObject(pInfo.hProcess, INFINITE);
		DWORD ec = 0;
		GetExitCodeProcess(pInfo.hProcess, &ec);
		CloseHandle(pInfo.hProcess);
		CloseHandle(pInfo.hThread);
	}


	void PipRemove(const wchar_t* packages)
	{
		PROCESS_INFORMATION pInfo = { 0 };
		STARTUPINFO sInfo = { 0 };
		sInfo.cb = sizeof(sInfo);
		wchar_t yy[1000] = {};
		swprintf_s(yy, 1000, L"\"%s\\scripts\\pip.exe\" uninstall -y %s", folder().c_str(), packages);
		CreateProcess(0, yy, 0, 0, 0, CREATE_NEW_CONSOLE, 0, 0, &sInfo, &pInfo);
		SetPriorityClass(pInfo.hProcess, IDLE_PRIORITY_CLASS);
		SetThreadPriority(pInfo.hThread, THREAD_PRIORITY_IDLE);
		WaitForSingleObject(pInfo.hProcess, INFINITE);
		DWORD ec = 0;
		GetExitCodeProcess(pInfo.hProcess, &ec);
		CloseHandle(pInfo.hProcess);
		CloseHandle(pInfo.hThread);
	}


	bool Install7ZipUrl(const char* zurl)
	{
		auto F = FetchShow(zurl);
		auto mf2 = folder();
		mf2 += L"\\test.7z";
		if (1)
		{
			PutFile(mf2.c_str(), F);
			if (mf2.empty())
				return 0;
		}
		std::vector<wchar_t> p7(1000);
		swprintf_s(p7.data(), 1000, L"x -y -o\"%s\" \"%s\"", folder().c_str(), mf2.c_str());
		// f:\tp2\aed\7zr e -y -or:\fd 1_1_1_0.sf2
		SHELLEXECUTEINFO sei = { 0 };
		sei.cbSize = sizeof(sei);
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;
		sei.lpFile = _7z.c_str();
		sei.lpParameters = p7.data();
		ShellExecuteEx(&sei);
		WaitShow([&]()
			{
				WaitForSingleObject(sei.hProcess, INFINITE);
				CloseHandle(sei.hProcess);
			});
		DeleteFile(mf2.c_str());
		return 1;
	}

	bool InstallZipUrl(const char* url, std::wstring fo = {})
	{
		if (!url)
			return false;
		if (fo.empty())
			fo = folder();
		auto fe = FetchShow(url);
		if (fe.empty())
			return 0;
		WaitShow([&]()
			{
				ZZIP z((void*)fe.data(), fe.size());
				std::vector<ZZIP_ENTRY> ze;
				z.Enum(ze);
				for (size_t i = 0; i < ze.size(); i++)
				{
					auto& z2 = ze[i];
					ystring nd;
					nd.Format(L"%s\\%S", fo.c_str(), z2.name.c_str());
					if (z2.isdir)
					{
						SHCreateDirectory(0, nd);
						continue;
					}
					z.ExtractFile(z2.name.c_str(), nd.c_str());
				}

			});

		return 1;
	}

};

static signed int CudaAv = 0;
bool IsCudaAvailable()
{
	if (CudaAv == 1)
		return true;
	if (CudaAv == -1)
		return false;


	CudaAv = -1;
	CComPtr<IDXGIFactory> pFactory = NULL;
	CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
	for (UINT ia = 0; ; ia++)
	{
		CComPtr<IDXGIAdapter> ad;
		pFactory->EnumAdapters(ia, &ad);
		if (!ad)
			break;

		DXGI_ADAPTER_DESC desc = {};
		ad->GetDesc(&desc);
		if (desc.VendorId == 4318)
			CudaAv = 1;
		for (UINT io = 0; ; io++)
		{
			CComPtr<IDXGIOutput> out;
			ad->EnumOutputs(io, &out);
			if (!out)
				break;

			DXGI_OUTPUT_DESC desc2 = {};
			out->GetDesc(&desc2);
		}
	}

	if (CudaAv == 1)
		return true;
	return false;
}




HRESULT InstallPython()
{
	std::wstring fold = PythonFolder();

	auto exec = fold + L"\\python.exe";
	if (PathFileExists(exec.c_str()))
		return S_FALSE;

	MODULEX m(fold.c_str(), L"7zr.exe", 0);
	if (!m.InstallPythonPackage(0))
		return E_FAIL;

	if (IsCudaAvailable())
		m.PipInstall(L"torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu121");
	else
		m.PipInstall(L"torch torchvision torchaudio");

	m.PipInstall(L"numpy onnx onnxruntime onnxruntime-directml");
	return S_OK;
}

