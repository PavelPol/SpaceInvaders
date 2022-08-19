#pragma once
#ifndef GRAPHIC_H
#define GRAPHIC_H

#include <windows.h>
#include <d3d9.h>
#include <d3dx9mesh.h>
#include <dxerr.h>
#include <vector>
#include <string>

class Graphic
{
public:
    struct CUSTOMVERTEX { FLOAT x, y, z, rhw; DWORD color; };
    struct TEXCUSTOMVERTEX { FLOAT x, y, z, rhw; float tx, ty; };
private:
    // Window
    int wndWidth, 
        wndHeight;
    HWND hWnd;
    WNDPROC messageProc;
    // DirectX
    LPDIRECT3D9             pD3D;
    LPDIRECT3DDEVICE9       pDevice;
    LPD3DXFONT              pFont;
    LPDIRECT3DVERTEXBUFFER9 pVertexBuffer;
    LPDIRECT3DVERTEXBUFFER9 bgVertexBuffer;
    LPDIRECT3DTEXTURE9      bgTexture;

    int triangleCount;
    std::wstring statusText;
    

    int vertexConstruct(int x, int y, 
                        const CUSTOMVERTEX *sourceVertex, CUSTOMVERTEX *vertex, int size);
    bool createWindow();
    void closeWindow();
    void terminateIfFailed(HRESULT hr);
    void Graphic::destroy();
public:
    static const int CellSize;
    
    enum {
        WHITE = D3DCOLOR_XRGB(255, 255, 255),
        RED   = D3DCOLOR_XRGB(255,   0,   0),
        GREEN = D3DCOLOR_XRGB(  0, 255,   0),
        BLUE  = D3DCOLOR_XRGB(  0,   0, 255)
    };

    Graphic(int wndWidth, int wndHeight, WNDPROC _msgproc);
    ~Graphic();

    int drawEnemyShip          (int x, int y, int color, CUSTOMVERTEX *vertex);
    int drawPlayerShip         (int x, int y, int color, CUSTOMVERTEX *vertex);
    int drawWall               (int x, int y, int color, CUSTOMVERTEX *vertex);
    int drawPlayerRocket       (int x, int y, int color, CUSTOMVERTEX *vertex);
    int drawPlayerHomingRocket (int x, int y, int color, CUSTOMVERTEX *vertex);
    int drawEnemyRocket        (int x, int y, int color, CUSTOMVERTEX *vertex);
    int drawBang               (int x, int y, int color, CUSTOMVERTEX *vertex);

    void clear();
    void setVertex(CUSTOMVERTEX *_vertex, int triangleCount);
    void execMessages();
    void setStatusText(const std::wstring text);

    void render();
};

#endif