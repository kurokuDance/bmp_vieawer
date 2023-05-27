#pragma once
#include <windows.h>
#include <d2d1.h>
#include <shobjidl.h> 
#pragma comment(lib, "d2d1")
#include "basewin.h"
#include "BMP.h"

BITMAP bmi;
HBITMAP hbm;

template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

class MainWindow : public BaseWindow<MainWindow>
{

    ID2D1Factory* pFactory;
    ID2D1HwndRenderTarget* pRenderTarget;
    ID2D1SolidColorBrush* pBrush;

    char Name[100];//путь к файлу
    HRESULT CreateGraphicsResources();//созданиее области графики
    void    DiscardGraphicsResources();
    void    OnPaint();//вывод изображения на экран

public:

    MainWindow() : pFactory(NULL), pRenderTarget(NULL), pBrush(NULL)
    {
    }

    PCWSTR  ClassName() const { return L"BMP op/read"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};



HRESULT MainWindow::CreateGraphicsResources()
{
    HRESULT hr = S_OK;
    if (pRenderTarget == NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &pRenderTarget);

        if (SUCCEEDED(hr))
        {
            const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 1.0f, 0);
            hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);
        }
    }
    return hr;
}

void MainWindow::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
}

void MainWindow::OnPaint()
{
    BMP myBmp;
    bool isOneCh = false;
    int w = myBmp.getWidth();
    int h = myBmp.getHeight();

    //const char* a = "C:\\Users\\Pavel\\Pictures\\aaa.bmp";
    const char* a = Name;
    myBmp.readImgFile(a);

    if (myBmp.isBmp)
    {
        w = myBmp.getWidth();
        h = myBmp.getHeight();

        BITMAPINFO bminfo;

        R_G_B* bits = new R_G_B[w * h];

        HDC pDC = GetDC(m_hwnd);
        HDC hdcMem = CreateCompatibleDC(pDC);
        HBITMAP bm = CreateCompatibleBitmap(pDC, w, h);

        HGDIOBJ hOld = SelectObject(hdcMem, bm);

        bminfo.bmiHeader.biSize = myBmp.fileInfoHeader.biSize;
        bminfo.bmiHeader.biWidth = myBmp.fileInfoHeader.biWidth;
        bminfo.bmiHeader.biHeight = myBmp.fileInfoHeader.biHeight;
        bminfo.bmiHeader.biPlanes = myBmp.fileInfoHeader.biPlanes;
        bminfo.bmiHeader.biBitCount = 24;
        bminfo.bmiHeader.biCompression = myBmp.fileInfoHeader.biCompression;
        bminfo.bmiHeader.biSizeImage = myBmp.fileInfoHeader.biSizeImage;
        bminfo.bmiHeader.biXPelsPerMeter = myBmp.fileInfoHeader.biXPelsPerMeter;
        bminfo.bmiHeader.biYPelsPerMeter = myBmp.fileInfoHeader.biYPelsPerMeter;
        bminfo.bmiHeader.biClrUsed = myBmp.fileInfoHeader.biClrUsed;
        bminfo.bmiHeader.biClrImportant = myBmp.fileInfoHeader.biClrImportant;


        ////////////////


        R_G_B red;
        int v = 0;
        std::vector<std::vector<R_G_B>> mypix = myBmp.getPixels();
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++)
            {
                red = { mypix[i][j].b, mypix[i][j].g, mypix[i][j].r };
                bits[v] = red;
                v++;
            }

        SetWindowPos(m_hwnd, m_hwnd, 0, 0, w, h + 38, SWP_NOZORDER);

        SetDIBits(hdcMem, bm, 0, h, bits, &bminfo, DIB_RGB_COLORS);

        BitBlt(pDC, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);

        

    }
    if (myBmp.isTiff)
    {
        w = myBmp.w - myBmp.w % 4;
        h = myBmp.h;

        BITMAPINFO bminfo;

        R_G_B* bits = new R_G_B[w * h];

        HDC pDC = GetDC(m_hwnd);
        HDC hdcMem = CreateCompatibleDC(pDC);
        HBITMAP bm = CreateCompatibleBitmap(pDC, w, h);

        HGDIOBJ hOld = SelectObject(hdcMem, bm);

        bminfo.bmiHeader.biSize = 40;
        bminfo.bmiHeader.biWidth = w;
        bminfo.bmiHeader.biHeight = h;
        bminfo.bmiHeader.biPlanes = 1;
        bminfo.bmiHeader.biBitCount = 24;
        bminfo.bmiHeader.biCompression = 0;
        bminfo.bmiHeader.biSizeImage = (w * sizeof(R_G_B)) * abs(h);
        bminfo.bmiHeader.biXPelsPerMeter = 0;
        bminfo.bmiHeader.biYPelsPerMeter = 0;
        bminfo.bmiHeader.biClrUsed = 0;
        bminfo.bmiHeader.biClrImportant = 0;


        R_G_B red;
        int v = 0;
        std::vector<std::vector<R_G_B>> mypix = myBmp.getPixels();
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++)
            {
                red = { mypix[i][j].b, mypix[i][j].g, mypix[i][j].r };
                bits[v] = red;
                v++;
            }

        SetWindowPos(m_hwnd, m_hwnd, 0, 0, w, h + 38, SWP_NOZORDER);

        SetDIBits(hdcMem, bm, 0, h, bits, &bminfo, DIB_RGB_COLORS);

        BitBlt(pDC, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);

    }
}
    
    
    
    


LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        //открытие openfolder окна
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
            COINIT_DISABLE_OLE1DDE);
        if (SUCCEEDED(hr))
        {
            IFileOpenDialog* pFileOpen;

            //создание FileOpenDialog
            hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

            if (SUCCEEDED(hr))
            {

                hr = pFileOpen->Show(NULL);
                
                // получение имени файла
                if (SUCCEEDED(hr))
                {
                    IShellItem* pItem;
                    hr = pFileOpen->GetResult(&pItem);
                    if (SUCCEEDED(hr))
                    {
                        PWSTR pszFilePath;
                        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                        if (SUCCEEDED(hr))
                        {
                            CoTaskMemFree(pszFilePath);
                            wcstombs(Name, pszFilePath, 100);
                        }

                    }
                }

            }

        }
        if (FAILED(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
        {
            return -1;
        }
        return 0;
    }


    case WM_DESTROY:
        DiscardGraphicsResources();
        SafeRelease(&pFactory);
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        OnPaint();
        break;

    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}
