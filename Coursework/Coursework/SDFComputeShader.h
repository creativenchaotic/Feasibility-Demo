#pragma once
#include "D:\University\Year 4\CMP400 Honours Project Proposal and Execution\Feasibility Demo\Code\Demo\Feasibility-Demo\Coursework\include\BaseShader.h"
#include <vector>
#include "Externals.h"

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
    ID3D11ShaderResourceView* getTexture3D() { return texture3DComputeShaderOutputReadable; };
    void setSimulationDataSRV(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* simulationOutputRV);

    //Passes simulation values into compute shader buffer
    void setBufferConstants(ID3D11DeviceContext* dc, int numParticlesVal, float blendAmount, int stride, int offset, RenderSimulationType currentSimType);

    void release();

    void setWaveParameters(ID3D11DeviceContext* deviceContext, float deltaTime, float ampl, float freq, float speed, XMFLOAT3 direction, float ampl2, float freq2, float speed2, XMFLOAT3 direction2, float ampl3, float freq3, float speed3, XMFLOAT3 direction3, float steepnessFactor, int isSampleWave);

private:
    void initShader(const wchar_t* cfile, const wchar_t* blank);

    struct SDFConstantBufferType
    {
        int numParticles;
        float blendAmount;
        int stride;
        float offset;
        XMFLOAT4 renderSetting;
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

    //RW Structured Buffers-----------------------------------------------
    ID3D11Buffer* particlesComputeShaderOutput;
    ID3D11ShaderResourceView* sdfPixelCalcOutputReadable;
    ID3D11UnorderedAccessView* sdfPixelCalcOutputWritable;

    ID3D11Texture3D* texture3DComputeShaderOutput;
    ID3D11ShaderResourceView* texture3DComputeShaderOutputReadable;
    ID3D11UnorderedAccessView* texture3DComputeShaderOutputWritable;

    ID3D11Buffer* particlesInitialData;
    ID3D11ShaderResourceView* particlesInitialDataReadable;

    //Constant Buffer-----------------------------------------------------
    ID3D11Buffer* sdfConstantsBuffer;
    ID3D11Buffer* timeBuffer;


};

