#pragma once
#include "DXF.h"

using namespace DirectX;

class OffsetCalculationComputeShader :
    public BaseShader
{
public:
    OffsetCalculationComputeShader(ID3D11Device* device, HWND hwnd);
    ~OffsetCalculationComputeShader();
    void setShaderParameters(ID3D11DeviceContext* dc);
    void createOutputUAVs(ID3D11Device* pd3dDevice, int numParticles, std::vector<int>* particles);
    void unbind(ID3D11DeviceContext* dc);
    ID3D11ShaderResourceView* getComputeShaderOutput() { return offsetCalculationsOutputReadable; };

private:
    void initShader(const wchar_t* cfile, const wchar_t* blank);


    //RW Structured Buffers-----------------------------------------------
    ID3D11Buffer* offsetCalculationsOutput;
    ID3D11ShaderResourceView* offsetCalculationsOutputReadable;
    ID3D11UnorderedAccessView* offsetCalculationsOutputWritable;
};

