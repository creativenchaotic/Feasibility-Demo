#include "ComputeShader.h"

//struct ParticleData; FIX ME

ComputeShader::ComputeShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"ComputeShader.cso", NULL);
}

ComputeShader::~ComputeShader()
{

}

void ComputeShader::initShader(const wchar_t* cfile, const wchar_t* blank)
{
	loadComputeShader(cfile); //load + compile shader files

    D3D11_BUFFER_DESC simConstantsBufferDesc;

    // Setup constant buffer
    simConstantsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    simConstantsBufferDesc.ByteWidth = sizeof(SimulationConstantsBufferType);
    simConstantsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    simConstantsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    simConstantsBufferDesc.MiscFlags = 0;
    simConstantsBufferDesc.StructureByteStride = 0;
    renderer->CreateBuffer(&simConstantsBufferDesc, NULL, &simuationConstantsBuffer);
}

void ComputeShader::createOutputUAV(ID3D11Device* pd3dDevice, int numParticles)//Called each time the number of particles is changed
{
    // Create SB
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = numParticles * sizeof(ParticleData);//sizeofT should be the particle data struct
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(ParticleData);//make this the size of particle data
    pd3dDevice->CreateBuffer(&bufferDesc, nullptr, &particlesComputeShaderOutput);//Creates the buffer

    // Create SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = numParticles;
    pd3dDevice->CreateShaderResourceView(*&particlesComputeShaderOutput, &srvDesc, &particlesOutputReadable);//Creates the shader resource view from the buffer so you can read values

    // Create UAV
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements = numParticles;
    pd3dDevice->CreateUnorderedAccessView(*&particlesComputeShaderOutput, &uavDesc, &particlesOutputWritable);//Creates the unordered access view so you can write to the buffer
}

void ComputeShader::createBuffer(ID3D11Device* pd3dDevice, int numParticles, std::vector<ParticleData>* particles)//Pass in the particles Initial data is the initial contents of the buffer
{
    int size = sizeof(ParticleData);
    //CREATING BUFFER
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));

    bufferDesc.ByteWidth = numParticles * sizeof(ParticleData);//Size of the buffer = numParticles * particle data
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(ParticleData);//Stride being taken in memory to read data
    D3D11_SUBRESOURCE_DATA bufferInitData;
    bufferInitData.pSysMem = particles->data();//Initial data that is getting passed into the buffer
    pd3dDevice->CreateBuffer(&bufferDesc, (particles) ? &bufferInitData : nullptr, &particlesComputeShaderInput);//If particles exist set initial data

   // Create SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = numParticles;
    pd3dDevice->CreateShaderResourceView(*&particlesComputeShaderInput, &srvDesc, &particlesComputeShaderInputSRV);
}

void ComputeShader::setSimulationConstants(ID3D11DeviceContext* deviceContext, float gravityVal, float bounceDamping)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    SimulationConstantsBufferType* simulationConstPtr;
    deviceContext->Map(simuationConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    simulationConstPtr = (SimulationConstantsBufferType*)mappedResource.pData;
    simulationConstPtr->gravity = gravityVal;
    simulationConstPtr->bounceDampingFactor = bounceDamping;

    deviceContext->Unmap(simuationConstantsBuffer, 0);
    deviceContext->CSSetConstantBuffers(1, 1, &simuationConstantsBuffer);
}

void ComputeShader::setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* texture1)
{
	dc->CSSetShaderResources(0, 1, &particlesComputeShaderInputSRV);//same as SRVs  
	dc->CSSetUnorderedAccessViews(0, 1, &particlesOutputWritable, 0);//Same as UAVs
}


void ComputeShader::unbind(ID3D11DeviceContext* dc)
{
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	dc->CSSetShaderResources(0, 1, nullSRV);

	// Unbind output from compute shader
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Disable Compute Shader
	dc->CSSetShader(nullptr, nullptr, 0);
}