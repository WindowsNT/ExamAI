#pragma once
#include "Item.g.h"
#include "project.hpp"

extern std::map<HWND, winrt::Windows::Foundation::IInspectable> windows;

namespace winrt::ExamAI::implementation
{
    struct Item : ItemT<Item>
    {
        Item()
        {

        }
        std::wstring _n1;
        std::wstring _n2;
        hstring Name1()
        {
            return _n1.c_str();
        }
        hstring Name2()
        {
            return _n2.c_str();
        }
         long long Source()
        {
             return (long long)lictx.get();
        }

        void Name1(hstring n)
        {
            _n1 = n.c_str();
        }

        void Name2(hstring n)
        {
            _n2 = n.c_str();
        }
        std::shared_ptr<LI_CONTEXT> lictx;
        void Source(long long n)
        {
			LI_CONTEXT* q = (LI_CONTEXT*)n;
			lictx = std::make_shared<LI_CONTEXT>();
            *lictx = *q;
            delete q;
        }

        bool Sel()
        {
            return 0;
        }
        void Sel(bool)
        {
        }

        bool _c = false;
        bool Correct()
        {
            return _c;
        }
        void Correct(bool cc)
        {
            _c = cc;
        }


        winrt::Windows::Foundation::Collections::IObservableVector<winrt::ExamAI::Item> li_Answers()
        {
            auto items = single_threaded_observable_vector<winrt::ExamAI::Item>();
            if (lictx)
            {
				for (int i = 0 ; i < lictx->q->a.size() ; i++)
				{
                    auto& a = lictx->q->a[i];
                    winrt::ExamAI::Item item;
                    item.Name1(a.a);
					item.Name2(L"");
                    

                    auto n1 = new LI_CONTEXT();
                    n1->q = lictx->q;
                    n1->e = lictx->e;
                    n1->a = &a;
                    item.Source((long long)n1);

                    // Red if correct
                    if (a.Correct == 1)
                        item.Correct(true);
                    


					items.Append(item);
				}
            }
            return items;
        }


        winrt::Microsoft::UI::Xaml::Media::Brush ColorX()
        {
/*
            auto resources = winrt::Microsoft::UI::Xaml::Application::Current().Resources();
            // This is the default foreground brush for primary text
            auto value = resources.Lookup(box_value(L"TextFillColorPrimaryBrush"));
			return value.as<winrt::Microsoft::UI::Xaml::Media::SolidColorBrush>();
*/

/*            int Theme = SettingsX->GetRootElement().vv("Theme").GetValueInt();
            bool is_light_theme();
            if (Theme == 0)
            {
                if (is_light_theme())
                    Theme = 1;
                else
                    Theme = 2;
            }

            winrt::Microsoft::UI::Xaml::Controls::Button dummy;

			dummy.RequestedTheme(
				Theme == 1 ? winrt::Microsoft::UI::Xaml::ElementTheme::Light :
				Theme == 2 ? winrt::Microsoft::UI::Xaml::ElementTheme::Dark :
				winrt::Microsoft::UI::Xaml::ElementTheme::Default);

            // Ensure template is applied (forces theme to take effect)
//            dummy.ApplyTemplate();

            // Try to find the brush in this theme context
         //   auto darkBrush = GetBrushForTheme(L"TextFillColorPrimaryBrush", winrt::Microsoft::UI::Xaml::ElementTheme::Dark);
         //   auto lightBrush = GetBrushForTheme(L"TextFillColorPrimaryBrush", winrt::Microsoft::UI::Xaml::ElementTheme::Light);
          //  auto errorBrushDark = GetBrushForTheme(L"ErrorTextFillColorBrush", winrt::Microsoft::UI::Xaml::ElementTheme::Dark);

//            auto resource = dummy.Resources().Lookup(box_value(L"TextFillColorPrimaryBrush"));
//            auto resource = dummy.Resources().Lookup(box_value(L"AccentTextFillColorPrimaryBrush"));
            auto resource = dummy.Resources().Lookup(box_value(L"ErrorTextFillColorBrush"));
            return resource.try_as<winrt::Microsoft::UI::Xaml::Media::Brush>();

*/


            auto col = winrt::Windows::UI::Color();
            col.A = 255;
            col.R = 255;
            col.G = 0;
            col.B = 0;
            if (_c)
            {
                col.R = 0;
                col.G = 0;
                col.B = 255;
            }
            auto sb = winrt::Microsoft::UI::Xaml::Media::SolidColorBrush();
            sb.Color(col);
            return sb;
        }

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
        {
            return m_propertyChanged.add(handler);
        }
        void PropertyChanged(winrt::event_token const& token) noexcept
        {
            m_propertyChanged.remove(token);
        }
        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;


    };
}
namespace winrt::ExamAI::factory_implementation
{
    struct Item : ItemT<Item, implementation::Item>
    {
    };
}
