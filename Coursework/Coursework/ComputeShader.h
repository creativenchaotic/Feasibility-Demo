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

	//Buffer to input data into the compute shader------------------------
	ID3D11Buffer* particlesComputeShaderInput;
	ID3D11ShaderResourceView* particlesComputeShaderInputSRV;

	//RW Structured Buffers-----------------------------------------------
	ID3D11Buffer* particlesComputeShaderOutput;
	ID3D11ShaderResourceView* particlesOutputReadable;
	ID3D11UnorderedAccessView* particlesOutputWritable;

	//Particle Positions
	ID3D11Buffer* particlePositionBuffer;
	ID3D11ShaderResourceView* pariclePositionBufferSRV;
	ID3D11UnorderedAccessView* pariclePositionBufferUAV;

	//Particle Predicted Positions
	ID3D11Buffer* particlePredictedPositionBuffer;
	ID3D11ShaderResourceView* pariclePredictedPositionBufferSRV;
	ID3D11UnorderedAccessView* pariclePredictedPositionBufferUAV;

	//Particle Velocities
	ID3D11Buffer* particleVelocityBuffer;
	ID3D11ShaderResourceView* particleVelocityBufferSRV;
	ID3D11UnorderedAccessView* particleVelocityBufferUAV;

	//Particle Densities
	ID3D11Buffer* particleDensityBuffer;
	ID3D11ShaderResourceView* particleDensityBufferSRV;
	ID3D11UnorderedAccessView* particleDensityBufferUAV;

	//Particle Spatial Indices
	ID3D11Buffer* spatialIndicesBuffer;
	ID3D11ShaderResourceView* spatialIndicesBufferSRV;
	ID3D11UnorderedAccessView* spatialIndicesBufferUAV;

	//Particle Spatial Offsets
	ID3D11Buffer* spatialOffsetsBuffer;
	ID3D11ShaderResourceView* spatialOffsetsBufferSRV;
	ID3D11UnorderedAccessView* spatialOffsetsBufferUAV;

	//Constant Buffer-----------------------------------------------------
	ID3D11Buffer* simulationConstantsBuffer;

};

