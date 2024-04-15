#pragma once
#include "DXF.h"
#include "SPH_Particle.h"

using namespace DirectX;

class SPHSimulationComputeShaderSecondPass : public BaseShader
{
public:
	SPHSimulationComputeShaderSecondPass(ID3D11Device* device, HWND hwnd);
	~SPHSimulationComputeShaderSecondPass();

	void setShaderParameters(ID3D11DeviceContext* dc);
	void createOutputUAVs(ID3D11Device* pd3dDevice, int numParticles);
	void setSimulationConstants(ID3D11DeviceContext* dc, int numParticlesVal, float gravityVal, float delta, float bounceDamping, float smoothingRadiusVal, float targetDensityVal, float pressureMultiplierVal, float nearPressureMultVal, float viscosity, float edgeForceVal, float edgeForceDistanceVal, float bb_Top, float bb_Bottom, float bb_LeftSide, float bb_rightSide, float bb_Back, float bb_front, XMMATRIX localToWorld, XMMATRIX worldToLocal);//Used to pass in constant variables such as gravity or damping values
	void unbind(ID3D11DeviceContext* dc);
	ID3D11ShaderResourceView* getComputeShaderOutput() { return particlesOutputReadable; };
	void setSimulationDataSRV(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* sphSimulationFirstPassOutput, ID3D11ShaderResourceView* computeShaderSRVBitonicMergesortParticleIndices, ID3D11ShaderResourceView* computeShaderSRVOffsetsCalculationOutput);


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

		float boundingBoxTop;
		float boundingBoxBottom;
		float boundingBoxLeftSide;
		float boundingBoxRightSide;

		float boundingBoxFront;
		float boundingBoxBack;
		XMFLOAT2 padding2 = XMFLOAT2(0.0f, 0.0f);

		XMMATRIX localToWorld;
		XMMATRIX worldToLocal;
	};

	void initShader(const wchar_t* cfile, const wchar_t* blank);

	//RW Structured Buffers-----------------------------------------------
	ID3D11Buffer* particlesComputeShaderOutput;
	ID3D11ShaderResourceView* particlesOutputReadable;
	ID3D11UnorderedAccessView* particlesOutputWritable;

	//Constant Buffer-----------------------------------------------------
	ID3D11Buffer* simulationConstantsBuffer;
};

