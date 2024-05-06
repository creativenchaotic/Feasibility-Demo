#include "BitonicMergesort.h"

BitonicMergesort::BitonicMergesort(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"BitonicMergesort.cso", NULL);
}

BitonicMergesort::~BitonicMergesort()
{
}

void BitonicMergesort::initShader(const wchar_t* cfile, const wchar_t* blank)
{
    loadComputeShader(cfile); //load + compile shader files

    D3D11_BUFFER_DESC bitonicMergesortBufferDesc;

    // Setup constant buffer
    bitonicMergesortBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bitonicMergesortBufferDesc.ByteWidth = sizeof(BitonicMergesortSettingsBufferType);
    bitonicMergesortBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bitonicMergesortBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bitonicMergesortBufferDesc.MiscFlags = 0;
    bitonicMergesortBufferDesc.StructureByteStride = 0;
    renderer->CreateBuffer(&bitonicMergesortBufferDesc, NULL, &bitonicMergesortSettingsBuffer);
}

void BitonicMergesort::setShaderParameters(ID3D11DeviceContext* dc)
{
	dc->CSSetUnorderedAccessViews(0, 1, &bitonicMergesortOutputWritable, 0);//Same as UAVs
}

void BitonicMergesort::createOutputUAVs(ID3D11Device* pd3dDevice, int numParticles)
{

    //Creating a buffer to output the data from the compute shader
    D3D11_BUFFER_DESC bufferDesc = {};//Creating a buffer description to create the buffer from
    bufferDesc.ByteWidth = numParticles * sizeof(uint3);//sizeofT should be the particle data struct. Setting the size of the buffer to whatever amount is needed
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;//Setting how the buffer works
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(uint3);//make this the size of particle data//Setting the stride that the buffer needs to take when reading each element from memory
    pd3dDevice->CreateBuffer(&bufferDesc, nullptr, &bitonicMergesortOutput);//Creates the buffer

    // Create SRV - Lets you read from the Buffer created
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = numParticles;
    pd3dDevice->CreateShaderResourceView(*&bitonicMergesortOutput, &srvDesc, &bitonicMergesortOutputReadable);//Creates the shader resource view from the buffer so you can read values

    // Create UAV - Lets you write from the compute shader
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements = numParticles;
    pd3dDevice->CreateUnorderedAccessView(*&bitonicMergesortOutput, &uavDesc, &bitonicMergesortOutputWritable);//Creates the unordered access view so you can write to the buffer

}

void BitonicMergesort::setBitonicMergesortSettings(ID3D11DeviceContext* deviceContext, int numParticlesVal, int groupWidthVal, int groupHeightVal, int stepIndexVal)
{

    D3D11_MAPPED_SUBRESOURCE mappedResource;

    BitonicMergesortSettingsBufferType* bitonicMergesortConstPtr;
    deviceContext->Map(bitonicMergesortSettingsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    bitonicMergesortConstPtr = (BitonicMergesortSettingsBufferType*)mappedResource.pData;

    bitonicMergesortConstPtr->numParticles = numParticlesVal;
    bitonicMergesortConstPtr->groupWidth = groupWidthVal;
    bitonicMergesortConstPtr->groupHeight = groupHeightVal;
    bitonicMergesortConstPtr->stepIndex = stepIndexVal;

    deviceContext->Unmap(bitonicMergesortSettingsBuffer, 0);
    deviceContext->CSSetConstantBuffers(0, 1, &bitonicMergesortSettingsBuffer);

}

void BitonicMergesort::setSimulationDataSRV(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* computeShaderSRV)
{
    deviceContext->CSSetShaderResources(0, 1, &computeShaderSRV);

}

void BitonicMergesort::unbind(ID3D11DeviceContext* dc)
{
	// Unbind output from compute shader
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

    ID3D11ShaderResourceView* nullSRV[] = { NULL };
    dc->CSSetShaderResources(0, 1, nullSRV);

    ID3D11Buffer* nullBuffer[] = { NULL };
    dc->CSSetConstantBuffers(0, 1, nullBuffer);

	// Disable Compute Shader
	dc->CSSetShader(nullptr, nullptr, 0);
}



