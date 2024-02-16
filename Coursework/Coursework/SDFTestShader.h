#pragma once
#include "DXF.h"
#include "Externals.h"

//Base Test shader to test using signed distance fields

class SDFTestShader :
    public BaseShader
{
    struct CameraBufferType {
        XMFLOAT4 cameraPosition;
        XMFLOAT4 timer;
    };

    struct SDFValuesBufferType
    {
        XMFLOAT4 blendingAmount;
    };

public:
    SDFTestShader(ID3D11Device* device, HWND hwnd);
    ~SDFTestShader();

    void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, XMFLOAT3 cameraPos, float delta);
    void setSDFParameters(ID3D11DeviceContext*, float blendVal);

private:
    void initShader(const wchar_t* cs, const wchar_t* ps);

    ID3D11Buffer* matrixBuffer;
    ID3D11Buffer* cameraBuffer;
    ID3D11Buffer* sdfBuffer;
};

