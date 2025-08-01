#include "pch.h"


#ifdef _DEBUG

#pragma comment(lib,"f:\\opencv\\s\\64\\lib\\Debug\\opencv_world4100d.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\lib\\Debug\\opencv_img_hash4100d.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\lib\\Debug\\opencv_ts4100d.lib")
//#pragma comment(lib,"f:\\opencv\\s\\64\\lib\\Debug\\ade.lib")

#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Debug\\aded.lib")
//#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Debug\\IlmImfd.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Debug\\ippiwd.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Debug\\ittnotifyd.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Debug\\libjpeg-turbod.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Debug\\libopenjp2d.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Debug\\libpngd.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Debug\\libprotobufd.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Debug\\libtiffd.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Debug\\libwebpd.lib")
//#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Debug\\quircd.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Debug\\zlibd.lib")

#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\ippicv\\ippicv_win\\icv\\lib\\intel64\\ippicvmt.lib")


#else

#ifdef _WIN64

#pragma comment(lib,"f:\\opencv\\s\\64\\lib\\Release\\opencv_world4100.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\lib\\Release\\opencv_img_hash4100.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\lib\\Release\\opencv_ts4100.lib")
//#pragma comment(lib,"s4\\64\\lib\\Release\\ade.lib")

#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Release\\ade.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Release\\ippiw.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Release\\ittnotify.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Release\\libjpeg-turbo.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Release\\libopenjp2.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Release\\libpng.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Release\\libprotobuf.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Release\\libtiff.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Release\\libwebp.lib")
//#pragma comment(lib,"s4\\64\\3rdparty\\lib\\Release\\quirc.lib")
#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\lib\\Release\\zlib.lib")

#pragma comment(lib,"f:\\opencv\\s\\64\\3rdparty\\ippicv\\ippicv_win\\icv\\lib\\intel64\\ippicvmt.lib")



#else


#pragma comment(lib,"s4\\32\\lib\\Release\\opencv_world4100.lib")
#pragma comment(lib,"s4\\32\\lib\\Release\\opencv_img_hash4100.lib")
#pragma comment(lib,"s4\\32\\lib\\Release\\opencv_ts4100.lib")
//#pragma comment(lib,"s4\\32\\lib\\Release\\ade.lib")

#pragma comment(lib,"s4\\32\\3rdparty\\lib\\Release\\ade.lib")
//#pragma comment(lib,"s4\\32\\3rdparty\\lib\\Release\\IlmImf.lib")
#pragma comment(lib,"s4\\32\\3rdparty\\lib\\Release\\ippiw.lib")
#pragma comment(lib,"s4\\32\\3rdparty\\lib\\Release\\ittnotify.lib")
#pragma comment(lib,"s4\\32\\3rdparty\\lib\\Release\\libjpeg-turbo.lib")
#pragma comment(lib,"s4\\32\\3rdparty\\lib\\Release\\libopenjp2.lib")
#pragma comment(lib,"s4\\32\\3rdparty\\lib\\Release\\libpng.lib")
#pragma comment(lib,"s4\\32\\3rdparty\\lib\\Release\\libprotobuf.lib")
#pragma comment(lib,"s4\\32\\3rdparty\\lib\\Release\\libtiff.lib")
#pragma comment(lib,"s4\\32\\3rdparty\\lib\\Release\\libwebp.lib")
//#pragma comment(lib,"s4\\32\\3rdparty\\lib\\Release\\quirc.lib")
#pragma comment(lib,"s4\\32\\3rdparty\\lib\\Release\\zlib.lib")

#pragma comment(lib,"f:\\opencv\\s\\32\\3rdparty\\ippicv\\ippicv_win\\icv\\lib\\ia32\\ippicvmt.lib")


#endif

#endif

using namespace cv;



bool imwritew(const wchar_t* fw, cv::Mat& img)
{
	using namespace cv;
	std::vector<char> ext(1000);
	WideCharToMultiByte(CP_UTF8, 0, fw, (int)wcslen(fw), ext.data(), 1000, 0, 0);
	std::vector<uchar> buf;
	if (!imencode(ext.data(), img, buf))
		return false;
	FILE* fp = 0;
	_wfopen_s(&fp, fw, L"wb");
	if (!fp)
		return false;
	fwrite(buf.data(), 1, buf.size(), fp);
	fclose(fp);
	return 1;
}


cv::Mat imreadw(const wchar_t* filename, int flags = 1)
{
	using namespace cv;
	FILE* fp = 0;
	_wfopen_s(&fp, filename, L"rb");
	if (!fp)
	{
		return Mat::zeros(1, 1, CV_8U);
	}
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	char* buf = new char[sz];
	fseek(fp, 0, SEEK_SET);
	fread(buf, 1, sz, fp);
	_InputArray arr(buf, sz);
	Mat img = imdecode(arr, flags);
	delete[] buf;
	fclose(fp);
	return img;


}

bool DetectQR(const wchar_t* img,int& k1,int& k2)
{
	// Read image
	Mat inputImage;
	inputImage = imreadw(img);

	QRCodeDetector qrDecoder = QRCodeDetector::QRCodeDetector();

	Mat bbox, rectifiedImage;

	std::string data = qrDecoder.detectAndDecode(inputImage, bbox, rectifiedImage);
	if (data.length() > 0)
	{
		// Itks key_x_y
		std::vector<std::string> s = split(data, '_');
		if (s.size() == 3)
		{
			std::string x = s[1].c_str();
			std::string y = s[2].c_str();
			k1 = atoi(x.c_str());
			k2 = atoi(y.c_str());
		}
		return true;
	}
	return false;
}

std::vector<TRAIN_CLASS> AnswerTest(const wchar_t* rf)
{
	cv::Mat img = imreadw(rf, cv::IMREAD_COLOR);
	cv::Mat original = img.clone();

	// Convert to grayscale
	cv::Mat gray;
	cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

	// Blur to reduce noise
	cv::Mat blurred;
	cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);

	// Threshold
	cv::Mat thresh;
	cv::adaptiveThreshold(blurred, thresh, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
		cv::THRESH_BINARY_INV, 11, 2);

	// Deskew using Hough transform
	cv::Mat edges;
	cv::Canny(thresh, edges, 50, 150);
	std::vector<cv::Vec2f> lines;
	cv::HoughLines(edges, lines, 1, CV_PI / 180, 150);
	float angle = 0.0f;
	int count = 0;

	for (auto& l : lines)
	{
		float theta = l[1];
		if (theta > CV_PI / 4 && theta < 3 * CV_PI / 4) {
			angle += theta;
			count++;
		}
	}
	if (count > 0) {
		angle /= count;
		float degrees = (float)((angle * 180 / CV_PI) - 90.0f);
		cv::Point2f center(img.cols / 2.0F, img.rows / 2.0F);
		cv::Mat rot = cv::getRotationMatrix2D(center, degrees, 1.0);
		cv::warpAffine(img, img, rot, img.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));
		cv::warpAffine(gray, gray, rot, gray.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255));
		cv::warpAffine(thresh, thresh, rot, thresh.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0));
	}

	// Morph close to strengthen edges
	cv::Mat morph;
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::morphologyEx(thresh, morph, cv::MORPH_CLOSE, kernel);

	cv::Mat dilated;
	cv::dilate(morph, dilated, kernel);

	// Detect contours
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(dilated, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	std::vector<cv::Rect> boxes;

	for (size_t i = 0; i < contours.size(); ++i) {
		double peri = cv::arcLength(contours[i], true);
		std::vector<cv::Point> approx;
		cv::approxPolyDP(contours[i], approx, 0.02 * peri, true);

		if (approx.size() == 4 && cv::isContourConvex(approx)) {
			cv::Rect rect = cv::boundingRect(approx);
			float aspectRatio = static_cast<float>(rect.width) / rect.height;
			int area = rect.area();

			if (aspectRatio > 0.8f && aspectRatio < 1.2f && area > 500 && area < 10000)
				boxes.push_back(rect);
		}
	}

	// Sort left-to-right, top-to-bottom
	std::sort(boxes.begin(), boxes.end(), [](const cv::Rect& a, const cv::Rect& b) {
		int rowA = a.y / 10;
		int rowB = b.y / 10;
		return (rowA == rowB) ? (a.x < b.x) : (a.y < b.y);
		});

	std::vector<TRAIN_CLASS> Inference(std::vector<std::wstring> files, unsigned int B = 1, unsigned int wi = 64, unsigned int he = 64);
	std::vector<std::wstring> thefiles;

	for (size_t i = 0; i < boxes.size(); ++i) {
		cv::Mat roi = gray(boxes[i]);
		cv::Mat resized;
		cv::resize(roi, resized, cv::Size(64, 64));

		std::wstring filename = datafolder + L"\\box_" + std::to_wstring(i) + L".png";
		imwritew(filename.c_str(), resized);
		thefiles.push_back(filename);
	}

	std::vector<TRAIN_CLASS> results = Inference(thefiles, 1, 64, 64);

#ifdef _DEBUG
	for (const auto& r : boxes)
		cv::rectangle(img, r, cv::Scalar(0, 255, 0), 2);
	cv::namedWindow("Detected Boxes", cv::WINDOW_NORMAL);
	cv::imshow("Detected Boxes", img);
	cv::waitKey(0);
#endif

	return results;
}
