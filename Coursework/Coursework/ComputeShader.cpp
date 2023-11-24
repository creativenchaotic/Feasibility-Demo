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
	loadComputeShader(cfile);
}

void ComputeShader::createOutputUAV(ID3D11Device* pd3dDevice, int numParticles)//Pass in the particles Initial data is the initial contents of the buffer
{
    // Create SB
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = numParticles * sizeof(ParticleData);//sizeofT should be the particle data struct
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(ParticleData);//make this the size of particle data

    pd3dDevice->CreateBuffer(&bufferDesc, nullptr, &particlesComputeShaderOutput);

    // Create SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = numParticles;
    pd3dDevice->CreateShaderResourceView(*&particlesComputeShaderOutput, &srvDesc, &particlesOutputReadable);

    // Create UAV
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements = numParticles;
    pd3dDevice->CreateUnorderedAccessView(*&particlesComputeShaderOutput, &uavDesc, &particlesOutputWritable);
}

void ComputeShader::createBuffer(ID3D11Device* pd3dDevice, int numParticles, std::vector<ParticleData>* particles)
{
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = numParticles * sizeof(ParticleData);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(ParticleData);

    D3D11_SUBRESOURCE_DATA bufferInitData = {};
    bufferInitData.pSysMem = particles;
    pd3dDevice->CreateBuffer(&bufferDesc, (particles) ? &bufferInitData : nullptr, &particlesComputeShaderInput);

    // Create SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = numParticles;
    pd3dDevice->CreateShaderResourceView(*&particlesComputeShaderInput, &srvDesc, &particlesComputeShaderInputSRV);
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