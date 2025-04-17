#pragma once
#define DISABLE_XAML_GENERATED_MAIN
#define DML_TARGET_VERSION_USE_LATEST
#include <windows.h>
#include <unknwn.h>
#include <restrictederrorinfo.h>
#include <hstring.h>
#include <queue>
#include <any>
#include <optional>
#include <stack>
#include <mutex>
#include <wininet.h>
#include <vector>
#include <set>
#include <functional>
#include <memory>
#include <limits>
#include <wia.h>
#include <vector>
#include <shlobj.h>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <filesystem>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <random>
#include <dwmapi.h>
#include <uxtheme.h>
#include <atlbase.h>
#include <wincodec.h>
#include <comutil.h>
#include <numeric>

// Undefine GetCurrentTime macro to prevent
// conflict with Storyboard::GetCurrentTime
#undef GetCurrentTime
#undef min
#undef max

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Microsoft.UI.Composition.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Input.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Microsoft.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <microsoft.ui.xaml.window.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <microsoft.ui.xaml.media.dxinterop.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.Windows.AppNotifications.h>
#include <winrt/Microsoft.Windows.AppNotifications.Builder.h>
#include <winrt/Microsoft.UI.Xaml.Navigation.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <wil/cppwinrt_helpers.h>
#include <appmodel.h>
#include <dxgi1_6.h>
#include <DirectML.h>
#include "directmlx.h"
#include "d3dx12.h"
#include "xml3all.h"
#include "ystring.h"
#include "mime2.h"

extern const wchar_t* ttitle;
extern std::shared_ptr<XML3::XML> SettingsX;
extern std::wstring datafolder;
std::wstring PythonFolder();

const wchar_t* s(size_t);

#include "dmllib.hpp"

#include <opencv2/opencv.hpp>

enum class TRAIN_CLASS
{
	EMPTY = 0,
	TICK = 1,
	CROSS = 2,
	DOT = 3,
};


template <typename T = char>
std::vector<T> ExtractResource(HINSTANCE hXX, const TCHAR* Name, const TCHAR* ty)
{
	std::vector<T> data;
	HRSRC R = FindResource(hXX, Name, ty);
	if (!R)
		return {};
	HGLOBAL hG = LoadResource(hXX, R);
	if (!hG)
		return {};
	DWORD S = SizeofResource(hXX, R);
	char* p = (char*)LockResource(hG);
	if (!p)
	{
		FreeResource(R);
		return {};
	}
	data.resize(S / sizeof(T));
	memcpy(data.data(), p, S);
	FreeResource(R);
	return data;
}


template <typename T = float>
std::vector<T> LoadBin(const wchar_t* f)
{
	std::vector<T> d;
	HANDLE hX = CreateFile(f, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if (hX == INVALID_HANDLE_VALUE)
		return {};
	LARGE_INTEGER sz = { 0 };
	GetFileSizeEx(hX, &sz);
	d.resize((size_t)(sz.QuadPart / sizeof(T)));
	DWORD A = 0;
	ReadFile(hX, d.data(), (DWORD)sz.QuadPart, &A, 0);
	CloseHandle(hX);
	if (A != sz.QuadPart)
		return {};
	return d;
}

template <typename T>
bool SaveBin(const std::wstring& filename, const std::vector<T>& data) {
	FILE* f = 0;
	_wfopen_s(&f, filename.c_str(), L"wb");
	if (!f) return false;
	fwrite(data.data(), sizeof(T), data.size(), f);
	fclose(f);
	return true;
}




template <typename T>
bool PutFile(const wchar_t* f, std::vector<T>& d, bool Fw = true)
{
	HANDLE hX = CreateFile(f, GENERIC_WRITE, 0, 0, Fw ? CREATE_ALWAYS : CREATE_NEW, 0, 0);
	if (hX == INVALID_HANDLE_VALUE)
		return false;
	DWORD A = 0;
	WriteFile(hX, d.data(), (DWORD)(d.size() * sizeof(T)), &A, 0);
	CloseHandle(hX);
	if (A != d.size())
		return false;
	return true;
}


class PushPopDir
{
	std::wstring cd;
public:

	PushPopDir(const wchar_t* nd)
	{
		cd = std::filesystem::current_path(); //getting path
		if (nd)
			std::filesystem::current_path(nd);
	}

	~PushPopDir()
	{
		std::filesystem::current_path(cd.c_str());
	}

};

DWORD Run(const wchar_t* y, bool W, DWORD flg);
void Locate([[maybe_unused]] const wchar_t* fi);


