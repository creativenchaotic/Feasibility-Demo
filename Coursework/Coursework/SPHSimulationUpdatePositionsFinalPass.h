#pragma once
#include "DXF.h"

class SPHSimulationUpdatePositionsFinalPass : public BaseShader
{

public:
	SPHSimulationUpdatePositionsFinalPass(ID3D11Device* device, HWND hwnd);
	~SPHSimulationUpdatePositionsFinalPass();

	void setShaderParameters(ID3D11DeviceContext* dc);
	void createOutputUAVs(ID3D11Device* pd3dDevice, int numParticles);
	void createInitialDataSRV(ID3D11Device* pd3dDevice, std::vector<XMFLOAT4>* particleData);
	void setSimulationConstants(ID3D11DeviceContext* dc, int numParticlesVal, float gravityVal, float delta, float bounceDamping, float smoothingRadiusVal, float targetDensityVal, float pressureMultiplierVal, float nearPressureMultVal, float viscosity, float edgeForceVal, float edgeForceDistanceVal, float bb_Top, float bb_Bottom, float bb_LeftSide, float bb_rightSide, float bb_Back, float bb_front, XMMATRIX localToWorld, XMMATRIX worldToLocal, int isSampleWave);//Used to pass in constant variables such as gravity or damping values
	void unbind(ID3D11DeviceContext* dc);
	ID3D11ShaderResourceView* getComputeShaderOutput() { return particlesOutputReadable; };
	void setSimulationDataSRV(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* sphSimViscosityPassOutput);
	void release();

	void setWaveParameters(ID3D11DeviceContext* deviceContext, float deltaTime, float ampl, float freq, float speed, XMFLOAT3 direction, float ampl2, float freq2, float speed2, XMFLOAT3 direction2, float ampl3, float freq3, float speed3, XMFLOAT3 direction3, float steepnessFactor, int isSampleWave);

private:

	struct SimulationConstantsBufferType {
		uint32_t numParticles;
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
		int isSampleWave;

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

		int isSampleWave;

		XMFLOAT4 direction3;
	};

	void initShader(const wchar_t* cfile, const wchar_t* blank);

	//RW Structured Buffers-----------------------------------------------
	ID3D11Buffer* particlesComputeShaderOutput;
	ID3D11ShaderResourceView* particlesOutputReadable;
	ID3D11UnorderedAccessView* particlesOutputWritable;

	//Initial Particle Data-----------------------------------------------
	ID3D11Buffer* particlesInitialData;
	ID3D11ShaderResourceView* particlesInitialDataReadable;

	//Constant Buffer-----------------------------------------------------
	ID3D11Buffer* simulationConstantsBuffer;
	ID3D11Buffer* timeBuffer;
};

