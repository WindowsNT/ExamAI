#include "pch.h"
#include "MainPage.xaml.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

#include "Item.h"
#include "Item.g.h"

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
std::vector<TRAIN_CLASS> Inference(std::vector<std::wstring> files, unsigned int B = 1, unsigned int wi = 64, unsigned int he = 64);

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.
std::vector<TRAIN_CLASS> AnswerTest(const wchar_t* rf);

namespace winrt::ExamAI::implementation
{
    void MainPage::Refresh(const wchar_t* s)
    {
        m_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ s });
    }

    void MainPage::OnInference(IInspectable, IInspectable)
    {
//        Inference({ LR"(F:\WUITOOLS\ExamAI\python\synthetic_marks_drawn\0_empty\0_empty_0133.png)",LR"(F:\WUITOOLS\ExamAI\python\synthetic_marks_drawn\1_tick\1_tick_0213.png)",LR"(F:\WUITOOLS\ExamAI\python\synthetic_marks_drawn\2_cross\2_cross_0013.png)",LR"(F:\WUITOOLS\ExamAI\python\synthetic_marks_drawn\3_dot\3_dot_0113.png)" }, 1, 64, 64);
    }


    winrt::Windows::Foundation::Collections::IObservableVector<winrt::ExamAI::Item> MainPage::li_Questions()
    {
        auto items = single_threaded_observable_vector<winrt::ExamAI::Item>();
        for (auto& q : exam.questions)
        {
            winrt::ExamAI::Item it;
            it.Name1(q.q);
            std::wstring as;
			for (auto& a : q.a)
			{
				if (!as.empty())
					as += L",";
                as += a.a;
			}
            it.Name2(as);
			auto n = new LI_CONTEXT();
			n->q = &q;
			n->e = &exam;
			it.Source((long long)n);
            items.Append(it);
        }
        return items;

    }


    void MainPage::Loaded(IInspectable const&, IInspectable const&)
    {
		if (__argc > 1)
		{
			std::wstring s = __wargv[1];
            XML3::XML x(s.c_str());
            fil = s;
            XML3::XMLElement e = x.GetRootElement();
            exam.Unser(e);
            Refresh();
        }
    }
    void MainPage::OnAnswer2(const wchar_t* jpg)
    {
        auto results = AnswerTest(jpg);
        // must be same as tests
        size_t MustCountBoxes = 0;
        for (auto& e : exam.questions)
        {
            MustCountBoxes += e.a.size();
        }

        int Corrects = 0;
        if (MustCountBoxes == results.size())
        {
            auto j = 0;
            for (auto& e : exam.questions)
            {
                e.Grade = 0;
                for (size_t c = 0; c < e.a.size(); c++)
                {
                    auto res = results[j];
                    j++;
                    if (res == TRAIN_CLASS::EMPTY)
                        continue;
                    if (e.a[c].Correct == 1)
                    {
                        e.Grade = 1;
                        Corrects++;
                    }
                }
            }
        }
        std::wstring ll = L"Correct answers: ";
        ll += std::to_wstring(Corrects);
        ll += L" of ";
        ll += std::to_wstring(exam.questions.size());
        ll += L" questions";
        MessageBoxW(0, ll.c_str(), L"ExamAI", MB_OK);
    }

    void MainPage::OnAnswer(IInspectable, IInspectable)
    {

        OPENFILENAME of = { 0 };
        of.lStructSize = sizeof(of);
        of.hwndOwner = (HWND)0;
        of.lpstrFilter = L"*.jpg\0*.jpg\0\0";
        std::vector<wchar_t> fnx(10000);
        of.lpstrFile = fnx.data();
        of.nMaxFile = 10000;
        of.lpstrDefExt = L"jpg";
        of.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if (!GetOpenFileName(&of))
            return;

		OnAnswer2(of.lpstrFile);
    }

    void MainPage::OnScan(IInspectable const&, IInspectable const&)
    {
        std::optional<std::vector<std::wstring>> TwFile(HWND hh, const wchar_t* path);
        HWND hw = 0;
        for (auto& wi : windows)
        {
            if (wi.second == *this)
            {
				hw = wi.first;
				break;
            }
        }


        auto f = TwFile(hw, datafolder.c_str());
        if (!f.has_value())
            return;
		for (auto& fi : *f)
		{
            OnAnswer2(fi.c_str());
            DeleteFile(fi.c_str());
		}
    }


    void MainPage::OnGenerateDataset(IInspectable, IInspectable)
    {
        HRESULT InstallPython();
        auto fo = PythonFolder();
        if (FAILED(InstallPython()))
            return;

		std::wstring ff = fo + L"\\python.exe";
		std::wstring ff2 = fo + L"\\examai_gensamples.py";
		auto ve = ExtractResource(GetModuleHandle(0), L"PY1", L"DATA");

		PutFile(ff2.c_str(), ve,true);   

		std::wstring cmd = ff + L" " + ff2;
        cmd += L" ";
        cmd += datafolder;
        cmd += L"\\dataset";
        PushPopDir ppd(fo.c_str());
        Run(cmd.c_str(), true, CREATE_NEW_CONSOLE);
        std::wstring d = datafolder + L"\\dataset";
        Locate(d.c_str());
    }

    void MainPage::OnTrain(IInspectable, IInspectable)
    {
//        void Train(unsigned int B = 1, unsigned int wi = 64, unsigned int he = 64, int EPOCHS = 40);
//		Train(1, 64, 64, 10);
    }


    void MainPage::OnLV1RightTapped(IInspectable const&, IInspectable const& a)
    {
        auto top = Content().as<Panel>();
        auto lv = top.FindName(L"xListView").as<winrt::Microsoft::UI::Xaml::Controls::ListView>();
        auto rt = a.as<winrt::Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs>();
    }



    void MainPage::OnLV2RightTapped(IInspectable const&, IInspectable const& a)
    {
        auto top = Content().as<Panel>();
        auto lv = top.FindName(L"xListView2").as<winrt::Microsoft::UI::Xaml::Controls::ListView>();
        auto rt = a.as<winrt::Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs>();
    }


    void MainPage::OnAddQ(IInspectable const&, IInspectable const&)
    {
        QUESTION q;
		q.q = L"Question";
        Push();
        exam.questions.push_back(q);
        Refresh();
    }

    void MainPage::OnAddA(IInspectable const&, IInspectable const&)
    {
        Push();
        auto top = Content().as<Panel>();
        auto lv = top.FindName(L"xListView").as<winrt::Microsoft::UI::Xaml::Controls::ListView>();
        auto items = lv.Items();
        for (unsigned int i = 0 ; i < items.Size() ; i++)
        {
            auto it2 = items.GetAt(i);
            auto iti = it2.as<winrt::ExamAI::Item>();
            auto it = lv.ContainerFromItem(it2).as<winrt::Microsoft::UI::Xaml::Controls::ListViewItem>();
            if (it.IsSelected())
            {
                // Find the Q
                auto n = (LI_CONTEXT*)iti.Source();
                ANSWER ans;
				ans.a = L"Answer";
				n->q->a.push_back(ans);
            }
        }
        Refresh();
    }


    void MainPage::OnSelectAllA(IInspectable const&, IInspectable const&)
    {

        auto top = Content().as<Panel>();
        auto lv = top.FindName(L"xListView").as<winrt::Microsoft::UI::Xaml::Controls::ListView>();
        auto items = lv.Items();
        for (auto it2 : items)
        {
            auto iti = it2.as<winrt::ExamAI::Item>();
            auto it = lv.ContainerFromItem(it2).as<winrt::Microsoft::UI::Xaml::Controls::ListViewItem>();
            if (it.IsSelected())
            {
                auto ctr = it.ContentTemplateRoot();
                auto dt = ctr.as<Panel>();
                auto lv2 = dt.FindName(L"xListView2").as<winrt::Microsoft::UI::Xaml::Controls::ListView>();
                it.IsSelected(false);
                if (lv2)
                {
                    auto items2 = lv2.Items();
                    for (auto it3 : items2)
                    {
                        auto it4 = lv2.ContainerFromItem(it3).as<winrt::Microsoft::UI::Xaml::Controls::ListViewItem>();
                        it4.IsSelected(true);
                    }
                }

            }
        }
    }


    void MainPage::OnUndo(IInspectable const&, IInspectable const&)
    {
		if (undos.size() == 0)
			return;
		redos.push(exam);
		exam = undos.top();
		undos.pop();
		Refresh();
    }
    void MainPage::OnRedo(IInspectable const&, IInspectable const&)
    {
		if (redos.size() == 0)
			return;
		undos.push(exam);
		exam = redos.top();
		redos.pop();
		Refresh();
    }

    void MainPage::OnMark(int M)
    {
        bool X = 0;
        auto top = Content().as<Panel>();
        auto lv = top.FindName(L"xListView").as<winrt::Microsoft::UI::Xaml::Controls::ListView>();
        auto items = lv.Items();
        for (auto it2 : items)
        {
            auto iti = it2.as<winrt::ExamAI::Item>();
            auto it = lv.ContainerFromItem(it2).as<winrt::Microsoft::UI::Xaml::Controls::ListViewItem>();
            auto ctr = it.ContentTemplateRoot();
            auto dt = ctr.as<Panel>();
            auto lv2 = dt.FindName(L"xListView2").as<winrt::Microsoft::UI::Xaml::Controls::ListView>();
            if (lv2)
            {
                auto n = (LI_CONTEXT*)iti.Source();
                auto items2 = lv2.Items();
                for (auto it3 : items2)
                {
                    auto it4 = lv2.ContainerFromItem(it3).as<winrt::Microsoft::UI::Xaml::Controls::ListViewItem>();
                    if (it4.IsSelected())
                    {
                        auto it3i = it3.as<winrt::ExamAI::Item>();
                        auto n2 = (LI_CONTEXT*)it3i.Source();
                        auto q = n->q;
                        for (size_t yy = 0; yy < exam.questions.size(); yy++)
                        {
                            if (q == &exam.questions[yy])
                            {
                                auto a = n2->a;
                                for (size_t yyy = 0; yyy < q->a.size(); yyy++)
                                {
                                    if (a == &q->a[yyy])
                                    {
                                        if (!X)
                                            Push();
                                        X = 1;
                                        q->a[yyy].Correct = M;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        Refresh();
    }
    void MainPage::OnMarkC(IInspectable const&, IInspectable const&)
    {
        OnMark(1);
    }

	void MainPage::OnMarkI(IInspectable const&, IInspectable const&)
	{
        OnMark(0);
	}

    void MainPage::OnSelectAll(IInspectable const&, IInspectable const&)
    {
        auto top = Content().as<Panel>();
        auto lv = top.FindName(L"xListView").as<winrt::Microsoft::UI::Xaml::Controls::ListView>();
        auto items = lv.Items();
        for (auto it2 : items)
        {
            auto it = lv.ContainerFromItem(it2).as<winrt::Microsoft::UI::Xaml::Controls::ListViewItem>();
            it.IsSelected(true);
        }
    }

    void MainPage::OnDeleteQ(IInspectable const&, IInspectable const&)
    {
        auto top = Content().as<Panel>();
		auto lv = top.FindName(L"xListView").as<winrt::Microsoft::UI::Xaml::Controls::ListView>();
        auto items = lv.Items();
		if (items.Size() == 0)
			return;
        bool X = 0;
        for (int i = items.Size() - 1 ; i >= 0 ; i--)
        {
            auto it2 = items.GetAt((int)i).as<winrt::ExamAI::Item>();
			auto it = lv.ContainerFromItem(it2).as<winrt::Microsoft::UI::Xaml::Controls::ListViewItem>();
            if (it.IsSelected())
            {
                if (!X)
                    Push();
                X = 1;
                // Get an Item
                auto n = (LI_CONTEXT*)it2.Source();
                auto q = n->q;
				for (size_t yy =  0 ; yy < exam.questions.size() ; yy++)
				{
					if (q == &exam.questions[yy])
					{
						exam.questions.erase(exam.questions.begin() + yy);
						break;
					}
				}
            }
        }

        if (!X)
        {
            // Check subs
            for (auto it2 : items)
            {
                auto iti = it2.as<winrt::ExamAI::Item>();
                auto it = lv.ContainerFromItem(it2).as<winrt::Microsoft::UI::Xaml::Controls::ListViewItem>();
                auto ctr = it.ContentTemplateRoot();
                auto dt = ctr.as<Panel>();
                auto lv2 = dt.FindName(L"xListView2").as<winrt::Microsoft::UI::Xaml::Controls::ListView>();
                if (lv2)
                {
                    auto n = (LI_CONTEXT*)iti.Source();
                    auto items2 = lv2.Items();
                    for (auto it3 : items2)
                    {
                        auto it4 = lv2.ContainerFromItem(it3).as<winrt::Microsoft::UI::Xaml::Controls::ListViewItem>();
                        if (it4.IsSelected())
                        {
                            auto it3i = it3.as<winrt::ExamAI::Item>();
                            auto n2 = (LI_CONTEXT*)it3i.Source();
                            auto q = n->q;
                            for (size_t yy = 0; yy < exam.questions.size(); yy++)
                            {
                                if (q == &exam.questions[yy])
                                {
                                    auto a = n2->a;
                                    for (size_t yyy = 0; yyy < q->a.size(); yyy++)
                                    {
                                        if (a == &q->a[yyy])
                                        {
                                            if (!X)
                                                Push();
                                            X = 1;
                                            q->a.erase(q->a.begin() + yyy);
                                            break;
                                        }
                                    }
                                }
                            }

                        }
                    }
                }
            }
        }
        Refresh();
    }


    void MainPage::OnValidatePython(IInspectable, IInspectable)
    {
        HRESULT InstallPython();
        auto fo = PythonFolder();
        if (FAILED(InstallPython()))
            return;

        std::wstring ff = fo + L"\\python.exe";
        std::wstring ff2 = fo + L"\\examai_validate.py";
        auto ve = ExtractResource(GetModuleHandle(0), L"PY3", L"DATA");
        PutFile(ff2.c_str(), ve, true);

        std::wstring cmd = ff + L" " + ff2;
        cmd += L" ";
        cmd += datafolder;
        cmd += L"\\ticknet.onnx";
        cmd += L" ";
        cmd += datafolder;
        cmd += L"\\val_dataset.pt";
        PushPopDir ppd(fo.c_str());
        Run(cmd.c_str(), true, CREATE_NEW_CONSOLE);

    }

    void MainPage::OnTrainPython(IInspectable, IInspectable)
    {
        HRESULT InstallPython();
        auto fo = PythonFolder();
        if (FAILED(InstallPython()))
            return;

        std::wstring ff = fo + L"\\python.exe";
        std::wstring ff2 = fo + L"\\examai_train.py";
        auto ve = ExtractResource(GetModuleHandle(0), L"PY2", L"DATA");
        PutFile(ff2.c_str(), ve, true);

        std::wstring cmd = ff + L" " + ff2;
        cmd += L" ";
        cmd += datafolder;
        cmd += L"\\dataset";
        cmd += L" ";
        cmd += datafolder;
        PushPopDir ppd(fo.c_str());
        Run(cmd.c_str(), true, CREATE_NEW_CONSOLE);
        std::wstring d = datafolder + L"\\dataset";
        Locate(d.c_str());
    }

    void MainPage::OnNew(IInspectable const&, IInspectable const&)
    {
        winrt::ExamAI::MainWindow CreateWi();
        CreateWi();
        void PostThemeChange();
        PostThemeChange();

    }
    void MainPage::OnOpen(IInspectable const&, IInspectable const&)
    {

        OPENFILENAME of = { 0 };
        of.lStructSize = sizeof(of);
        of.hwndOwner = (HWND)0;
        of.lpstrFilter = L"*.examai\0*.examai\0\0";
        std::vector<wchar_t> fnx(10000);
        of.lpstrFile = fnx.data();
        of.nMaxFile = 10000;
        of.lpstrDefExt = L"examai";
        of.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if (!GetOpenFileName(&of))
            return;

        Push();
		XML3::XML x(of.lpstrFile);
		XML3::XMLElement e = x.GetRootElement();
		exam.Unser(e);
        Refresh();
    }
    void MainPage::OnSave(IInspectable const& c1, IInspectable const& c2)
    {
        if (exam.questions.size() == 0)
        {
			OnSaveAs(c1, c2);
            return;
        }

        DeleteFile(fil.c_str());
        XML3::XML x(fil.c_str());
        exam.Ser(x.GetRootElement());
        x.Save();

    }

    void MainPage::OnPrintAll(IInspectable const&, IInspectable const&)
    {
        std::wstring TempFile3();
        std::wstring s = TempFile3();
        s += L".html";
        exam.Gen(s.c_str(),true);
        ShellExecute(0, L"open", s.c_str(), 0, 0, SW_SHOWMAXIMIZED);
    }

    void MainPage::OnPrint(IInspectable const&, IInspectable const&)
    {
        std::wstring TempFile3();
        std::wstring s = TempFile3();
        s += L".html";
		exam.Gen(s.c_str());
		ShellExecute(0,L"open",s.c_str(),0,0,SW_SHOWMAXIMIZED);
    }

    void MainPage::OnExit(IInspectable const&, IInspectable const&)
    {
        PostMessage((HWND)wnd(), WM_CLOSE, 0, 0);
    }
    void MainPage::OnSaveAs(IInspectable const&, IInspectable const&)
    {
        OPENFILENAME of = { 0 };
        of.lStructSize = sizeof(of);
        of.hwndOwner = (HWND)0;
        of.lpstrFilter = L"*.examai\0*.examai\0\0";
        std::vector<wchar_t> fnx(10000);
        of.lpstrFile = fnx.data();
        of.nMaxFile = 10000;
        of.lpstrDefExt = L"examai";
        of.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
        if (!GetSaveFileName(&of))
            return;

        fil = of.lpstrFile;
		DeleteFile(of.lpstrFile);
		XML3::XML x(of.lpstrFile);
        exam.Ser(x.GetRootElement());
		x.Save();
    }

}
