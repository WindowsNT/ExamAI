#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::ExamAI::implementation
{
	void MainWindow::ItemInvoked(IInspectable, NavigationViewItemInvokedEventArgs ar)
	{
		auto topnv = Content().as<NavigationView>();
		if (!topnv)
			return;
		Frame fr = topnv.FindName(L"contentFrame").as<Frame>();
		if (!fr)
			return;
		if (ar.IsSettingsInvoked())
		{
			fr.Navigate(winrt::xaml_typename<winrt::ExamAI::Settings>());
			return;
		}
		auto it = ar.InvokedItemContainer().as<NavigationViewItem>();
		if (!it)
			return;
		auto n = it.Name();
		if (n == L"ViewMain")
			fr.Navigate(winrt::xaml_typename<winrt::ExamAI::MainPage>());
		if (n == L"ViewLight")
		{
			auto ct = SettingsX->GetRootElement().vv("Theme").GetValueInt(0);
			if (ct == 0)
			{
				bool is_light_theme();
				if (is_light_theme())
					ct = 1;
				else
					ct = 2;
			}
			if (ct == 1)
				SettingsX->GetRootElement().vv("Theme").SetValueInt(2);
			else
				SettingsX->GetRootElement().vv("Theme").SetValueInt(1);
			SettingsX->Save();
			void PostThemeChange();
			PostThemeChange();
			return;
		}
	//		fr.Navigate(winrt::xaml_typename<winrt::VisualDML::Network>());
		/*		if (n == L"ViewAudio")
			fr.Navigate(winrt::xaml_typename<winrt::tsed::Audio>());
		if (n == L"ViewLinks")
			fr.Navigate(winrt::xaml_typename<winrt::tsed::Links>());
			*/
	}

	void MainWindow::OnLoad(IInspectable, IInspectable)
	{
		auto topnv = Content().as<NavigationView>();
		if (topnv)
		{
			Frame fr = topnv.FindName(L"contentFrame").as<Frame>();
			if (fr)
			{
				fr.Navigate(winrt::xaml_typename<winrt::ExamAI::MainPage>());
				fr.Content().as<winrt::ExamAI::MainPage>().wnd(wnd());
			}
			/*
			topnv.KeyDown([this](IInspectable const&, Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& a)
				{
					auto topnv = Content().as<NavigationView>();
					if (topnv)
					{
						Frame fr = topnv.FindName(L"contentFrame").as<Frame>();
						if (fr)
						{
							auto co = fr.Content().try_as<winrt::VisualDML::MLGraph>();
							if (co)
							{
								co.Key((long long)a.Key(), false);
							}
						}
					}
				});
				*/

		}
	}

}
