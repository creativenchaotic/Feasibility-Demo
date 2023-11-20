#pragma once
#include "DXF.h"

using namespace DirectX;

//Simple shader to render the SPH particles. 
class SPHShader :
    public BaseShader
{
public:
    SPHShader(ID3D11Device* device, HWND hwnd);
    ~SPHShader();

    void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection);

private:
    void initShader(const wchar_t* cs, const wchar_t* ps);


    ID3D11Buffer* matrixBuffer;
};

