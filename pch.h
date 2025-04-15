#pragma once
#define DISABLE_XAML_GENERATED_MAIN
#define DML_TARGET_VERSION_USE_LATEST
#include <windows.h>
#include <unknwn.h>
#include <restrictederrorinfo.h>
#include <hstring.h>
#include <queue>
#include <any>
#include <optional>
#include <stack>
#include <mutex>
#include <vector>
#include <set>
#include <functional>
#include <memory>
#include <limits>
#include <vector>
#include <shlobj.h>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <filesystem>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <random>
#include <dwmapi.h>
#include <uxtheme.h>
#include <atlbase.h>
#include <wincodec.h>


// Undefine GetCurrentTime macro to prevent
// conflict with Storyboard::GetCurrentTime
#undef GetCurrentTime
#undef min
#undef max

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Microsoft.UI.Composition.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Input.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Microsoft.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <microsoft.ui.xaml.window.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <microsoft.ui.xaml.media.dxinterop.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.Windows.AppNotifications.h>
#include <winrt/Microsoft.Windows.AppNotifications.Builder.h>
#include <winrt/Microsoft.UI.Xaml.Navigation.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <wil/cppwinrt_helpers.h>
#include <appmodel.h>
#include <dxgi1_6.h>
#include <DirectML.h>
#include "directmlx.h"
#include "d3dx12.h"
#include "xml3all.h"

extern const wchar_t* ttitle;
extern std::shared_ptr<XML3::XML> SettingsX;
extern std::wstring datafolder;

const wchar_t* s(size_t);

#include "dmllib.hpp"
