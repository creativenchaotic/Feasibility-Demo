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
        XMMATRIX viewMatrix;
    };

    struct SDFValuesBufferType
    {
        float blendingAmount;
        int numParticles;
        int currentRenderSetting;
        int simType;
    };

    //Light values
    struct LightBufferType
    {
        XMFLOAT4 diffuse;
        XMFLOAT4 ambient;
        XMFLOAT4 direction;
        XMFLOAT4 lightPosition;
    };

    //Material values
    struct MaterialBufferType {
        float roughness;
        float metallic;
        float baseReflectivity;
        float particleSize;
    };

public:
    SDFTestShader(ID3D11Device* device, HWND hwnd);
    ~SDFTestShader();

    void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, XMFLOAT3 cameraPos, float delta, ID3D11ShaderResourceView* renderTexture);
    void setParticlePositionsSRV(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* computeShaderSRV, ID3D11ShaderResourceView* texture3d);//Sets the SRV for the compute shader
	void setSDFParameters(ID3D11DeviceContext*, float blendVal, int numParticles, RenderSettings currentRenderSetting, RenderSimulationType currentSimType);
    void setLightingParameters(ID3D11DeviceContext* deviceContext, Light* light);
    void setMaterialValues(ID3D11DeviceContext* deviceContext, float roughness, float metallic, float reflectivity, float particleSize);
    void unbind(ID3D11DeviceContext* dc);


private:
    void initShader(const wchar_t* cs, const wchar_t* ps);

    ID3D11Buffer* matrixBuffer;
    ID3D11Buffer* cameraBuffer;
    ID3D11Buffer* sdfBuffer;
    ID3D11Buffer* materialBuffer;
    ID3D11Buffer* lightBuffer;
    ID3D11SamplerState* sampleState;
    ID3D11SamplerState* sampleState3D;

};

