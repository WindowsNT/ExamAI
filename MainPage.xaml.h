#pragma once

#include "MainPage.g.h"
#include "Item.h"
#include "Item.g.h"


#include "project.hpp"

namespace winrt::ExamAI::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        EXAM exam;
        std::wstring fil;
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

        std::string qr_listen_final;
        std::wstring qr_listen_final2;

        winrt::hstring SvgSource2();
        
        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
        void Refresh(std::vector<std::wstring> strs);
        void Refresh(const wchar_t* s = L"");
        void UploadThread();
        void UploadThread2(SOCKET y);
        void Loaded(IInspectable const&, IInspectable const&);
        void OnNew(IInspectable const&, IInspectable const&);
        void OnOpen(IInspectable const&, IInspectable const&);
        void OnScan(IInspectable const&, IInspectable const&);
        void OnUpload(IInspectable const&, IInspectable const&);
        void OnSave(IInspectable const&, IInspectable const&);
        void OnPrint(IInspectable const&, IInspectable const&);
        void OnPrintAll(IInspectable const&, IInspectable const&);
        void OnExit(IInspectable const&, IInspectable const&);
        void OnSaveAs(IInspectable const&, IInspectable const&);
        void OnGenerateDataset(IInspectable, IInspectable);
        void OnTrain(IInspectable, IInspectable);
        void OnTrainPython(IInspectable, IInspectable);
        void OnValidatePython(IInspectable, IInspectable);
        void OnInference(IInspectable, IInspectable);
        void OnAnswer(IInspectable, IInspectable);
        void OnAnswer2(const wchar_t* jpg);
        void OnAddQ(IInspectable const&, IInspectable const&);
        void OnAddA(IInspectable const&, IInspectable const&);
        void OnUndo(IInspectable const&, IInspectable const&);
        void OnRedo(IInspectable const&, IInspectable const&);
        void OnLV1RightTapped(IInspectable const&, IInspectable const& a);
        void OnLV2RightTapped(IInspectable const&, IInspectable const& a);
        void OnSelectAll(IInspectable const&, IInspectable const&);
        void OnMark(int M);
        void OnMarkC(IInspectable const&, IInspectable const&);
        void OnMarkI(IInspectable const&, IInspectable const&);
        void OnSelectAllA(IInspectable const&, IInspectable const&);
        void OnDeleteQ(IInspectable const&, IInspectable const&);
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::ExamAI::Item> li_Questions();



    };
}

namespace winrt::ExamAI::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
