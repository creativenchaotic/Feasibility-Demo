#pragma once
#include "DXF.h"
#include "SPH_Particle.h"

using namespace DirectX;

//BITONIC MERGESORT SORTING ALGORITHM USED TO SORT THE PARTICLES BASED ON A HASHING VALUE SET IN THE INITIAL SPH SIM COMPUTE SHADER


class BitonicMergesort : public BaseShader
{
public:
	BitonicMergesort(ID3D11Device* device, HWND hwnd);
	~BitonicMergesort();

	void setShaderParameters(ID3D11DeviceContext* dc);
	void createOutputUAVs(ID3D11Device* pd3dDevice, int numParticles);
	void setBitonicMergesortSettings(ID3D11DeviceContext* dc, int numParticlesVal, int groupWidthVal, int groupHeightVal, int stepIndexVal);
	void setSimulationDataSRV(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* computeShaderSRV);
	void unbind(ID3D11DeviceContext* dc);
	ID3D11ShaderResourceView* getComputeShaderOutput() { return bitonicMergesortOutputReadable; };
	void release();

private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);

	struct BitonicMergesortSettingsBufferType {
		uint32_t numParticles;
		uint32_t groupWidth;
		uint32_t groupHeight;
		uint32_t stepIndex;
	};

	//RW Structured Buffers-----------------------------------------------
	ID3D11Buffer* bitonicMergesortOutput;
	ID3D11ShaderResourceView* bitonicMergesortOutputReadable;
	ID3D11UnorderedAccessView* bitonicMergesortOutputWritable;

	//Constant Buffer-----------------------------------------------------
	ID3D11Buffer* bitonicMergesortSettingsBuffer;
};

