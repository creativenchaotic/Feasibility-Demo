#include "OffsetCalculationComputeShader.h"

OffsetCalculationComputeShader::OffsetCalculationComputeShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"OffsetCalculationComputeShader.cso", NULL);
}

OffsetCalculationComputeShader::~OffsetCalculationComputeShader()
{
}

void OffsetCalculationComputeShader::initShader(const wchar_t* cfile, const wchar_t* blank)
{
	loadComputeShader(cfile); //load + compile shader files

    D3D11_BUFFER_DESC offsetCalculationsBufferDesc;

    // Setup constant buffer
    offsetCalculationsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    offsetCalculationsBufferDesc.ByteWidth = sizeof(OffsetCalculationsSettingsBufferType);
    offsetCalculationsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    offsetCalculationsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    offsetCalculationsBufferDesc.MiscFlags = 0;
    offsetCalculationsBufferDesc.StructureByteStride = 0;
    renderer->CreateBuffer(&offsetCalculationsBufferDesc, NULL, &offsetCalculationsSettingsBuffer);
}

void OffsetCalculationComputeShader::setShaderParameters(ID3D11DeviceContext* dc)
{
	dc->CSSetUnorderedAccessViews(0, 1, &offsetCalculationsOutputWritable, 0);//Same as UAVs
}

void OffsetCalculationComputeShader::createOutputUAVs(ID3D11Device* pd3dDevice, int numParticles, std::vector<int>* particlesSpatialOffsets)
{
    //Creating a buffer to output the data from the compute shader
    D3D11_BUFFER_DESC bufferDesc = {};//Creating a buffer description to create the buffer from
    bufferDesc.ByteWidth = numParticles * sizeof(int);//sizeofT should be the particle data struct. Setting the size of the buffer to whatever amount is needed
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;//Setting how the buffer works
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(int);//make this the size of particle data//Setting the stride that the buffer needs to take when reading each element from memory
    D3D11_SUBRESOURCE_DATA bufferInitData;
    bufferInitData.pSysMem = particlesSpatialOffsets->data();//Initial data that is getting passed into the buffer
    pd3dDevice->CreateBuffer(&bufferDesc, (particlesSpatialOffsets) ? &bufferInitData : nullptr, &offsetCalculationsOutput);//Creates the buffer

    // Create SRV - Lets you read from the Buffer created
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = numParticles;
    pd3dDevice->CreateShaderResourceView(*&offsetCalculationsOutput, &srvDesc, &offsetCalculationsOutputReadable);//Creates the shader resource view from the buffer so you can read values

    // Create UAV - Lets you write from the compute shader
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements = numParticles;
    pd3dDevice->CreateUnorderedAccessView(*&offsetCalculationsOutput, &uavDesc, &offsetCalculationsOutputWritable);//Creates the unordered access view so you can write to the buffer

}

void OffsetCalculationComputeShader::setOffsetCalculationsSettings(ID3D11DeviceContext* deviceContext, int numParticlesVal)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    OffsetCalculationsSettingsBufferType* bitonicMergesortConstPtr;
    deviceContext->Map(offsetCalculationsSettingsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    bitonicMergesortConstPtr = (OffsetCalculationsSettingsBufferType*)mappedResource.pData;

    bitonicMergesortConstPtr->numParticles = numParticlesVal;

    deviceContext->Unmap(offsetCalculationsSettingsBuffer, 0);
    deviceContext->CSSetConstantBuffers(0, 1, &offsetCalculationsSettingsBuffer);
}

void OffsetCalculationComputeShader::unbind(ID3D11DeviceContext* dc)
{
    // Unbind output from compute shader
    ID3D11UnorderedAccessView* nullUAV[] = { NULL };
    dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

    // Disable Compute Shader
    dc->CSSetShader(nullptr, nullptr, 0);
}

void OffsetCalculationComputeShader::setSimulationDataSRV(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* computeShaderSRV)
{
    deviceContext->CSSetShaderResources(0, 1, &computeShaderSRV);
}

