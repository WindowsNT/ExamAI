#include <pch.h>


const wchar_t* ttitle = L"ExamAI";
std::shared_ptr<XML3::XML> SettingsX;
std::wstring datafolder;


bool is_light_theme() {
    // based on https://stackoverflow.com/questions/51334674/how-to-detect-windows-10-light-dark-mode-in-win32-application

    // The value is expected to be a REG_DWORD, which is a signed 32-bit little-endian
    char buffer[4];
    DWORD cbData = 4;
    RegGetValueW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"AppsUseLightTheme",
        RRF_RT_REG_DWORD, // expected value type
        nullptr,
        buffer,
        &cbData);

    // convert bytes written to our buffer to an int, assuming little-endian
    auto i = int(buffer[3] << 24 |
        buffer[2] << 16 |
        buffer[1] << 8 |
        buffer[0]);

    return i == 1;
}


CComPtr<IWICImagingFactory> wbfact;

void SaveWic(IWICBitmapSource* src, const wchar_t* out)
{
    if (!wbfact)
        CoCreateInstance(CLSID_WICImagingFactory, 0, CLSCTX_INPROC_SERVER,
            __uuidof(IWICImagingFactory), (void**)&wbfact);

    CComPtr<IWICBitmapEncoder> wicBitmapEncoder;
    wbfact->CreateEncoder(GUID_ContainerFormatPng, 0, &wicBitmapEncoder);
	CComPtr<IWICStream> stream;
	wbfact->CreateStream(&stream);  

	stream->InitializeFromFilename(out, GENERIC_WRITE);
	wicBitmapEncoder->Initialize(stream, WICBitmapEncoderNoCache);
	CComPtr<IWICBitmapFrameEncode> wicFrameEncode;
	wicBitmapEncoder->CreateNewFrame(&wicFrameEncode, 0);
	wicFrameEncode->Initialize(0);
	UINT W = 0, H = 0;
	src->GetSize(&W, &H);
	wicFrameEncode->SetSize(W, H);
    GUID g2 = GUID_WICPixelFormat32bppBGRA;
	wicFrameEncode->SetPixelFormat(&g2);
	wicFrameEncode->WriteSource(src, 0);
	wicFrameEncode->Commit();
	wicBitmapEncoder->Commit();

}
CComPtr<IWICBitmap> LoadWic(const wchar_t* o)
{
	if (!wbfact)
		CoCreateInstance(CLSID_WICImagingFactory, 0, CLSCTX_INPROC_SERVER,
			__uuidof(IWICImagingFactory), (void**)&wbfact);
	CComPtr<IWICBitmapDecoder> pDecoder = NULL;
	CComPtr<IWICBitmapFrameDecode> pSource = NULL;

	auto hr = wbfact->CreateDecoderFromFilename(
		o,
		NULL, GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);
	if (FAILED(hr)) return {};
	hr = pDecoder->GetFrame(0, &pSource);
	if (FAILED(hr)) return {};

    CComPtr<IWICBitmap> bix;
    wbfact->CreateBitmapFromSource(pSource, WICBitmapCacheOnDemand, &bix);
    return bix;
}



int CALLBACK BrowseCallbackProc(
    HWND hwnd,
    UINT uMsg,
    LPARAM,
    LPARAM lpData
)
{
    if (uMsg == BFFM_INITIALIZED)
    {
        ITEMIDLIST* sel = (ITEMIDLIST*)lpData;
        if (!sel)
            return 0;
        SendMessage(hwnd, BFFM_SETSELECTION, false, (LPARAM)sel);
    }
    return 0;
}


std::wstring TempFile3()
{
    std::vector<wchar_t> td(1000);
    GetTempPathW(1000, td.data());

    wcscat_s(td.data(), 1000, L"\\");
    std::vector<wchar_t> x(1000);
    GetTempFileNameW(td.data(), L"tmp", 0, x.data());
    DeleteFile(x.data());

    return x.data();
}


bool BrowseFolder(HWND hh, const TCHAR* tit, const TCHAR* root, const TCHAR* sel, TCHAR* rv)
{
    IShellFolder* sf = 0;
    SHGetDesktopFolder(&sf);
    if (!sf)
        return false;

    BROWSEINFO bi = { 0 };
    bi.hwndOwner = hh;
    if (root && _tcslen(root))
    {
        PIDLIST_RELATIVE pd = 0;
        wchar_t dn[1000] = { 0 };
        wcscpy_s(dn, 1000, root);
        sf->ParseDisplayName(hh, 0, dn, 0, &pd, 0);
        bi.pidlRoot = pd;
    }
    bi.lpszTitle = tit;
    bi.ulFlags = BIF_EDITBOX | BIF_NEWDIALOGSTYLE;
    bi.lpfn = BrowseCallbackProc;
    if (sel && _tcslen(sel))
    {
        PIDLIST_RELATIVE pd = 0;
        wchar_t dn[1000] = { 0 };
        wcscpy_s(dn, 1000, sel);
        sf->ParseDisplayName(hh, 0, dn, 0, &pd, 0);
        bi.lParam = (LPARAM)pd;
    }

    PIDLIST_ABSOLUTE pd = SHBrowseForFolder(&bi);
    if (!pd)
        return false;

    SHGetPathFromIDList(pd, rv);
    CoTaskMemFree(pd);
    return true;
}




#include "english.hpp"


const wchar_t* s(size_t idx)
{
    if (idx > MAX_LANG)
        return L"";
    return z_strings[idx];
}



DWORD Run(const wchar_t* y, bool W, DWORD flg)
{
    PROCESS_INFORMATION pInfo = { 0 };
    STARTUPINFO sInfo = { 0 };

    sInfo.cb = sizeof(sInfo);
    wchar_t yy[1000];
    swprintf_s(yy, 1000, L"%s", y);
    CreateProcess(0, yy, 0, 0, 0, flg, 0, 0, &sInfo, &pInfo);
    SetPriorityClass(pInfo.hProcess, IDLE_PRIORITY_CLASS);
    SetThreadPriority(pInfo.hThread, THREAD_PRIORITY_IDLE);
    if (W)
        WaitForSingleObject(pInfo.hProcess, INFINITE);
    DWORD ec = 0;
    GetExitCodeProcess(pInfo.hProcess, &ec);
    CloseHandle(pInfo.hProcess);
    CloseHandle(pInfo.hThread);
    return ec;
}




void Locate([[maybe_unused]] const wchar_t* fi)
{
    wchar_t buf[1024] = {};
    swprintf_s(buf, 1024, L"/select,\"%s\"", fi);
    ShellExecute(0, L"open", L"explorer.exe", buf, 0, SW_SHOWMAXIMIZED);
}




std::optional<std::vector<std::wstring>> TwFile(HWND hh, const wchar_t* szFolder)
{
    CComPtr<IWiaDevMgr2> d;
    CoCreateInstance(CLSID_WiaDevMgr2, 0, CLSCTX_ALL, __uuidof(IWiaDevMgr2), (void**)&d);
    if (!d)
        return {};

    std::vector<std::wstring> z;

    LONG                nFiles = 0;
    LPWSTR* rFilePaths = NULL;
    CComPtr<IWiaItem2> pItem = NULL;
    //    IWiaItem2* pItemRoot = NULL;
     //   BSTR                devName = NULL;
    auto hr = d->GetImageDlg(0, NULL, hh, _bstr_t(szFolder), _bstr_t(L"scanfile.jpg"), &nFiles, &rFilePaths, &pItem);
    if (FAILED(hr))
        return {};

    for (LONG f = 0; f < nFiles; f++)
    {
        z.push_back(rFilePaths[f]);
        SysFreeString(rFilePaths[f]);
    }

    return z;
}

inline std::wstring g2s(CLSID g)
{
	wchar_t w[100] = { 0 };
	StringFromGUID2(g, w, 100);
	return w;
}
inline CLSID s2g(const wchar_t* t)
{
	CLSID g = { 0 };
	CLSIDFromString(t, &g);
	return g;
}


std::vector<std::string> LocalIPs;
struct AD_AND_IP
{
	GUID g;
	std::string ip;
};

void ListIpAddresses(std::vector<std::string>& ipAddrs, bool ConnOnly)
{
	ipAddrs.clear();
	using namespace std;
	IP_ADAPTER_ADDRESSES* adapter_addresses(NULL);
	IP_ADAPTER_ADDRESSES* adapter(NULL);


	std::vector<AD_AND_IP> adandips;

	// Start with a 16 KB buffer and resize if needed -
	// multiple attempts in case interfaces change while
	// we are in the middle of querying them.
	DWORD adapter_addresses_buffer_size = 16 * 1024;
	for (int attempts = 0; attempts != 3; ++attempts)
	{
		adapter_addresses = (IP_ADAPTER_ADDRESSES*)malloc(adapter_addresses_buffer_size);
		//		assert(adapter_addresses);

		DWORD error = GetAdaptersAddresses(
			AF_UNSPEC,
			GAA_FLAG_SKIP_ANYCAST |
			GAA_FLAG_SKIP_MULTICAST |
			GAA_FLAG_SKIP_DNS_SERVER |
			GAA_FLAG_SKIP_FRIENDLY_NAME,
			NULL,
			adapter_addresses,
			&adapter_addresses_buffer_size);

		if (ERROR_SUCCESS == error)
		{
			// We're done here, people!
			break;
		}
		else if (ERROR_BUFFER_OVERFLOW == error)
		{
			// Try again with the new size
			free(adapter_addresses);
			adapter_addresses = NULL;

			continue;
		}
		else
		{
			// Unexpected error code - log and throw
			free(adapter_addresses);
			adapter_addresses = NULL;

			// @todo
		//	LOG_AND_THROW_HERE();
		}
	}

	// Iterate through all of the adapters
	for (adapter = adapter_addresses; NULL != adapter; adapter = adapter->Next)
	{
		// Skip loopback adapters
		if (IF_TYPE_SOFTWARE_LOOPBACK == adapter->IfType)
		{
			continue;
		}

		if (ConnOnly && adapter->OperStatus != IfOperStatusUp)
			continue;

		// Parse all IPv4 and IPv6 addresses
		for (
			IP_ADAPTER_UNICAST_ADDRESS* address = adapter->FirstUnicastAddress;
			NULL != address;
			address = address->Next)
		{
			auto family = address->Address.lpSockaddr->sa_family;
			if (AF_INET == family)
			{
				// IPv4
				SOCKADDR_IN* ipv4 = reinterpret_cast<SOCKADDR_IN*>(address->Address.lpSockaddr);

				char str_buffer[INET_ADDRSTRLEN] = { 0 };
				inet_ntop(AF_INET, &(ipv4->sin_addr), str_buffer, INET_ADDRSTRLEN);

				//std::map<int, std::vector<std::wstring>> AdapterAddresses;
				if (strstr(str_buffer, "169.254.") == 0)
				{
					AD_AND_IP aai;
					aai.g = s2g(ystring(adapter->AdapterName));
					aai.ip = str_buffer;
					adandips.push_back(aai);
					ipAddrs.push_back(str_buffer);
				}
			}
			else if (AF_INET6 == family)
			{
				// IPv6
				SOCKADDR_IN6* ipv6 = reinterpret_cast<SOCKADDR_IN6*>(address->Address.lpSockaddr);

				char str_buffer[INET6_ADDRSTRLEN] = { 0 };
				inet_ntop(AF_INET6, &(ipv6->sin6_addr), str_buffer, INET6_ADDRSTRLEN);

				std::string ipv6_str(str_buffer);

				// Detect and skip non-external addresses
				bool is_link_local(false);
				bool is_special_use(false);

				if (0 == ipv6_str.find("fe"))
				{
					char c = ipv6_str[2];
					if (c == '8' || c == '9' || c == 'a' || c == 'b')
					{
						is_link_local = true;
					}
				}
				else if (0 == ipv6_str.find("2001:0:"))
				{
					is_special_use = true;
				}

				if (!(is_link_local || is_special_use))
				{
					AD_AND_IP aai;
					aai.g = s2g(ystring(adapter->AdapterName));
					aai.ip = ipv6_str;
					adandips.push_back(aai);

					ipAddrs.push_back(ipv6_str);
				}
			}
			else
			{
				// Skip all other types of addresses
				continue;
			}
		}
	}

	// Cleanup
	free(adapter_addresses);
	adapter_addresses = NULL;

	std::sort(ipAddrs.begin(), ipAddrs.end());
	ipAddrs.erase(std::unique(ipAddrs.begin(), ipAddrs.end()), ipAddrs.end());
	// Cheers!
	if (ConnOnly)
	{
		CComPtr<INetworkListManager> nlm;
		nlm.CoCreateInstance(CLSID_NetworkListManager);
		std::vector<GUID> ActivesOnly;
		if (nlm)
		{
			CComPtr<IEnumNetworks> en;
			nlm->GetNetworks(NLM_ENUM_NETWORK_ALL, &en);
			if (en)
			{
				for (;;)
				{
					ULONG fe = 0;
					CComPtr<INetwork> ne;
					en->Next(1, &ne, &fe);
					if (fe != 1)
						break;
					if (!ne)
						break;

					VARIANT_BOOL co = {};
					ne->get_IsConnected(&co);


					CComPtr<IEnumNetworkConnections> nec;
					ne->GetNetworkConnections(&nec);
					if (nec)
					{
						for (;;)
						{
							ULONG fe2 = 0;
							CComPtr<INetworkConnection> nex;
							nec->Next(1, &nex, &fe2);
							if (fe2 != 1)
								break;
							if (!nex)
								break;

							GUID adi = {};
							nex->GetAdapterId(&adi);
							ActivesOnly.push_back(adi);
						}
					}

					if (co == VARIANT_TRUE)
						continue;

				}
			}
		}

		for (auto& e : adandips)
		{
			bool A = 0;
			for (auto& aa : ActivesOnly)
			{
				if (aa == e.g)
				{
					A = 1;
					break;
				}
			}

			if (!A)
			{
				// Adapter is not active, remove the ips from the list
				for (int i = (int)ipAddrs.size() - 1; i >= 0 && !ipAddrs.empty(); i--)
				{
					if (e.ip == ipAddrs[i])
					{
						ipAddrs.erase(ipAddrs.begin() + i);
					}
				}
			}

		}
	}

}

std::shared_future<std::string> GetIP4;
std::shared_future<std::string> GetIP6;

std::vector<char> Fetch(const char* TheLink)
{
	// Create thread that will show download progress
	DWORD Size;
	unsigned long bfs = 1000;
	TCHAR ss[1000] = { 0 };
	DWORD TotalTransferred = 0;
	std::vector<char> aaaa;


	int err = 1;

	HINTERNET hI = 0, hRead = 0;
	DWORD dwTimeOut = 5000;

	hI = InternetOpen(L"RDE Debugger", INTERNET_OPEN_TYPE_DIRECT, 0, 0, 0);
	if (!hI)
		goto finish;
	InternetSetOption(hI, INTERNET_OPTION_CONNECT_TIMEOUT, (LPVOID)&dwTimeOut, 4);

	hRead = InternetOpenUrlA(hI, TheLink, 0, 0, INTERNET_FLAG_NO_CACHE_WRITE, 0);
	if (!hRead)
		goto finish;

	if (!HttpQueryInfo(hRead, HTTP_QUERY_CONTENT_LENGTH, ss, &bfs, 0))
		Size = (DWORD)-1;
	else
		Size = _ttoi(ss);

	for (;;)
	{
		DWORD n;
		std::vector <char> Buff(100010);

		memset(Buff.data(), 0, 100010);
		BOOL  F = InternetReadFile(hRead, Buff.data(), 100000, &n);
		if (F == false)
		{
			err = 2;
			break;
		}
		if (n == 0)
		{
			// End of file !
			err = 0;
			break;
		}
		TotalTransferred += n;

		//Write to File !
		//char xx = Buff[n];
		size_t olds = aaaa.size();
		aaaa.resize(olds + n);
		memcpy(aaaa.data() + olds, Buff.data(), n);

		int NewPos = 0;
		if (Size != -1)
			NewPos = (100 * TotalTransferred) / Size;
	}


finish:
	InternetCloseHandle(hRead);
	InternetCloseHandle(hI);
	return aaaa;
}

std::string GetWanIP()
{
	static std::string wip;
	if (wip.length())
		return wip;

	//	WAITINGMESSAGE wm;
	//	wm.Show(0, hIcon1, s(4665), s(4666), s(1650), false);
	auto du = Fetch("https://www.turbo-play.com/ip.php");
	if (du.size() == 0)
		return wip;
	std::vector<char> dx(du.size() + 10);
	memcpy(dx.data(), du.data(), du.size());
	wip = std::string(dx.data());
	if (wip.empty())
		return wip;
	while (wip[wip.length() - 1] == '\n' || wip[wip.length() - 1] == '\r')
		wip.erase(wip.end() - 1);
	return wip;
	//	return "ERRGETWANIP";
}

std::string GetWanIP6()
{
	//	WAITINGMESSAGE wm;
	//	wm.Show(0, hIcon1, s(4665), s(4667), s(1650), false);
	static std::string wip;
	if (wip.length())
		return wip;

	auto du = Fetch("https://www.turbo-play.com/ip6.php");
	if (du.size() == 0)
		return wip;
	std::vector<char> dx(du.size() + 10);
	memcpy(dx.data(), du.data(), du.size());
	wip = std::string(dx.data());
	if (wip.empty())
		return wip;
	while (wip[wip.length() - 1] == '\n' || wip[wip.length() - 1] == '\r')
		wip.erase(wip.end() - 1);
	return wip;
	//	return "ERRGETWANIP";
}

void GetIPs()

{
	ListIpAddresses(LocalIPs, 1);
	if (GetIP4.valid() && GetIP6.valid())
		return;
	GetIP4 = std::async([]() -> std::string
		{
			return GetWanIP();
		});
	GetIP6 = std::async([]() -> std::string
		{
			return GetWanIP6();
		});
}

std::string GetActiveIP4()
{
	auto sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	std::string r;
	sockaddr_in remoteaddr = { 0 };
	remoteaddr.sin_family = AF_INET;
	inet_pton(AF_INET, "70.32.24.194", (void*)&remoteaddr.sin_addr.s_addr);
	remoteaddr.sin_port = htons(443);

	if (connect(sock, (sockaddr*)&remoteaddr, sizeof(remoteaddr)) == 0)
	{
		sockaddr_in localaddr = { 0 };
		int len = sizeof(localaddr);

		getsockname(sock, (sockaddr*)&localaddr, &len);
		// use localaddr as needed...
		char ch2[1000] = {};
		inet_ntop(AF_INET, (void*)&localaddr.sin_addr, ch2, 1000);
		if (strlen(ch2))
			r = ch2;
	}
	closesocket(sock);
	return r;
}



#include "qrcodegen.cpp"



// Returns a string of SVG code for an image depicting the given QR Code, with the given number
// of border modules. The string always uses Unix newlines (\n), regardless of the platform.
std::string toSvgString(const  qrcodegen::QrCode& qr, int border) {
	if (border < 0)
		throw std::domain_error("Border must be non-negative");
	if (border > INT_MAX / 2 || border * 2 > INT_MAX - qr.getSize())
		throw std::overflow_error("Border too large");

	std::ostringstream sb;
	sb << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	sb << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
	sb << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 ";
	sb << (qr.getSize() + border * 2) << " " << (qr.getSize() + border * 2) << "\" stroke=\"none\">\n";
	sb << "\t<rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\"/>\n";
	sb << "\t<path d=\"";
	for (int y = 0; y < qr.getSize(); y++) {
		for (int x = 0; x < qr.getSize(); x++) {
			if (qr.getModule(x, y)) {
				if (x != 0 || y != 0)
					sb << " ";
				sb << "M" << (x + border) << "," << (y + border) << "h1v1h-1z";
			}
		}
	}
	sb << "\" fill=\"#000000\"/>\n";
	sb << "</svg>\n";
	return sb.str();
}
