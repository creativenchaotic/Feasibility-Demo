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
	void setSimulationConstants(ID3D11DeviceContext* dc, int numParticlesVal, float gravityVal, float delta, float bounceDamping,float smoothingRadiusVal, float targetDensityVal,float pressureMultiplierVal, float nearPressureMultVal, float viscosity, float edgeForceVal, float edgeForceDistanceVal, XMFLOAT2 bb_TopBottom, XMFLOAT2 bb_FrontBack, XMFLOAT2 bb_Sides);//Used to pass in constant variables such as gravity or damping values
	void unbind(ID3D11DeviceContext* dc);
	ID3D11ShaderResourceView* getComputeShaderOutput() { return particlesOutputReadable; };


private:

	struct SimulationConstantsBufferType {
		int numParticles;
		float gravity;
		float deltaTime;
		float collisionsDamping;

		float smoothingRadius;
		float targetDensity;
		float pressureMultiplier;
		float nearPressureMultiplier;

		float viscosityStrength;
		float edgeForce;
		float edgeForceDst;
		float padding = 0.0f;

		XMFLOAT2 boundingBoxTopAndBottom;
		XMFLOAT2 boudningBoxFrontAndBack;
		XMFLOAT2 boundingBoxSides;
		XMFLOAT2 padding2 = XMFLOAT2(0.0f,0.0f);
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

