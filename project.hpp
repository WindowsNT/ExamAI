#pragma once

struct STUDENT
{
	std::wstring n;
	int Grade = 0;
	void Ser(XML3::XMLElement& e)
	{
		e.vv("n").SetValue(n.c_str());
		e.vv("g").SetValueInt(Grade);
	}
	void Unser(XML3::XMLElement& e)
	{
		n = e.vv("n").GetWideValue();
		Grade = e.vv("g").GetValueInt(0);
	}
};

struct CLASSROOM
{
    std::wstring n;
	std::vector<STUDENT> students;
	void Ser(XML3::XMLElement& e)
	{
		e.vv("n").SetValue(n.c_str());
		auto& st = e["students"];
		for (auto& s : students)
		{
			auto& ee = st.AddElement("s");
			s.Ser(ee);
		}
	}
	void Unser(XML3::XMLElement& e)
	{
		n = e.vv("n").GetWideValue();
		auto& st = e["students"];
		students.clear();
		for (auto& s : st)
		{
			STUDENT s1;
			s1.Unser(s);
			students.push_back(s1);
		}
	}
};

struct ANSWER
{
    std::wstring a;
    int Correct = 0;

    void Ser(XML3::XMLElement& e)
    {
        e.vv("a").SetWideValue(a.c_str());
        e.vv("c").SetValueInt(Correct);
    }

    void Unser(XML3::XMLElement& e)
    {
        a = e.vv("a").GetWideValue();
        Correct = e.vv("c").GetValueInt(0);
    }
};
struct QUESTION
{
    std::wstring q;
    std::vector<ANSWER> a;
    int Grade = 0;

    void Ser(XML3::XMLElement& e)
    {
        e.vv("q").SetWideValue(q.c_str());
        auto& ans = e["answers"];
        for (auto& aa : a)
        {
            auto& ee = ans.AddElement("a");
            aa.Ser(ee);
        }
    }

    void Unser(XML3::XMLElement& e)
    {
		q = e.vv("q").GetWideValue();
		auto& ans = e["answers"];
		a.clear();
		for (auto& aa : ans)
		{
			ANSWER aa1;
			aa1.Unser(aa);
			a.push_back(aa1);
		}
    }


};


struct EXAM
{
    std::vector<QUESTION> questions;
	std::vector<CLASSROOM> classrooms;
    std::wstring n;

    void Ser(XML3::XMLElement& e)
    {
		e.vv("n").SetWideValue(n.c_str());
		auto& q = e["questions"];
		for (auto& qq : questions)
		{
			auto& ee = q.AddElement("q");
			qq.Ser(ee);
		}

		auto& cl = e["classrooms"];
        for (auto& cc : classrooms)
        {
            auto& ee = cl.AddElement("c");
            cc.Ser(ee);
        }
    }

    void Unser(XML3::XMLElement& e)
    {
		n = e.vv("n").GetWideValue();
        auto& q = e["questions"];
        questions.clear();
        for (auto& qq : q)
        {
            QUESTION qu;
            qu.Unser(qq);
            questions.push_back(qu);
        }

		auto& cl = e["classrooms"];
		classrooms.clear();
		for (auto& cc : cl)
		{
			CLASSROOM qu;
			qu.Unser(cc);
			classrooms.push_back(qu);
		}
    }

    void Gen(const wchar_t* html_out,bool ForAll = 0)
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

		auto PrintQs = [&]()
			{
				for (auto& q : questions)
				{
					s += L"<div>";
					s += q.q;
					s += L"<br><br>";
					s += LR"(<table class="table"><tbody><tr>)";
					for (int i = 0; i < q.a.size(); i++)
					{
						s += LR"(<td><span class="box"></span></td><td> )";
						s += q.a[i].a;
						s += L"</td>";
					}
					s += LR"(</tr></tbody></table>)";
					s += L"</div>";
				}
			};


        if (ForAll)
        {
			wchar_t key[1000] = {};
			for (size_t cidx = 0; cidx < classrooms.size(); cidx++)
			{
				auto& c = classrooms[cidx];
				for (size_t kidx = 0 ; kidx < c.students.size() ; kidx++)
				{
					auto& st = c.students[kidx];
					s += L"<h3>";
					s += c.n + L" &mdash; " + st.n;
					s += L"</h3>";

					// Generate QR code for the student
					s += L"<img src='https://api.qrserver.com/v1/create-qr-code/?size=100x100&data=";
					swprintf_s(key, 1000, L"key_%zd_%zd", cidx + 1, kidx + 1);
					s += key;
					s += L"' alt='QR Code' />";
					s += L"<hr><br>";

					PrintQs();

					// Page change
					s += L"<div style='page-break-after: always;'></div>";
				}
			}
        }
		else
		{
			s += L"<h3>";
			s += n;
			s += L"</h3><hr><br>";
			PrintQs();
		}


		s += L"</body></html>";
		DeleteFile(html_out);
		std::wofstream f(html_out,std::ios::binary);
		f << ystring(s).a_str();

    }
};

struct LI_CONTEXT
{
    EXAM* e = 0;
    QUESTION* q = 0;
    ANSWER* a = 0;
};
