#include "SDFComputeShader.h"

SDFComputeShader::SDFComputeShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"SDF_cs.cso", NULL);
}

SDFComputeShader::~SDFComputeShader()
{
}

void SDFComputeShader::initShader(const wchar_t* cfile, const wchar_t* blank)
{
    loadComputeShader(cfile); //load + compile shader files


    D3D11_BUFFER_DESC simConstantsBufferDesc;

    // Setup constant buffer
    simConstantsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    simConstantsBufferDesc.ByteWidth = sizeof(SDFConstantBufferType);
    simConstantsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    simConstantsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    simConstantsBufferDesc.MiscFlags = 0;
    simConstantsBufferDesc.StructureByteStride = 0;
    renderer->CreateBuffer(&simConstantsBufferDesc, NULL, &sdfConstantsBuffer);
}

void SDFComputeShader::setBufferConstants(ID3D11DeviceContext* dc, int numParticlesVal, float blendAmount)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    SDFConstantBufferType* simulationConstPtr;
    dc->Map(sdfConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    simulationConstPtr = (SDFConstantBufferType*)mappedResource.pData;

    simulationConstPtr->numParticles = numParticlesVal;
    simulationConstPtr->blendAmount = blendAmount;
    simulationConstPtr->padding = 0.0f;
    simulationConstPtr->padding2 = 0.0f;

    dc->Unmap(sdfConstantsBuffer, 0);
    dc->CSSetConstantBuffers(0, 1, &sdfConstantsBuffer);
}

void SDFComputeShader::setShaderParameters(ID3D11DeviceContext* dc)
{
	dc->CSSetUnorderedAccessViews(0, 1, &sdfPixelCalcOutputWritable, 0);//Same as UAVs
}

void SDFComputeShader::createOutputUAVs(ID3D11Device* pd3dDevice, std::vector<XMFLOAT4>* particles)
{
    //Creating a buffer to output the data from the compute shader
    D3D11_BUFFER_DESC bufferDesc = {};//Creating a buffer description to create the buffer from
    bufferDesc.ByteWidth = particles->size() * sizeof(particles[0]);//sizeofT should be the particle data struct. Setting the size of the buffer to whatever amount is needed
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;//Setting how the buffer works
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(particles);//make this the size of particle data//Setting the stride that the buffer needs to take when reading each element from memory
    D3D11_SUBRESOURCE_DATA bufferInitData;
    bufferInitData.pSysMem = particles->data();//Initial data that is getting passed into the buffer
    pd3dDevice->CreateBuffer(&bufferDesc, (particles) ? &bufferInitData : nullptr, &particlesComputeShaderOutput);//Creates the buffer

    // Create SRV - Lets you read from the Buffer created
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = particles->size();
    pd3dDevice->CreateShaderResourceView(*&particlesComputeShaderOutput, &srvDesc, &sdfPixelCalcOutputReadable);//Creates the shader resource view from the buffer so you can read values

    // Create UAV - Lets you write from the compute shader
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements = particles->size();
    pd3dDevice->CreateUnorderedAccessView(*&particlesComputeShaderOutput, &uavDesc, &sdfPixelCalcOutputWritable);//Creates the unordered access view so you can write to the buffer
}

void SDFComputeShader::unbind(ID3D11DeviceContext* dc)
{
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	dc->CSSetShaderResources(0, 1, nullSRV);

	// Unbind output from compute shader
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Disable Compute Shader
	dc->CSSetShader(nullptr, nullptr, 0);
}

