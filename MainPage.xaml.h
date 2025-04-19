#pragma once

#include "MainPage.g.h"
#include "Item.h"
#include "Item.g.h"




struct QUESTION
{
    std::wstring q;
    std::vector<std::wstring> a;
    int Correct = 0;
    int Grade = 0;

    void Ser(XML3::XMLElement& e)
    {
        e.vv("q").SetWideValue(q.c_str());
        auto& ans = e["answers"];
        for (auto& aa : a)
        {
            auto& ee = ans.AddElement("a");
            ee.vv("a").SetWideValue(aa.c_str());
        }
        e.vv("c").SetValueInt(Correct);
    }

    void Unser(XML3::XMLElement& e)
    {
		q = e.vv("q").GetWideValue();
		auto& ans = e["answers"];
		a.clear();
		for (auto& aa : ans)
		{
			a.push_back(aa.vv("a").GetWideValue());
		}
		Correct = e.vv("c").GetValueInt(0);
    }


};


struct EXAM
{
    std::vector<QUESTION> questions;

    void Ser(XML3::XMLElement& e)
    {
		auto& q = e["questions"];
		for (auto& qq : questions)
		{
			auto& ee = q.AddElement("q");
			qq.Ser(ee);
		}
    }

    void Unser(XML3::XMLElement& e)
    {
        auto& q = e["questions"];
        questions.clear();
        for (auto& qq : q)
        {
            QUESTION qu;
            qu.Unser(qq);
            questions.push_back(qu);
        }
    }

    void Gen(const wchar_t* html_out)
    {
        std::wstring s;
        
        s += L"<html><head><title>Exam</title></head><body>";


        s += LR"(
<meta charset="utf-8">
   <meta name="viewport" content="width=device-width, initial-scale=1">
   <script src="https://cdnjs.cloudflare.com/ajax/libs/jquery/3.7.1/jquery.min.js" integrity="sha512-v2CJ7UaYy4JwqLDIrZUI/4hqeoQieOmAZNXBeQyjo21dadnwR+8ZaIJVT8EE2iyI61OV8e6M8PP2/4hpQINQ/g==" crossorigin="anonymous" referrerpolicy="no-referrer"></script>
   <script src="https://xemantic.github.io/shader-web-background/dist/shader-web-background.min.js"></script>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/jquery.blockUI/2.70/jquery.blockUI.min.js"></script>
    <link rel="preconnect" href="https://fonts.googleapis.com">
   <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
   <link href="https://fonts.googleapis.com/css2?family=Noto+Serif:ital,wght@0,100..900;1,100..900&display=swap" rel="stylesheet">
   <link rel="stylesheet" href="https://unpkg.com/flickity@2/dist/flickity.min.css">
   <script src="https://unpkg.com/flickity@2/dist/flickity.pkgd.min.js"></script>
   <script src="https://unpkg.com/masonry-layout@4/dist/masonry.pkgd.min.js"></script>
   <link href="https://cdn.datatables.net/v/bm/jszip-3.10.1/dt-2.0.8/af-2.7.0/b-3.0.2/b-colvis-3.0.2/b-html5-3.0.2/b-print-3.0.2/date-1.5.2/fc-5.0.1/fh-4.0.1/kt-2.12.1/r-3.0.2/sc-2.4.3/sb-1.7.1/sp-2.3.1/sl-2.0.3/sr-1.4.1/datatables.min.css" rel="stylesheet">
   <script src="https://cdn.datatables.net/v/bm/jszip-3.10.1/dt-2.0.8/af-2.7.0/b-3.0.2/b-colvis-3.0.2/b-html5-3.0.2/b-print-3.0.2/date-1.5.2/fc-5.0.1/fh-4.0.1/kt-2.12.1/r-3.0.2/sc-2.4.3/sb-1.7.1/sp-2.3.1/sl-2.0.3/sr-1.4.1/datatables.min.js"></script>
   <script src="https://unpkg.com/@lottiefiles/lottie-player@latest/dist/lottie-player.js"></script>
   <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
   <link href="https://cdnjs.cloudflare.com/ajax/libs/chosen/1.8.7/chosen.css" rel="stylesheet">
   <script type="text/javascript" src="https://cdnjs.cloudflare.com/ajax/libs/chosen/1.8.7/chosen.jquery.js"></script>
   <link href="https://cdnjs.cloudflare.com/ajax/libs/summernote/0.9.1/summernote-bs5.css" rel="stylesheet"><script src="https://cdnjs.cloudflare.com/ajax/libs/summernote/0.9.1/summernote-bs5.js"></script>
)";

        s += LR"(<style>
    td {text-align: center; vertical-align: middle;}

    .box {
      width: 30px;
      height: 30px;
      border: 2px solid black;
      margin: 6px;
      display: inline-block;
      text-align: center;
      line-height: 30px;
      user-select: none;
    }

  </style>)";
		for (auto& q : questions)
		{
			s += L"<div>";
			s += q.q;
			s += L"<br><br>";
            s += LR"(<table class="table"><tbody><tr>)";
			for (int i = 0; i < q.a.size(); i++)
			{
                // Generate tick boxes in the same line using the .box class
                s += LR"(<td><span class="box"></span></td><td> )";
                s += q.a[i];
                s += L"</td>";
			}
            s += LR"(</tr></tbody></table>)";
            s += L"</div>";
		}
		s += L"</body></html>";
		std::wofstream f(html_out);
		f << s;

    }
};

namespace winrt::ExamAI::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        EXAM exam;
        std::stack<EXAM> undos;
        std::stack<EXAM> redos;

        void Push()
        {
			undos.push(exam);
        }

        void Undo()
        {
			if (undos.size() == 0)
				return;
			redos.push(exam);
			exam = undos.top();
			undos.pop();
            Refresh();
        }

        void Redo()
        {
			if (redos.size() == 0)
				return;
			undos.push(exam);
			exam = redos.top();
			redos.pop();
            Refresh();

        }

        MainPage()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
        {
            return m_propertyChanged.add(handler);
        }
        void PropertyChanged(winrt::event_token const& token) noexcept
        {
            m_propertyChanged.remove(token);
        }
        static winrt::hstring txt(long jx)
        {
            return s(jx);
        }

		long long _wnd = 0;
        long long wnd()
        {
            return _wnd;
        }

		void wnd(long long value)
		{
			_wnd = value;
		}

        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
        void Refresh(std::vector<std::wstring> strs);
        void Refresh(const wchar_t* s = L"");
        void Loaded(IInspectable const&, IInspectable const&);
        void OnNew(IInspectable const&, IInspectable const&);
        void OnOpen(IInspectable const&, IInspectable const&);
        void OnScan(IInspectable const&, IInspectable const&);
        void OnSave(IInspectable const&, IInspectable const&);
        void OnPrint(IInspectable const&, IInspectable const&);
        void OnExit(IInspectable const&, IInspectable const&);
        void OnSaveAs(IInspectable const&, IInspectable const&);
        void OnGenerateDataset(IInspectable, IInspectable);
        void OnTrain(IInspectable, IInspectable);
        void OnTrainPython(IInspectable, IInspectable);
        void OnValidatePython(IInspectable, IInspectable);
        void OnInference(IInspectable, IInspectable);
        void OnAnswer(IInspectable, IInspectable);
        void OnAnswer2(const wchar_t* jpg);
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::ExamAI::Item> li_Questions();



    };
}

namespace winrt::ExamAI::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
