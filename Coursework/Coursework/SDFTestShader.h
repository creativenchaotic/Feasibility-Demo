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
        XMFLOAT4 numParticles;
    };

public:
    SDFTestShader(ID3D11Device* device, HWND hwnd);
    ~SDFTestShader();

    void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, XMFLOAT3 cameraPos, float delta, ID3D11ShaderResourceView* renderTexture);
    void setParticlePositionsSRV(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* computeShaderSRV);//Sets the SRV for the compute shader
	void setSDFParameters(ID3D11DeviceContext*, float blendVal, float numParticles);

private:
    void initShader(const wchar_t* cs, const wchar_t* ps);

    ID3D11Buffer* matrixBuffer;
    ID3D11Buffer* cameraBuffer;
    ID3D11Buffer* sdfBuffer;
    ID3D11SamplerState* sampleState;
};

