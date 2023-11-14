#pragma once

#include "DXF.h"
//Water shader
using namespace std;
using namespace DirectX;

class WaterShader : public BaseShader
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

	//Wave manipulation values
	struct TimeBufferType {
		float time;
		float amplitude1;
		float frequency1;
		float speed1;

		XMFLOAT4 direction1;

		float amplitude2;
		float frequency2;
		float speed2;

		float steepnessFactor;

		XMFLOAT4 direction2;

		float amplitude3;
		float frequency3;
		float speed3;

		float waterHeight;

		XMFLOAT4 direction3;
	};

	//Camera position
	struct CameraBufferType
	{
		XMFLOAT4 cameraPosition;
	};

	//Attenuation values
	struct AttenuationBufferType {
		float constantFactor;
		float linearFactor;
		float quadraticFactor;
		float padding2;
	};

	//Material values
	struct MaterialBufferType {
		float roughness;
		float metallic;
		float baseReflectivity;
		float padding;
	};

public:
	WaterShader(ID3D11Device* device, HWND hwnd);
	~WaterShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, XMFLOAT4 waterSpecular, XMFLOAT4 cameraPos);
	void setWaveParameters(ID3D11DeviceContext* deviceContext, float deltaTime, float ampl, float freq, float speed, XMFLOAT3 direction, float ampl2, float freq2, float speed2, XMFLOAT3 direction2, float ampl3, float freq3, float speed3, XMFLOAT3 direction3,float steepnessFactor, float waterHeight);
	void setLightingParameters(ID3D11DeviceContext* deviceContext, Light* light, Light* light2, float lightType, float lightType2, float spotlightSize);
	void setAttenuationFactors(ID3D11DeviceContext* deviceContext, XMFLOAT3 attenuationFactor);
	void setMaterialValues(ID3D11DeviceContext* deviceContext, float roughness, float metallic, float reflectivity);

private:
	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* timeBuffer;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* attenuationBuffer;
	ID3D11Buffer* materialBuffer;
};

