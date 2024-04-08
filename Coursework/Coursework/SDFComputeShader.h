#pragma once
#include "D:\University\Year 4\CMP400 Honours Project Proposal and Execution\Feasibility Demo\Code\Demo\Feasibility-Demo\Coursework\include\BaseShader.h"
#include <vector>

class SDFComputeShader :
    public BaseShader
{
public:
    SDFComputeShader(ID3D11Device* device, HWND hwnd);
    ~SDFComputeShader();

    void setShaderParameters(ID3D11DeviceContext* dc);//Sets the output UAV for the compute shader
    void createOutputUAVs(ID3D11Device* pd3dDevice, std::vector<XMFLOAT4>* particles);//Creates the output UAVs for the shader
    void unbind(ID3D11DeviceContext* dc);
    ID3D11ShaderResourceView* getComputeShaderOutput() { return sdfPixelCalcOutputReadable; };

    //Passes simulation values into compute shader buffer
    void setBufferConstants(ID3D11DeviceContext* dc, int numParticlesVal, float blendAmount);

private:
    void initShader(const wchar_t* cfile, const wchar_t* blank);

    struct SDFConstantBufferType
    {
        int numParticles;
        float blendAmount;
        float padding = 0;
        float padding2 = 0;
    };

    //RW Structured Buffers-----------------------------------------------
    ID3D11Buffer* particlesComputeShaderOutput;
    ID3D11ShaderResourceView* sdfPixelCalcOutputReadable;
    ID3D11UnorderedAccessView* sdfPixelCalcOutputWritable;

    //Constant Buffer-----------------------------------------------------
    ID3D11Buffer* sdfConstantsBuffer;

};

