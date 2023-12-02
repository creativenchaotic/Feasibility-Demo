#pragma once
#include "DXF.h"
#include "SPH_Particle.h"

using namespace DirectX;


class BitonicMergesort : public BaseShader
{
public:
	BitonicMergesort(ID3D11Device* device, HWND hwnd);
	~BitonicMergesort();

	void setShaderParameters(ID3D11DeviceContext* dc);
	void createOutputUAVs(ID3D11Device* pd3dDevice, int numParticles, std::vector<XMFLOAT3>* particles);
	void setBitonicMergesortSettings(ID3D11DeviceContext* dc, int numParticlesVal, int groupWidthVal, int groupHeightVal, int stepIndexVal);
	void unbind(ID3D11DeviceContext* dc);
	ID3D11ShaderResourceView* getComputeShaderOutput() { return bitonicMergesortOutputReadable; };

private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);

	struct BitonicMergesortSettingsBufferType {
		int numParticles;
		int groupWidth;
		int groupHeight;
		int stepIndex;
	};

	//RW Structured Buffers-----------------------------------------------
	ID3D11Buffer* bitonicMergesortOutput;
	ID3D11ShaderResourceView* bitonicMergesortOutputReadable;
	ID3D11UnorderedAccessView* bitonicMergesortOutputWritable;

	//Constant Buffer-----------------------------------------------------
	ID3D11Buffer* bitonicMergesortSettingsBuffer;
};

