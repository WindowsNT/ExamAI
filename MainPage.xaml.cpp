#include "pch.h"
#include "MainPage.xaml.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

#include "Item.h"
#include "Item.g.h"
std::wstring TempFile3();

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
std::vector<TRAIN_CLASS> Inference(std::vector<std::wstring> files, unsigned int B = 1, unsigned int wi = 64, unsigned int he = 64);

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.
std::vector<TRAIN_CLASS> AnswerTest(const wchar_t* rf);

class FMAP
{
private:

    HANDLE hX = INVALID_HANDLE_VALUE;
    HANDLE hY = INVALID_HANDLE_VALUE;
    unsigned long long sz = 0;
    char* d = 0;
    std::wstring mappedfile;

public:

    std::wstring fil(bool WithFolder) const
    {
        if (WithFolder)
            return mappedfile;
        std::vector<wchar_t> t(1000);
        wcscpy_s(t.data(), 1000, mappedfile.c_str());
        PathStripPath(t.data());
        return t.data();
    }
    void Stop(unsigned long long p)
    {
        LARGE_INTEGER li = { 0 };
        GetFileSizeEx(hX, &li);
        if (p > (unsigned long long)li.QuadPart)
            p = li.QuadPart;
        sz = p;

    }

    HANDLE& hF() { return hX; }
    unsigned long long size() { return sz; }
    char* p() { return d; }

    bool CreateForRecord(const wchar_t* f)
    {
        mappedfile = f;
        hX = CreateFile(f, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
        if (hX == INVALID_HANDLE_VALUE || hX == 0)
        {
            hX = 0;
            return false;
        }
        return true;
    }

    bool Map(const wchar_t* f)
    {
        hX = CreateFile(f, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
        if (hX == INVALID_HANDLE_VALUE || hX == 0)
        {
            hX = 0;
            return false;
        }
        LARGE_INTEGER li = { 0 };
        GetFileSizeEx(hX, &li);
        sz = li.QuadPart;
        hY = CreateFileMapping(hX, 0, PAGE_READONLY, 0, 0, 0);
        if (hY == 0)
            return false;
        d = (char*)MapViewOfFile(hY, FILE_MAP_READ, 0, 0, 0);
        mappedfile = f;
        return true;
    }

    bool Map2(HANDLE hX2)
    {
        LARGE_INTEGER li = { 0 };
        GetFileSizeEx(hX2, &li);
        sz = li.QuadPart;
        hY = CreateFileMapping(hX2, 0, PAGE_READONLY, 0, 0, 0);
        if (hY == 0)
            return false;
        d = (char*)MapViewOfFile(hY, FILE_MAP_READ, 0, 0, 0);
        return true;
    }

    void Unmap()
    {
        if (d)
            UnmapViewOfFile(d);
        d = 0;
        if (hY != 0)
            CloseHandle(hY);
        hY = 0;
        if (hX != 0)
            CloseHandle(hX);
        hX = 0;
        hY = 0;
    }


    FMAP(const wchar_t* f = 0)
    {
        if (f)
            Map(f);
    }
    ~FMAP()
    {
        Unmap();
    }

};




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


    void MainPage::UploadThread2(SOCKET y)
    {
        XSOCKET X(y);
        std::vector<char> b(20000);
        int a = X.receive(b.data(), 20000, false, 0);
        if (a == 0 || a == -1)
        {
            X.Close();
            return;
        }
        MIME2::CONTENT c;
        if (c.Parse(b.data(), true) == MIME2::MIMEERR::OK)
        {
            auto h = c.httphdr();
            auto s = split(h.Left(), L' ');
            if (s.size() > 1)
            {
                auto re = s[1];
                if (re.length() > 1)
                {
                    if (re == std::string("/upload2.php"))
                    {
                        // Receive all
                        auto es = a;
						auto eb = b.data();
                        std::vector<char> d4(es);
                        memcpy(d4.data(), eb, es);
                        unsigned long long ContentEstimation = 0;
                        c = {};
                        d4.resize(d4.size() + 1);
                        if (c.Parse(d4.data(), true, d4.size()) != MIME2::MIMEERR::OK)
                        {
                            X.Close();
                            return;
                        }
                        ContentEstimation = _atoi64(c.hval("Content-Length").c_str());
                        if (!ContentEstimation)
                        {
                            X.Close();
                            return;
                        }

                        auto str2 = strstr(d4.data(), "\r\n\r\n");
                        if (!str2)
                        {
                            X.Close();
                            return;
                        }

                        [[maybe_unused]] auto FullContent = ContentEstimation + (str2 - d4.data() + 4);
                        d4.resize(d4.size() - 1);

		                // Write t
                        auto tf = TempFile3();
		                auto hX = CreateFile(tf.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		                DWORD Actual = 0;
		                WriteFile(hX, d4.data(), (DWORD)d4.size(), &Actual, 0);
		                auto ts = d4.size();



		                for (;;)
		                {
			                if (ts >= FullContent)
				                break;
			                d4.resize(1024*1024);
			                int re2 = X.receive(d4.data(), 1024 * 1024);
			                if (re2 == 0 || re2 == -1)
				                break;
			                WriteFile(hX, d4.data(), re2, &Actual, 0);
			                ts += Actual;
			                if (FullContent)
			                {
			                }
		                }


		                if (FullContent != ts)
		                {
			                char* r2 = "HTTP/1.1 500 Err\r\nContent-Type: text/html\r\n\r\nUpload failed.";
			                X.transmit(r2, (int)strlen(r2), true);
			                X.Close();
			                return;
		                }

		                SetFilePointer(hX, 0, 0, FILE_BEGIN);
		                FMAP fm;
		                fm.Map2((HANDLE)hX);
		                auto sz = fm.size();
		                auto data = fm.p();


		                MIME2::CONTENT c2;
		                if (c2.Parse(data, true, sz) != MIME2::MIMEERR::OK)
		                {
			                char* r2 = "HTTP/1.1 500 Err\r\nContent-Type: text/html\r\n\r\nUpload failed.";
			                X.transmit(r2, (int)strlen(r2), true);
			                X.Close();
			                return;
		                }

		                char* r2 = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nUpload completed successfully.";
		                X.transmit(r2, (int)strlen(r2), true);
		                X.Close();

		                // Get all contents and find the "file" name
		                auto a1 = c2.hval("Content-Type", "boundary"); // a1 = the boundary
		                if (a1.empty())
			                return;

		                std::vector<MIME2::CONTENT> ContentsX;
		                MIME2::ParseMultipleContent2(c2.GetData().data(), c2.GetData().size(), a1.c_str(), ContentsX);

		                for (auto& co : ContentsX)
		                {

			                auto dat = co.GetData();
			                auto v = co.hval("Content-Disposition", "filename");
			                if (v.empty())
				                return;

                            // Extension
							std::string ext = co.hval("Content-Type", "name");
							if (ext.empty())
								ext = "jpg";

                            auto tf2 = TempFile3();
                            tf2 += L".";
                            tf2 += ystring(ext);

							PutFile(tf2.c_str(), dat, true); 
                            OnAnswer2(tf2.c_str());

		                }
                        return;
                    }
                    if (re == std::string("/upload.php"))
                    {


                        char* r = R"del(HTTP/1.1 200 OK
Content-Type: text/html; charset=utf-8
Connection: close

<!DOCTYPE HTML>
    <html>
    <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bulma@0.9.4/css/bulma.min.css">
	<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js"></script>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"> 
    <div class="content" style="margin: 20px">
	<title>ExamAI File Upload</title>
		<script>

	function updateProgress (oEvent) 
	{
		if (oEvent.lengthComputable) {
			var percentComplete = (oEvent.loaded*100) / oEvent.total;
			var prg = document.getElementById("prg");
			if (!prg)
				return;
			if (percentComplete >= 99)
				percentComplete = 100;
			prg.value = percentComplete;
			var prg2 = document.getElementById("prg2");
			if (!prg2)
				return;
			$("#prg2").text(percentComplete);
		} else {
			// Unable to compute progress information since the total size is unknown
		}
	}


function SubmitForm2(q,d,formname,divsubmit,contentdiv)
	{
	var da = document.getElementById(divsubmit);
	
	if (da)
		{
	if (d)
		da.innerHTML= "<span id=\"prg2\"></span><progress id=\"prg\" class=\"progress\"  name=\"prg\" value=\"0\" max=\"100\" style=\"width: 100%\"/>";
	else
		da.innerHTML= "";
		}
	
	var formData = new FormData(document.getElementById(formname));		
		
	var xhr = new XMLHttpRequest();
	xhr.upload.addEventListener("progress", function(evt)
		{
		updateProgress(evt);
		}
		, false);
	xhr.addEventListener("progress", function(evt)
		{
			updateProgress(evt);
     	}
		, false);
		xhr.onreadystatechange=function()
		{
			if (xhr.readyState == 4) 
			{
				if (xhr.status == 200)
				{
					document.getElementById(contentdiv).innerHTML = xhr.responseText;
				}
				else
				{
					document.getElementById(contentdiv).innerHTML= "Upload failed.";
				}
			}
		}
		xhr.open("POST", q);
		xhr.send(formData);
		
	
	
	return false;
	}

		function SubmitForm(q,d)
			   {
			   return SubmitForm2(q,d,'submitform','div-submit','div-content');
			   }
		</script>
    </head>
    <body>
<div id="div-content">
    
    ExamAI Upload.
    <hr>
	<form id="submitform" method="post" onsubmit="return SubmitForm('upload2.php',true);" enctype="application/x-www-form-urlencoded" >
	<table width="30%" border="0">
	<label for="file"></label>
        <input type="file" name="file[]" id="file" multiple accept="image/*" />
        <br/>
<br><br>
</div>
<div id="div-submit">
<button id="submitbutton">Submit</button>
</div>
        </form>
        
</div>
        </body>
        </html>
)del";

                        X.transmit(r, (int)strlen(r), true);
                        X.Close();
                    }
                }
            }
        }
    }


       void MainPage::UploadThread()
    {
		MIME2::CONTENT mime;
        XSOCKET x;
		x.Create(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

        int port = SettingsX->GetRootElement().vv("port").GetValueInt(7770);
        if (!x.BindAndListen(port))
            return;

        extern std::vector<std::string> LocalIPs;

        void ListIpAddresses(std::vector<std::string>&ipAddrs, bool C);
        ListIpAddresses(LocalIPs, 1);

        // Create the QR code image SVG
        char url[1000] = {};
        char url2[1000] = {};
		for (auto& ll : LocalIPs)
		{
            // is it ipv6 ?
            if (ll.find(':') != std::string::npos)
                continue;
            sprintf_s(url, 1000, "http://%s:%d/upload.php", LocalIPs[0].c_str(), port);
		}  

        qr_listen_final2 = ystring(url);
        // Create the SVG
        const qrcodegen::QrCode::Ecc errCorLvl = qrcodegen::QrCode::Ecc::LOW;  // Error correction level
        const qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(ystring(url).a_str(), errCorLvl);
        std::string toSvgString(const  qrcodegen::QrCode & qr, int border);
		std::string svg = toSvgString(qr, 4);

        sprintf_s(url2, 1000, "https://api.qrserver.com/v1/create-qr-code/?size=150x150&data=%s",url);
        qr_listen_final = url2;

        void PostThemeChange();
        PostThemeChange();
        for (;;)
        {
            SOCKET y = x.Accept();
            if (y == INVALID_SOCKET || y == 0 || y == -1)
                break;

			std::thread t(&MainPage::UploadThread2, this,y);
			t.detach();
        }
    }


       winrt::hstring MainPage::SvgSource2()
       {
           auto top = Content().as<Panel>();
		   HyperlinkButton hlb = top.FindName(L"hlb").as<HyperlinkButton>();
		   Image svgi = top.FindName(L"svgi").as<Image>();
           if (qr_listen_final2.length())
           {
               hlb.NavigateUri(winrt::Windows::Foundation::Uri(ystring(qr_listen_final2)));

               std::vector<char> Fetch(const char* TheLink);
               winrt::Windows::Storage::Streams::InMemoryRandomAccessStream memStream;
               auto foo = [&]()
                   {                  
                       auto fet = Fetch(qr_listen_final.c_str());

                        auto tf = TempFile3();
                        tf += L".png";
                        PutFile(tf.c_str(), fet, true);


                        // Step 1: Create stream and DataWriter
                        winrt::Windows::Storage::Streams::DataWriter writer{ memStream };

                        // Step 2: Write image bytes
                        writer.WriteBytes(winrt::array_view<const uint8_t>(
                            reinterpret_cast<const uint8_t*>(fet.data()),
                            static_cast<uint32_t>(fet.size())));

                        // Step 3: Commit and rewind
                        writer.StoreAsync().get();
                        writer.FlushAsync().get();
                        writer.DetachStream();
                        memStream.Seek(0);
                   };

               std::thread t(foo);
               t.join();
               // Step 4: Create BitmapImage and set stream source
               winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage bitmapImage;
               bitmapImage.SetSource(memStream);
               svgi.Source(bitmapImage);
           }


           return qr_listen_final2.c_str();
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

        std::thread t(&MainPage::UploadThread, this);
        t.detach();
    }
    void MainPage::OnAnswer2(const wchar_t* jpg)
    {
        bool DetectQR(const wchar_t* img, int& k1, int& k2);
        int k1 = 0, k2 = 0;
        DetectQR(jpg,k1,k2);

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
                if (k1 > 0 && k2 > 0)
                    exam.classrooms[k1 - 1].students[k2 - 1].Grade = 0;
                for (size_t c = 0; c < e.a.size(); c++)
                {
                    auto res = results[j];
                    j++;
                    if (res == TRAIN_CLASS::EMPTY)
                        continue;
                    if (e.a[c].Correct == 1)
                    {
                        e.Grade = 1;
                        if (k1 > 0 && k2 > 0)
                            exam.classrooms[k1 - 1].students[k2 - 1].Grade++;
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


    void MainPage::OnUpload(IInspectable const&, IInspectable const&)
    {

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
        std::wstring s = TempFile3();
        s += L".html";
        exam.Gen(s.c_str(),true);
        ShellExecute(0, L"open", s.c_str(), 0, 0, SW_SHOWMAXIMIZED);
    }

    void MainPage::OnPrint(IInspectable const&, IInspectable const&)
    {
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
