#pragma once
#include "DXF.h"
#include "SPH_Particle.h"

using namespace DirectX;

class OffsetCalculationComputeShader :
    public BaseShader
{
public:
    OffsetCalculationComputeShader(ID3D11Device* device, HWND hwnd);
    ~OffsetCalculationComputeShader();
    void setShaderParameters(ID3D11DeviceContext* dc);
    void createOutputUAVs(ID3D11Device* pd3dDevice, int numParticles, std::vector<ParticleData>* particles);
    void setOffsetCalculationsSettings(ID3D11DeviceContext* dc, int numParticlesVal);
    void unbind(ID3D11DeviceContext* dc);
    ID3D11ShaderResourceView* getComputeShaderOutput() { return offsetCalculationsOutputReadable; };
    void setSimulationDataSRV(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* computeShaderSRV);

private:
    void initShader(const wchar_t* cfile, const wchar_t* blank);

    struct OffsetCalculationsSettingsBufferType {
        int numParticles;
        XMFLOAT3 padding = XMFLOAT3(0.0f, 0.0f, 0.0f);
    };

    //RW Structured Buffers-----------------------------------------------
    ID3D11Buffer* offsetCalculationsOutput;
    ID3D11ShaderResourceView* offsetCalculationsOutputReadable;
    ID3D11UnorderedAccessView* offsetCalculationsOutputWritable;


    //Constant Buffer-----------------------------------------------------
    ID3D11Buffer* offsetCalculationsSettingsBuffer;
};

