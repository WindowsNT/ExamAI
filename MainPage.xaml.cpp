#include "pch.h"
#include "MainPage.xaml.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::ExamAI::implementation
{
    void MainPage::Refresh(const wchar_t* s)
    {
        m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ s });
    }


    void MainPage::OnTrain(IInspectable, IInspectable)
    {
        void Train();
        Train();
    }


    void MainPage::OnNew(IInspectable const&, IInspectable const&)
    {

    }
    void MainPage::OnOpen(IInspectable const&, IInspectable const&)
    {

    }
    void MainPage::OnSave(IInspectable const&, IInspectable const&)
    {

    }
    void MainPage::OnExit(IInspectable const&, IInspectable const&)
    {
        PostMessage((HWND)wnd(), WM_CLOSE, 0, 0);
    }
    void MainPage::OnSaveAs(IInspectable const&, IInspectable const&)
    {

    }

    void MainPage::OnDark(IInspectable const&, IInspectable const&)
    {
        SettingsX->GetRootElement().vv("Theme").SetValueInt(2);
        SettingsX->Save();
        void PostThemeChange();
        PostThemeChange();

    }
    void MainPage::OnLight(IInspectable const&, IInspectable const&)
    {
        SettingsX->GetRootElement().vv("Theme").SetValueInt(1);
        SettingsX->Save();
        void PostThemeChange();
        PostThemeChange();

    }
    void MainPage::OnDefault(IInspectable const&, IInspectable const&)
    {
        SettingsX->GetRootElement().vv("Theme").SetValueInt(0);
        SettingsX->Save();
        void PostThemeChange();
        PostThemeChange();
    }


}
