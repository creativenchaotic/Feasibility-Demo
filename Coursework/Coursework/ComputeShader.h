#pragma once
#include "DXF.h"
#include "SPH_Particle.h"

using namespace DirectX;

class ComputeShader :
    public BaseShader
{
public:
	ComputeShader(ID3D11Device* device, HWND hwnd);
	~ComputeShader();

	void setShaderParameters(ID3D11DeviceContext* dc);
	void createOutputUAV(ID3D11Device* pd3dDevice, int numParticles);
	void createBuffer(ID3D11Device* pd3dDevice, int numParticles, std::vector<ParticleData>* particles);
	void setSimulationConstants(ID3D11DeviceContext* dc, float gravityVal, float bounceDamping, float numParticlesVal, float restDensityVal, float delta, XMFLOAT2 bb_TopBottom, XMFLOAT2 bb_FrontBack, XMFLOAT2 bb_Sides);//Used to pass in constant variables such as gravity or damping values
	void unbind(ID3D11DeviceContext* dc);
	ID3D11ShaderResourceView* getComputeShaderOutput() { return particlesOutputReadable; };


private:

	struct SimulationConstantsBufferType {
		float bounceDampingFactor;
		float gravity;
		float numParticles;
		float restDensity;

		float deltaTime;
		XMFLOAT2 boundingBoxTopAndBottom;
		XMFLOAT2 boudningBoxFrontAndBack;
		XMFLOAT2 boundingBoxSides;
		XMFLOAT2 padding = XMFLOAT2(0.0f,0.0f);
	};

	void initShader(const wchar_t* cfile, const wchar_t* blank);

	ID3D11Buffer* particlesComputeShaderInput;
	ID3D11ShaderResourceView* particlesComputeShaderInputSRV;

	ID3D11Buffer* particlesComputeShaderOutput;
	ID3D11ShaderResourceView* particlesOutputReadable;
	ID3D11UnorderedAccessView* particlesOutputWritable;

	ID3D11Buffer* simulationConstantsBuffer;

	float bounceDampingFactor;
	float gravity = 9.8f;

};

