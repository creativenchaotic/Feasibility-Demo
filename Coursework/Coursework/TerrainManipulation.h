#pragma once

#include "DXF.h"

//Terrain shader
using namespace std;
using namespace DirectX;

class TerrainManipulation : public BaseShader
{
private:
	//Light values
	struct LightBufferType
	{
		XMFLOAT4 diffuse[2];
		XMFLOAT4 ambient[2];
		XMFLOAT4 direction[2];
		XMFLOAT4 lightPosition[2];
		XMFLOAT4 spotlightSize;

		float lightType[4];

	};

	//Attenuation values
	struct AttenuationBufferType {
		float constantFactor;
		float linearFactor;
		float quadraticFactor;
		float padding2;
	};

	//Camera position
	struct CameraBufferType
	{
		XMFLOAT4 cameraPosition;
	};

	//Material values
	struct MaterialBufferType {
		XMFLOAT4 grassMetallic;
		XMFLOAT4 grassBaseReflectivity;
		XMFLOAT4 rockMetallic;
		XMFLOAT4 rockReflectivity;
		XMFLOAT4 snowMetallic;
		XMFLOAT4 snowReflectivity;
		XMFLOAT4 sandMetallic;
		XMFLOAT4 sandReflectivity;

	};

	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX lightView;
		XMMATRIX lightProjection;
		XMMATRIX spotlightView;
		XMMATRIX spotlightProjection;
	};


public:
	TerrainManipulation(ID3D11Device* device, HWND hwnd);
	~TerrainManipulation();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* texture2, ID3D11ShaderResourceView* texture3, ID3D11ShaderResourceView* texture4, ID3D11ShaderResourceView* texture5, ID3D11ShaderResourceView* texture6, ID3D11ShaderResourceView* texture7, ID3D11ShaderResourceView* texture8, ID3D11ShaderResourceView* texture9, ID3D11ShaderResourceView* texture10, Light* directionalLight, Light* spotlight);
	void setAttenuationFactors(ID3D11DeviceContext* deviceContext, XMFLOAT3 attenuationFactor);
	void setLightingParameters(ID3D11DeviceContext* deviceContext, Light* light, Light* light2, float lightType, float lightType2, float spotlightSize);
	void setCameraPosition(ID3D11DeviceContext* deviceContext, XMFLOAT3 cameraPos);
	void setMaterialValues(ID3D11DeviceContext* deviceContext, float grassMetallic, float grassReflectivity, float rockMetallic, float rockReflectivity, float snowMetallic, float snowReflectivity, float sandMetallic, float sandReflectivity);
	void setShadowValues(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* depthTxtr, ID3D11ShaderResourceView* depthTxtr2);

private:
	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleState2;
	ID3D11SamplerState* shadowSampler;

	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* attenuationBuffer;
	ID3D11Buffer* materialBuffer;
};

