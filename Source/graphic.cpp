#include "version.h"
#include <d3d9.h>
#include <d3dx9mesh.h>
#include <dxerr.h>

#include "graphic.h"
#include "resource.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#define D3DFVF_CUSTOMVERTEX    (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
#define D3DFVF_TEXCUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

const int Graphic::CellSize = 5;

bool Graphic::createWindow()
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    // Регистрация класса окна
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW, 
                      messageProc, 0L, 0L, 
                      hInstance, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON)), 
                      LoadCursor(NULL, IDC_ARROW), NULL, NULL, TEXT("ssi_wnd"), NULL };
    if(!RegisterClassEx(&wc)) {
        return false;
    }
    hWnd = CreateWindow(TEXT("ssi_wnd"), 
                        TEXT("sSpace Invaders"), 
                        WS_CAPTION | WS_SYSMENU, 
                        CW_USEDEFAULT, NULL,
                        wndWidth  + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
                        wndHeight + GetSystemMetrics(SM_CYSMCAPTION) + GetSystemMetrics(SM_CYFIXEDFRAME) * 2,
                        GetDesktopWindow(), NULL, 
                        hInstance, 0);
    if (!hWnd) {
        return false;
    }

    pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if(!pD3D) {
        DestroyWindow(hWnd);
        return false;
    }

    // задание параметров инициализации видеоустройства
    D3DPRESENT_PARAMETERS params;
    ZeroMemory(&params, sizeof(params));
    params.Windowed         = TRUE;
    params.SwapEffect       = D3DSWAPEFFECT_DISCARD;
    params.BackBufferFormat = D3DFMT_UNKNOWN;

    // инициализация видеоустройства
    HRESULT hr = pD3D->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
        &params, &pDevice);
    if(FAILED(hr)){
        // неинициализировано видеоустройство
        pD3D->Release();
        pD3D = 0;
        DestroyWindow(hWnd);
        return false;
    }
    pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

    if (D3DXCreateFont(pDevice, 18, 14, 1, 0, 0,
                       DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS,
                       DEFAULT_QUALITY, 0,
                       TEXT("Times New Roman"), &pFont)) {
        MessageBox(NULL, TEXT("Ошибка генерации шифта"), TEXT("sSpaceInvaders"), MB_OK);
        pFont = 0;
    }

    if(D3DXCreateTextureFromResource(pDevice, hInstance, 
        MAKEINTRESOURCE(IDT_MAIN), &pScreen) != D3D_OK) {
        pD3D->Release();
        pDevice->Release();
        pD3D = 0;
        pDevice = 0;
        DestroyWindow(hWnd);
        return false;
    }
    
    TEXCUSTOMVERTEX screen[] = {
        {            0.0f,             0.0f, 0.0f, 0.0f, (D3DCOLOR)WHITE, 0.0f, 0.0f },
        { (float)wndWidth,             0.0f, 0.0f, 0.0f, (D3DCOLOR)WHITE, 1.0f, 0.0f },
        {            0.0f, (float)wndHeight, 0.0f, 0.0f, (D3DCOLOR)WHITE, 0.0f, 1.0f },
        
        {            0.0f, (float)wndHeight, 0.0f, 0.0f, (D3DCOLOR)WHITE, 0.0f, 1.0f },
        { (float)wndWidth,             0.0f, 0.0f, 0.0f, (D3DCOLOR)WHITE, 1.0f, 0.0f },
        { (float)wndWidth, (float)wndHeight, 0.0f, 0.0f, (D3DCOLOR)WHITE, 1.0f, 1.0f }
    };
    pDevice->CreateVertexBuffer(sizeof (screen), 0, D3DFVF_CUSTOMVERTEX, 
                                D3DPOOL_DEFAULT, &pScreenVertexBuffer, 0);
    VOID *pTmpVertexBuffer;
    pScreenVertexBuffer->Lock(0, sizeof (screen),
                        (void**)&pTmpVertexBuffer, 0);
    memcpy(pTmpVertexBuffer, screen, sizeof (screen));
    pScreenVertexBuffer->Unlock();

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    InvalidateRect(hWnd, NULL, FALSE);

    return true;
}

void Graphic::closeWindow()
{
    if (pVertexBuffer) {
        pVertexBuffer->Release();
        pVertexBuffer = 0;
    }
    if (pFont) {
        pFont->Release();
        pFont = 0;
    }
    if (pDevice) {
        pDevice->Release();
        pDevice = 0;
    }
    if (pD3D) {
        pD3D->Release();
        pD3D = 0;
    }
    if (hWnd) {
        ShowWindow(hWnd, SW_HIDE);
        DestroyWindow(hWnd);
        hWnd = 0;
    }
    UnregisterClass(TEXT("ssi_wnd"), GetModuleHandle(0));
}

Graphic::Graphic(int _wndWidth, int _wndHeight, WNDPROC _msgproc) :
        wndWidth(_wndWidth), wndHeight(_wndHeight), 
        hWnd(0), messageProc(_msgproc),
        pD3D(0), pDevice(0), pFont(0), pVertexBuffer(0), pScreen(0),
        triangleCount(0)
{
    if (!createWindow()) {
        throw "Not create window";
    }
}

Graphic::~Graphic()
{
    closeWindow();
}

int Graphic::vertexConstruct(int x, int y, 
                             const CUSTOMVERTEX *sourceVertex,
                             CUSTOMVERTEX *vertex, int count)
{
    memcpy(vertex, sourceVertex, count * sizeof (CUSTOMVERTEX));
    for (int i = 0; i < count; i++) {
        vertex[i].x += x; vertex[i].x *= CellSize;
        vertex[i].y += y; vertex[i].y *= CellSize;
    }
    return count;
}

int Graphic::drawEnemyShip(int x, int y, int color, CUSTOMVERTEX *vertex)
{
    CUSTOMVERTEX sourceVertex[] =
    {
        { 0.0f, 0.0f, 0.0f, 0.0f, color },
        { 3.0f, 0.0f, 0.0f, 0.0f, color },
        { 0.0f, 1.0f, 0.0f, 0.0f, color },

        { 0.0f, 1.0f, 0.0f, 0.0f, color },
        { 3.0f, 0.0f, 0.0f, 0.0f, color },
        { 3.0f, 1.0f, 0.0f, 0.0f, color },

        { 0.0f, 1.0f, 0.0f, 0.0f, color },
        { 3.0f, 1.0f, 0.0f, 0.0f, color },
        { 1.0f, 4.0f, 0.0f, 0.0f, color },

        { 1.0f, 4.0f, 0.0f, 0.0f, color },
        { 3.0f, 1.0f, 0.0f, 0.0f, color },
        { 2.0f, 4.0f, 0.0f, 0.0f, color }
    };
    return vertexConstruct(x, y, sourceVertex, vertex, 
                           sizeof (sourceVertex) / sizeof (CUSTOMVERTEX));
}

int Graphic::drawPlayerShip(int x, int y, int color, CUSTOMVERTEX *vertex)
{
    CUSTOMVERTEX sourceVertex[] =
    {
        { 1.0f, 0.0f, 0.0f, 0.0f, color },
        { 2.0f, 0.0f, 0.0f, 0.0f, color },
        { 0.0f, 3.0f, 0.0f, 0.0f, color },

        { 2.0f, 0.0f, 0.0f, 0.0f, color },
        { 3.0f, 3.0f, 0.0f, 0.0f, color },
        { 0.0f, 3.0f, 0.0f, 0.0f, color },

        { 0.0f, 3.0f, 0.0f, 0.0f, color },
        { 3.0f, 3.0f, 0.0f, 0.0f, color },
        { 0.0f, 4.0f, 0.0f, 0.0f, color },

        { 0.0f, 4.0f, 0.0f, 0.0f, color },
        { 3.0f, 3.0f, 0.0f, 0.0f, color },
        { 3.0f, 4.0f, 0.0f, 0.0f, color }
    };
    return vertexConstruct(x, y, sourceVertex, vertex, 
                           sizeof (sourceVertex) / sizeof (CUSTOMVERTEX));
}

int Graphic::drawWall(int x, int y, int color, CUSTOMVERTEX *vertex)
{
    CUSTOMVERTEX sourceVertex[] =
    {
        { 0.0f, 0.0f, 0.0f, 0.0f, color },
        { 2.0f, 0.0f, 0.0f, 0.0f, color },
        { 0.0f, 2.0f, 0.0f, 0.0f, color },

        { 0.0f, 2.0f, 0.0f, 0.0f, color },
        { 2.0f, 0.0f, 0.0f, 0.0f, color },
        { 2.0f, 2.0f, 0.0f, 0.0f, color }
    };
    return vertexConstruct(x, y, sourceVertex, vertex, 
                           sizeof (sourceVertex) / sizeof (CUSTOMVERTEX));
}

int Graphic::drawPlayerRocket(int x, int y, int color, CUSTOMVERTEX *vertex)
{
    CUSTOMVERTEX sourceVertex[] =
    {
        { 0.0f, 0.0f, 0.0f, 0.0f, color },
        { 1.0f, 0.0f, 0.0f, 0.0f, color },
        { 0.0f, 2.0f, 0.0f, 0.0f, color },

        { 0.0f, 2.0f, 0.0f, 0.0f, color },
        { 1.0f, 0.0f, 0.0f, 0.0f, color },
        { 1.0f, 2.0f, 0.0f, 0.0f, color }
    };
    return vertexConstruct(x, y, sourceVertex, vertex, 
                           sizeof (sourceVertex) / sizeof (CUSTOMVERTEX));
}

int Graphic::drawPlayerHomingRocket(int x, int y, int color, CUSTOMVERTEX *vertex)
{
    CUSTOMVERTEX sourceVertex[] =
    {
        { 0.0f, 1.0f, 0.0f, 0.0f, color },
        { 1.0f, 1.0f, 0.0f, 0.0f, color },
        { 0.0f, 2.0f, 0.0f, 0.0f, color },
        
        { 1.0f, 0.0f, 0.0f, 0.0f, color },
        { 2.0f, 0.0f, 0.0f, 0.0f, color },
        { 1.0f, 1.0f, 0.0f, 0.0f, color },

        { 0.0f, 2.0f, 0.0f, 0.0f, color },
        { 2.0f, 0.0f, 0.0f, 0.0f, color },
        { 2.0f, 2.0f, 0.0f, 0.0f, color },
        
        { 2.0f, 1.0f, 0.0f, 0.0f, color },
        { 3.0f, 1.0f, 0.0f, 0.0f, color },
        { 2.0f, 2.0f, 0.0f, 0.0f, color },

        { 2.0f, 2.0f, 0.0f, 0.0f, color },
        { 3.0f, 1.0f, 0.0f, 0.0f, color },
        { 3.0f, 2.0f, 0.0f, 0.0f, color },
    };
    return vertexConstruct(x, y, sourceVertex, vertex, 
                           sizeof (sourceVertex) / sizeof (CUSTOMVERTEX));
}

int Graphic::drawEnemyRocket(int x, int y, int color, CUSTOMVERTEX *vertex)
{
    CUSTOMVERTEX sourceVertex[] =
    {
        { 0.0f, 0.0f, 0.0f, 0.0f, color },
        { 1.0f, 0.0f, 0.0f, 0.0f, color },
        { 0.0f, 2.0f, 0.0f, 0.0f, color },

        { 0.0f, 2.0f, 0.0f, 0.0f, color },
        { 1.0f, 0.0f, 0.0f, 0.0f, color },
        { 1.0f, 2.0f, 0.0f, 0.0f, color }
    };
    return vertexConstruct(x, y, sourceVertex, vertex, 
                           sizeof (sourceVertex) / sizeof (CUSTOMVERTEX));
}

int Graphic::drawBang(int x, int y, int color, CUSTOMVERTEX *vertex)
{
    CUSTOMVERTEX sourceVertex[] =
    {
        { 0.0f, 0.0f, 0.0f, 0.0f, color },
        { 2.0f, 1.0f, 0.0f, 0.0f, color },
        { 2.0f, 2.0f, 0.0f, 0.0f, color },

        { 2.0f, 1.0f, 0.0f, 0.0f, color },
        { 4.0f, 0.0f, 0.0f, 0.0f, color },
        { 2.0f, 2.0f, 0.0f, 0.0f, color },

        { 2.0f, 1.0f, 0.0f, 0.0f, color },
        { 4.0f, 3.0f, 0.0f, 0.0f, color },
        { 2.0f, 2.0f, 0.0f, 0.0f, color },

        { 0.0f, 3.0f, 0.0f, 0.0f, color },
        { 2.0f, 1.0f, 0.0f, 0.0f, color },
        { 2.0f, 2.0f, 0.0f, 0.0f, color }
    };
    return vertexConstruct(x, y, sourceVertex, vertex, 
                           sizeof (sourceVertex) / sizeof (CUSTOMVERTEX));
}

void Graphic::clear()
{
    pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 0.0f, 0);
    pDevice->BeginScene();
    pDevice->EndScene();
    pDevice->Present(NULL, NULL, NULL, NULL);
}

void Graphic::setVertex(CUSTOMVERTEX *_vertex, int _triangleCount)
{
    triangleCount = _triangleCount;
    pDevice->CreateVertexBuffer(triangleCount * 3 * sizeof (CUSTOMVERTEX), 0, 
                                D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &pVertexBuffer, 0);

    VOID *pTmpVertexBuffer;
    pVertexBuffer->Lock(0, triangleCount * 3 *  sizeof (CUSTOMVERTEX),
                        (void**)&pTmpVertexBuffer, 0);
    memcpy(pTmpVertexBuffer, _vertex, triangleCount * 3 * sizeof (CUSTOMVERTEX));
    pVertexBuffer->Unlock();

    pDevice->SetStreamSource(0, pVertexBuffer, 0, sizeof(CUSTOMVERTEX));

    UpdateWindow(hWnd);
    InvalidateRect(hWnd, NULL, FALSE);
}

void Graphic::execMessages()
{
    MSG msg;
    if (GetMessage(&msg, hWnd, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg); 
    }
}

void Graphic::render()
{
    if(!pDevice)
        return;

    // очистка области вывода цветом
    pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 0.0f, 0);

    // прорисовка сцены
    if(SUCCEEDED(pDevice->BeginScene())) {
        pDevice->SetTexture(0, pScreen);
        pDevice->SetStreamSource(0, pScreenVertexBuffer, 0, sizeof(TEXCUSTOMVERTEX));
        pDevice->SetFVF(D3DFVF_TEXCUSTOMVERTEX);
        pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
        
        pDevice->SetTexture(0, 0);
        pDevice->SetStreamSource(0, pVertexBuffer, 0, sizeof(CUSTOMVERTEX));
        pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
        pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, triangleCount);

        if (pFont) {
            static RECT rect = {10, wndHeight - 25, wndWidth - 10, wndHeight - 5};
            pFont->DrawText(0, statusText.c_str(), -1, &rect,
                            DT_SINGLELINE | DT_LEFT | DT_VCENTER, (D3DCOLOR)WHITE);
        }

        // завершить вывод сцены
        pDevice->EndScene();
    }

    // вывод содержимого вторичного буфера
    pDevice->Present(NULL, NULL, NULL, NULL);
}

void Graphic::setStatusText(const std::wstring text)
{
    statusText = text;
}