#pragma once
#include "DXF.h"
#include "Externals.h"

using namespace DirectX;

//Simple shader to render the SPH particles. 
class SPHShader :
    public BaseShader
{
public:
    SPHShader(ID3D11Device* device, HWND hwnd);
    ~SPHShader();

    void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, XMFLOAT4 cameraPos, RenderSettings renderSetting);

private:
    void initShader(const wchar_t* cs, const wchar_t* ps);
	void setLightingParameters(ID3D11DeviceContext* deviceContext, Light* light);
	void setMaterialValues(ID3D11DeviceContext* deviceContext, float roughness, float metallic, float reflectivity);


	//Light values
	struct LightBufferType
	{
		XMFLOAT4 diffuse;
		XMFLOAT4 ambient;
		XMFLOAT4 direction;
		XMFLOAT4 lightPosition;
	};

	//Camera position
	struct CameraBufferType
	{
		XMFLOAT4 cameraPosition;
		XMFLOAT4 renderSettings;
	};

	//Material values
	struct MaterialBufferType {
		float roughness;
		float metallic;
		float baseReflectivity;
		float padding;
	};

    ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* materialBuffer;
};

