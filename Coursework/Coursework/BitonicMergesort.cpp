#include "BitonicMergesort.h"

BitonicMergesort::BitonicMergesort(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"ComputeShader.cso", NULL);
}

BitonicMergesort::~BitonicMergesort()
{
}

void BitonicMergesort::initShader(const wchar_t* cfile, const wchar_t* blank)
{
    loadComputeShader(cfile); //load + compile shader files
}

void BitonicMergesort::setShaderParameters(ID3D11DeviceContext* dc)
{
	dc->CSSetUnorderedAccessViews(0, 1, &bitonicMergesortOutputWritable, 0);//Same as UAVs
}

void BitonicMergesort::createOutputUAVs(ID3D11Device* pd3dDevice, int numParticles, std::vector<ParticleData>* particles)
{
    //Creating a buffer to output the data from the compute shader
    D3D11_BUFFER_DESC bufferDesc = {};//Creating a buffer description to create the buffer from
    bufferDesc.ByteWidth = numParticles * sizeof(ParticleData);//sizeofT should be the particle data struct. Setting the size of the buffer to whatever amount is needed
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;//Setting how the buffer works
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(ParticleData);//make this the size of particle data//Setting the stride that the buffer needs to take when reading each element from memory
    D3D11_SUBRESOURCE_DATA bufferInitData;
    bufferInitData.pSysMem = particles->data();//Initial data that is getting passed into the buffer
    pd3dDevice->CreateBuffer(&bufferDesc, (particles) ? &bufferInitData : nullptr, &bitonicMergesortOutput);//Creates the buffer

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

void BitonicMergesort::unbind(ID3D11DeviceContext* dc)
{
	// Unbind output from compute shader
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Disable Compute Shader
	dc->CSSetShader(nullptr, nullptr, 0);
}


