#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib,"gdiplus")

#include <windows.h>
#include <gdiplus.h>
#include <math.h>

using namespace Gdiplus;
TCHAR szClassName[] = TEXT("Window");
#define myround(x) (int)((x)+0.5)

inline void boxesForGauss(double sigma, int *sizes, int n)
{
	double wIdeal = sqrt((12 * sigma*sigma / n) + 1);
	int wl = (int)floor(wIdeal);
	if (wl % 2 == 0) --wl;

	const double wu = wl + 2;

	const double mIdeal = (12 * sigma*sigma - n*wl*wl - 4 * n*wl - 3 * n) / (-4 * wl - 4);
	const int m = myround(mIdeal);

	for (int i = 0; i<n; ++i)
		sizes[i] = int(i<m ? wl : wu);
}

inline void boxBlurH_4(BYTE* scl, BYTE* tcl, int w, int h, int r, int bpp, int stride)
{
	float iarr = (float)(1. / (r + r + 1));
	for (int i = 0; i < h; ++i) {
		int ti1 = i*stride;
		int ti2 = i*stride + 1;
		int ti3 = i*stride + 2;
		int ti4 = i*stride + 3;

		int li1 = ti1;
		int li2 = ti2;
		int li3 = ti3;
		int li4 = ti4;

		int ri1 = ti1 + r*bpp;
		int ri2 = ti2 + r*bpp;
		int ri3 = ti3 + r*bpp;
		int ri4 = ti4 + r*bpp;

		int fv1 = scl[ti1];
		int fv2 = scl[ti2];
		int fv3 = scl[ti3];
		int fv4 = scl[ti4];

		int lv1 = scl[ti1 + (w - 1)*bpp];
		int lv2 = scl[ti2 + (w - 1)*bpp];
		int lv3 = scl[ti3 + (w - 1)*bpp];
		int lv4 = scl[ti4 + (w - 1)*bpp];

		int val1 = (r + 1)*fv1;
		int val2 = (r + 1)*fv2;
		int val3 = (r + 1)*fv3;
		int val4 = (r + 1)*fv4;

		for (int j = 0; j < r; ++j) {
			val1 += scl[ti1 + j*bpp];
			val2 += scl[ti2 + j*bpp];
			val3 += scl[ti3 + j*bpp];
			val4 += scl[ti4 + j*bpp];
		}

		for (int j = 0; j <= r; ++j) {
			val1 += scl[ri1] - fv1;
			val2 += scl[ri2] - fv2;
			val3 += scl[ri3] - fv3;
			val4 += scl[ri4] - fv4;

			tcl[ti1] = myround(val1*iarr);
			tcl[ti2] = myround(val2*iarr);
			tcl[ti3] = myround(val3*iarr);
			tcl[ti4] = myround(val4*iarr);

			ri1 += bpp;
			ri2 += bpp;
			ri3 += bpp;
			ri4 += bpp;

			ti1 += bpp;
			ti2 += bpp;
			ti3 += bpp;
			ti4 += bpp;
		}

		for (int j = r + 1; j < w - r; ++j) {
			val1 += scl[ri1] - scl[li1];
			val2 += scl[ri2] - scl[li2];
			val3 += scl[ri3] - scl[li3];
			val4 += scl[ri4] - scl[li4];

			tcl[ti1] = myround(val1*iarr);
			tcl[ti2] = myround(val2*iarr);
			tcl[ti3] = myround(val3*iarr);
			tcl[ti4] = myround(val4*iarr);

			ri1 += bpp;
			ri2 += bpp;
			ri3 += bpp;
			ri4 += bpp;

			li1 += bpp;
			li2 += bpp;
			li3 += bpp;
			li4 += bpp;

			ti1 += bpp;
			ti2 += bpp;
			ti3 += bpp;
			ti4 += bpp;
		}

		for (int j = w - r; j < w; ++j) {
			val1 += lv1 - scl[li1];
			val2 += lv2 - scl[li2];
			val3 += lv3 - scl[li3];
			val4 += lv4 - scl[li4];

			tcl[ti1] = myround(val1*iarr);
			tcl[ti2] = myround(val2*iarr);
			tcl[ti3] = myround(val3*iarr);
			tcl[ti4] = myround(val4*iarr);

			li1 += bpp;
			li2 += bpp;
			li3 += bpp;
			li4 += bpp;

			ti1 += bpp;
			ti2 += bpp;
			ti3 += bpp;
			ti4 += bpp;
		}
	}
}

inline void boxBlurT_4(BYTE* scl, BYTE* tcl, int w, int h, int r, int bpp, int stride)
{
	float iarr = (float)(1.0f / (r + r + 1.0f));
	for (int i = 0; i < w; ++i) {
		int ti1 = i*bpp;
		int ti2 = i*bpp + 1;
		int ti3 = i*bpp + 2;
		int ti4 = i*bpp + 3;

		int li1 = ti1;
		int li2 = ti2;
		int li3 = ti3;
		int li4 = ti4;

		int ri1 = ti1 + r*stride;
		int ri2 = ti2 + r*stride;
		int ri3 = ti3 + r*stride;
		int ri4 = ti4 + r*stride;

		int fv1 = scl[ti1];
		int fv2 = scl[ti2];
		int fv3 = scl[ti3];
		int fv4 = scl[ti4];

		int lv1 = scl[ti1 + stride*(h - 1)];
		int lv2 = scl[ti2 + stride*(h - 1)];
		int lv3 = scl[ti3 + stride*(h - 1)];
		int lv4 = scl[ti4 + stride*(h - 1)];

		int val1 = (r + 1)*fv1;
		int val2 = (r + 1)*fv2;
		int val3 = (r + 1)*fv3;
		int val4 = (r + 1)*fv4;

		for (int j = 0; j < r; ++j) {
			val1 += scl[ti1 + j*stride];
			val2 += scl[ti2 + j*stride];
			val3 += scl[ti3 + j*stride];
			val4 += scl[ti4 + j*stride];
		}

		for (int j = 0; j <= r; ++j) {
			val1 += scl[ri1] - fv1;
			val2 += scl[ri2] - fv2;
			val3 += scl[ri3] - fv3;
			val4 += scl[ri4] - fv4;

			tcl[ti1] = myround(val1*iarr);
			tcl[ti2] = myround(val2*iarr);
			tcl[ti3] = myround(val3*iarr);
			tcl[ti4] = myround(val4*iarr);

			ri1 += stride;
			ri2 += stride;
			ri3 += stride;
			ri4 += stride;

			ti1 += stride;
			ti2 += stride;
			ti3 += stride;
			ti4 += stride;
		}

		for (int j = r + 1; j < h - r; ++j) {
			val1 += scl[ri1] - scl[li1];
			val2 += scl[ri2] - scl[li2];
			val3 += scl[ri3] - scl[li3];
			val4 += scl[ri4] - scl[li4];

			tcl[ti1] = myround(val1*iarr);
			tcl[ti2] = myround(val2*iarr);
			tcl[ti3] = myround(val3*iarr);
			tcl[ti4] = myround(val4*iarr);

			li1 += stride;
			li2 += stride;
			li3 += stride;
			li4 += stride;

			ri1 += stride;
			ri2 += stride;
			ri3 += stride;
			ri4 += stride;

			ti1 += stride;
			ti2 += stride;
			ti3 += stride;
			ti4 += stride;
		}

		for (int j = h - r; j < h; ++j) {
			val1 += lv1 - scl[li1];
			val2 += lv2 - scl[li2];
			val3 += lv3 - scl[li3];
			val4 += lv4 - scl[li4];

			tcl[ti1] = myround(val1*iarr);
			tcl[ti2] = myround(val2*iarr);
			tcl[ti3] = myround(val3*iarr);
			tcl[ti4] = myround(val4*iarr);

			li1 += stride;
			li2 += stride;
			li3 += stride;
			li4 += stride;

			ti1 += stride;
			ti2 += stride;
			ti3 += stride;
			ti4 += stride;
		}
	}
}

inline void boxBlur_4(BYTE* scl, BYTE* tcl, int w, int h, int rx, int ry, int bpp, int stride)
{
	memcpy(tcl, scl, stride*h);
	boxBlurH_4(tcl, scl, w, h, rx, bpp, stride);
	boxBlurT_4(scl, tcl, w, h, ry, bpp, stride);
}

inline void gaussBlur_4(BYTE* scl, BYTE* tcl, int w, int h, float rx, float ry, int bpp, int stride)
{
	int bxsX[4];
	boxesForGauss(rx, bxsX, 4);

	int bxsY[4];
	boxesForGauss(ry, bxsY, 4);

	boxBlur_4(scl, tcl, w, h, (bxsX[0] - 1) / 2, (bxsY[0] - 1) / 2, bpp, stride);
	boxBlur_4(tcl, scl, w, h, (bxsX[1] - 1) / 2, (bxsY[1] - 1) / 2, bpp, stride);
	boxBlur_4(scl, tcl, w, h, (bxsX[2] - 1) / 2, (bxsY[2] - 1) / 2, bpp, stride);
	boxBlur_4(scl, tcl, w, h, (bxsX[3] - 1) / 2, (bxsY[3] - 1) / 2, bpp, stride);
}

void DoGaussianBlur(Gdiplus::Bitmap* img, float radiusX, float radiusY)
{
	if (img == 0 || (radiusX == 0.0f && radiusY == 0.0f)) return;

	const int w = img->GetWidth();
	const int h = img->GetHeight();

	if (radiusX > w / 2) {
		radiusX = (float)(w / 2);
	}

	if (radiusY > h / 2) {
		radiusY = (float)(h / 2);
	}

	Gdiplus::Bitmap* temp = new Gdiplus::Bitmap(img->GetWidth(), img->GetHeight(), img->GetPixelFormat());

	Gdiplus::BitmapData bitmapData1;
	Gdiplus::BitmapData bitmapData2;
	Gdiplus::Rect rect(0, 0, img->GetWidth(), img->GetHeight());

	if (Gdiplus::Ok == img->LockBits(
		&rect,
		Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite,
		img->GetPixelFormat(),
		&bitmapData1
	)
		&&
		Gdiplus::Ok == temp->LockBits(
			&rect,
			Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite,
			temp->GetPixelFormat(),
			&bitmapData2
		)) {
		BYTE* src = (BYTE*)bitmapData1.Scan0;
		BYTE* dst = (BYTE*)bitmapData2.Scan0;

		const int bpp = 4;
		const int stride = bitmapData1.Stride;

		gaussBlur_4(src, dst, w, h, radiusX, radiusY, bpp, stride);

		img->UnlockBits(&bitmapData1);
		temp->UnlockBits(&bitmapData2);
	}

	delete temp;
}

void DoGaussianBlurPower(Gdiplus::Bitmap* img, float radiusX, float radiusY, int nPower)
{
	Gdiplus::Bitmap* pBitmap = img->Clone(0, 0, img->GetWidth(), img->GetHeight(), PixelFormat32bppARGB);
	DoGaussianBlur(pBitmap, radiusX, radiusY);
	Gdiplus::Graphics g(pBitmap);
	for (int i = 0; i < 8; ++i) {
		g.DrawImage(pBitmap, 0, 0);
		if ((1 << i) & nPower) {
			Gdiplus::Graphics g(img);
			g.DrawImage(pBitmap, 0, 0);
		}
	}
	delete pBitmap;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hButton;
	static Bitmap * pBitmapOriginal;
	static Bitmap * pBitmapDropShadow;
	switch (msg)
	{
	case WM_CREATE:
		hButton = CreateWindow(TEXT("BUTTON"), TEXT("ドロップシャドー"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		pBitmapOriginal = new Bitmap(100, 100);
		{
			Graphics g(pBitmapOriginal);
			g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
			g.DrawString(TEXT("ABC"), -1, &Font(TEXT("メイリオ"), 32), PointF(0, 0), &SolidBrush(Color(0, 0, 0)));
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			if (pBitmapDropShadow)
			{
				Graphics g(hdc);
				g.DrawImage(pBitmapDropShadow, 0, 50);
			}
			else
			{
				Graphics g(hdc);
				g.DrawImage(pBitmapOriginal, (INT)pBitmapOriginal->GetWidth(), (INT)pBitmapOriginal->GetHeight() + 50);
			}
			EndPaint(hWnd, &ps);
		}
	case WM_SIZE:
		MoveWindow(hButton, 10, 10, 256, 32, TRUE);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			delete pBitmapDropShadow;
			pBitmapDropShadow = new Gdiplus::Bitmap((INT)pBitmapOriginal->GetWidth() * 4, (INT)pBitmapOriginal->GetHeight() * 4, PixelFormat32bppARGB);
			{
				Gdiplus::Graphics gg(pBitmapDropShadow);
				gg.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
				gg.DrawImage(pBitmapOriginal, (INT)pBitmapOriginal->GetWidth() + 2, (INT)pBitmapOriginal->GetHeight() + 2, (INT)pBitmapOriginal->GetWidth(), (INT)pBitmapOriginal->GetHeight());
				DoGaussianBlur(pBitmapDropShadow, 3.0f, 3.0f);
				gg.DrawImage(pBitmapOriginal, (INT)pBitmapOriginal->GetWidth(), (INT)pBitmapOriginal->GetHeight(), (INT)pBitmapOriginal->GetWidth(), (INT)pBitmapOriginal->GetHeight());
			}
			InvalidateRect(hWnd, 0, 1);
		}
		break;
	case WM_DESTROY:
		delete pBitmapOriginal;
		delete pBitmapDropShadow;
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	ULONG_PTR gdiToken;
	GdiplusStartupInput gdiSI;
	GdiplusStartup(&gdiToken, &gdiSI, NULL);
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("ドロップシャドー"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	GdiplusShutdown(gdiToken);
	return (int)msg.wParam;
}
