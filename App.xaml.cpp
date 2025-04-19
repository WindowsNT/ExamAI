#include "pch.h"
#include "App.xaml.h"
#include "MainWindow.xaml.h"




bool is_light_theme();


using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;



WNDPROC wProc = 0;
HICON hIcon1 = 0;
std::map<HWND, winrt::Windows::Foundation::IInspectable> windows;
LRESULT CALLBACK cbx(HWND hh, UINT mm, WPARAM ww, LPARAM ll)
{
    return CallWindowProc(wProc, hh, mm, ww, ll);
}


void PostThemeChange()
{
    int Theme = SettingsX->GetRootElement().vv("Theme").GetValueInt();
    if (Theme == 0)
    {
        if (is_light_theme())
            Theme = 1;
        else
            Theme = 2;
    }

    auto et = winrt::Microsoft::UI::Xaml::ElementTheme::Dark;
	if (Theme == 1)
		et = winrt::Microsoft::UI::Xaml::ElementTheme::Light;   


    for (auto& w : windows)
    {
        BOOL value = true;
        if (Theme == 1)
            value = false;
        DwmSetWindowAttribute(w.first, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
		auto w2 = w.second.as<::ExamAI::MainWindow>();

        // and any navigation page
		auto topnv = w2.Content().try_as<FrameworkElement>();
        if (topnv)
        {
			topnv.RequestedTheme(et);   

            // and the frame
            auto fr = topnv.FindName(L"contentFrame");
            if (fr)
            {
                auto fr2 = fr.try_as<Frame>();
                if (fr2)
                {
                    fr2.RequestedTheme(et);
                    auto c = fr2.Content();
                    if (c)
                    {
                        // and the content
                        auto c2 = c.try_as<FrameworkElement>();
                        if (c2)
                            c2.RequestedTheme(et);
                    }
                }
            }
            topnv.UpdateLayout();

        }
    }
}


winrt::ExamAI::MainWindow CreateWi()
{
    winrt::ExamAI::MainWindow j;
    j.Activate();
    j.ExtendsContentIntoTitleBar(true);
    static int One = 0;

    auto n = j.as<::IWindowNative>();
    if (n)
    {
        HWND hh;
        n->get_WindowHandle(&hh);
        if (hh)
        {
            j.wnd((int64_t)hh);
            windows[hh] = j;
            hIcon1 = LoadIcon(GetModuleHandle(0), L"ICON_1");

            wProc = (WNDPROC)GetWindowLongPtr(hh, GWLP_WNDPROC);
            SetWindowLongPtr(hh, GWLP_WNDPROC, (LONG_PTR)cbx);


            SetWindowText(hh, ttitle);
            if (One == 0)
                ShowWindow(hh, SW_SHOWMAXIMIZED);
            One = 1;
#define GCL_HICONSM         (-34)
#define GCL_HICON           (-14)
            SetClassLongPtr(hh, GCL_HICONSM, (LONG_PTR)hIcon1);
            SetClassLongPtr(hh, GCL_HICON, (LONG_PTR)hIcon1);

            if (1)
            {
                BOOL value = true;

                if (SettingsX)
                {
                    int Theme = SettingsX->GetRootElement().vv("Theme").GetValueInt();
                    if (Theme == 0)
                    {
						if (is_light_theme())
							Theme = 1;
						else
							Theme = 2;
                    }
                    if (Theme == 1)
                        value = false;
                    ::DwmSetWindowAttribute(hh, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

                }
            }
        }
    }
    return j;
}








namespace winrt::ExamAI::implementation
{
    /// <summary>
    /// Initializes the singleton application object.  This is the first line of authored code
    /// executed, and as such is the logical equivalent of main() or WinMain().
    /// </summary>
    App::App()
    {
        // Xaml objects should not call InitializeComponent during construction.
        // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        if (SettingsX)
        {
            int Theme = SettingsX->GetRootElement().vv("Theme").GetValueInt();
            if (Theme == 1)
                RequestedTheme(ApplicationTheme::Light);
            if (Theme == 2)
                RequestedTheme(ApplicationTheme::Dark);
        }

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
        UnhandledException([](IInspectable const&, UnhandledExceptionEventArgs const& e)
        {
            if (IsDebuggerPresent())
            {
                auto errorMessage = e.Message();
                __debugbreak();
            }
        });
#endif
    }

    /// <summary>
    /// Invoked when the application is launched.
    /// </summary>
    /// <param name="e">Details about the launch request and process.</param>
    void App::OnLaunched([[maybe_unused]] LaunchActivatedEventArgs const& e)
    {
        window = CreateWi();
    }
}

// Helper
CComPtr<IWICBitmap> LoadWic(const wchar_t* o);
void SaveWic(IWICBitmapSource* src, const wchar_t* out);
extern CComPtr<IWICImagingFactory> wbfact;

void ChangeAssets(const wchar_t* src, const wchar_t* dir_dest)
{
    // Wic Bitmap Loader
	auto png = LoadWic(src);
    for (const auto& entry : std::filesystem::directory_iterator(dir_dest))
    {
        auto png2 = LoadWic(entry.path().c_str());
        if (!png2)
            continue;
        UINT wi = 0, he = 0;
        png2->GetSize(&wi, &he);

		CComPtr<IWICBitmapScaler> pScaler;  
		wbfact->CreateBitmapScaler(&pScaler);
		pScaler->Initialize(png, wi, he, WICBitmapInterpolationModeHighQualityCubic);

        png2 = 0;
        DeleteFile(entry.path().c_str());
		SaveWic(pScaler, entry.path().c_str());
    }
}


int __stdcall wWinMain(HINSTANCE h, HINSTANCE, [[maybe_unused]] PWSTR t, int)
{
    CoInitializeEx(0, COINIT_APARTMENTTHREADED);
//    ChangeAssets(L"f:\\wuitools\\ExamAI\\app.png", L"f:\\wuitools\\ExamAI\\assets");
    hIcon1 = LoadIcon(h, L"ICON_1");
    {
        void (WINAPI * pfnXamlCheckProcessRequirements)();
        auto module = ::LoadLibrary(L"Microsoft.ui.xaml.dll");
        if (module)
        {
            pfnXamlCheckProcessRequirements = reinterpret_cast<decltype(pfnXamlCheckProcessRequirements)>(GetProcAddress(module, "XamlCheckProcessRequirements"));
            if (pfnXamlCheckProcessRequirements)
            {
                (*pfnXamlCheckProcessRequirements)();
            }

            ::FreeLibrary(module);
        }
    }

    PWSTR p = 0;
    SHGetKnownFolderPath(FOLDERID_ProgramData, 0, 0, &p);
    std::wstring de = p;
    CoTaskMemFree(p);

    de += L"\\25E65495-600E-4EC3-99B6-6C71F19360F0";
    SHCreateDirectory(0, de.c_str());
    datafolder = de.c_str();
    std::wstring sf = de + L"\\settings.xml";
    SettingsX = std::make_shared<XML3::XML>(sf.c_str());


    // Extract the 7zr.exe there
	std::wstring ff = de + L"\\7zr.exe";
	if (!std::filesystem::exists(ff))
	{
		auto f = ExtractResource(GetModuleHandle(0), L"Zr", L"DATA");
		if (f.size() > 100)
		{
			SaveBin(ff.c_str(), f);
		}
	}

    // Also the weights if not there
    if (1)
    {
        std::vector<float> w1 = ExtractResource<float>(GetModuleHandle(0), L"WE1", L"DATA");
        std::vector<float> w2 = ExtractResource<float>(GetModuleHandle(0), L"WE2", L"DATA");
        std::vector<float> w3 = ExtractResource<float>(GetModuleHandle(0), L"WE3", L"DATA");
        
        std::wstring ff1 = de + L"\\net_0_weight.bin";
        PutFile<float>(ff1.c_str(), w1, false);

        std::wstring ff2 = de + L"\\net_3_weight.bin";
        PutFile<float>(ff2.c_str(), w2, false);

        std::wstring ff3 = de + L"\\net_6_weight.bin";
        PutFile<float>(ff3.c_str(), w3, false);
    }

    // And the biases if not there
    if (1)
    {
        std::vector<float> w1 = ExtractResource<float>(GetModuleHandle(0), L"BI1", L"DATA");
        std::vector<float> w2 = ExtractResource<float>(GetModuleHandle(0), L"BI2", L"DATA");
        std::vector<float> w3 = ExtractResource<float>(GetModuleHandle(0), L"BI3", L"DATA");

        std::wstring ff1 = de + L"\\net_0_bias.bin";
        PutFile<float>(ff1.c_str(), w1, false);

        std::wstring ff2 = de + L"\\net_3_bias.bin";
        PutFile<float>(ff2.c_str(), w2, false);

        std::wstring ff3 = de + L"\\net_6_bias.bin";
        PutFile<float>(ff3.c_str(), w3, false);
    }


    winrt::init_apartment(winrt::apartment_type::single_threaded);
    ::winrt::Microsoft::UI::Xaml::Application::Start(
        [](auto&&)
        {
            ::winrt::make<::winrt::ExamAI::implementation::App>();
        });

    SettingsX->Save();
    return 0;
}

