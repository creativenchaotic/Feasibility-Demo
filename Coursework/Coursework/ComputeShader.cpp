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
    renderer->CreateBuffer(&simConstantsBufferDesc, NULL, &simulationConstantsBuffer);
}

void ComputeShader::createOutputUAVs(ID3D11Device* pd3dDevice, int numParticles, std::vector<ParticleData>* particles)//Called each time the number of particles is changed
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
    pd3dDevice->CreateBuffer(&bufferDesc, (particles) ? &bufferInitData : nullptr, &particlesComputeShaderOutput);//Creates the buffer

    // Create SRV - Lets you read from the Buffer created
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = numParticles;
    pd3dDevice->CreateShaderResourceView(*&particlesComputeShaderOutput, &srvDesc, &particlesOutputReadable);//Creates the shader resource view from the buffer so you can read values

    // Create UAV - Lets you write from the compute shader
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements = numParticles;
    pd3dDevice->CreateUnorderedAccessView(*&particlesComputeShaderOutput, &uavDesc, &particlesOutputWritable);//Creates the unordered access view so you can write to the buffer

  /*  createPositionsBuffer(pd3dDevice, numParticles);
    createPredictedPositionsBuffer(pd3dDevice, numParticles);
    createVelocityBuffer(pd3dDevice, numParticles);
    createDensityBuffer(pd3dDevice, numParticles);
    createSpatialIndicesBuffer(pd3dDevice, numParticles);
    createSpatialOffsetsBuffer(pd3dDevice, numParticles);
    */

}

void ComputeShader::createBuffer(ID3D11Device* pd3dDevice, int numParticles, std::vector<ParticleData>* particles)//Pass in the particles Initial data is the initial contents of the buffer
{

    //CREATING BUFFER TO INPUT DATA INTO THE COMPUTE SHADER
    D3D11_BUFFER_DESC bufferDesc;//Creating a buffer description to create the buffer from
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

void ComputeShader::setSimulationConstants(ID3D11DeviceContext* deviceContext, int numParticlesVal, float gravityVal, float delta, float bounceDamping, float smoothingRadiusVal, float targetDensityVal, float pressureMultiplierVal, float nearPressureMultVal, float viscosity, float edgeForceVal, float edgeForceDistanceVal, float bb_Top, float bb_Bottom, float bb_LeftSide, float bb_rightSide, float bb_Back, float bb_front)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    SimulationConstantsBufferType* simulationConstPtr;
    deviceContext->Map(simulationConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    simulationConstPtr = (SimulationConstantsBufferType*)mappedResource.pData;

    simulationConstPtr->numParticles = numParticlesVal;
    simulationConstPtr->gravity = gravityVal;
    simulationConstPtr->deltaTime = delta;
    simulationConstPtr->collisionsDamping = bounceDamping;

    simulationConstPtr->smoothingRadius = smoothingRadiusVal;
    simulationConstPtr->targetDensity = targetDensityVal;
    simulationConstPtr->pressureMultiplier = pressureMultiplierVal;
    simulationConstPtr->nearPressureMultiplier = nearPressureMultVal;

    simulationConstPtr->viscosityStrength = viscosity;
    simulationConstPtr->edgeForce = edgeForceVal;
    simulationConstPtr->edgeForceDst = edgeForceDistanceVal;


    simulationConstPtr->boundingBoxTop = bb_Top;
    simulationConstPtr->boundingBoxBack = bb_Back;
    simulationConstPtr->boundingBoxBottom = bb_Bottom;
    simulationConstPtr->boundingBoxFront = bb_front;
    simulationConstPtr->boundingBoxLeftSide = bb_LeftSide;
    simulationConstPtr->boundingBoxRightSide = bb_rightSide;

    deviceContext->Unmap(simulationConstantsBuffer, 0);
    deviceContext->CSSetConstantBuffers(1, 1, &simulationConstantsBuffer);
}

void ComputeShader::setShaderParameters(ID3D11DeviceContext* dc)
{
	dc->CSSetShaderResources(0, 1, &particlesComputeShaderInputSRV);//same as SRVs  
	dc->CSSetUnorderedAccessViews(0, 1, &particlesOutputWritable, 0);//Same as UAVs
   /* dc->CSSetUnorderedAccessViews(0, 2, &particlePositionBufferUAV, 0);//Same as UAVs
    dc->CSSetUnorderedAccessViews(0, 3, &particlePredictedPositionBufferUAV, 0);//Same as UAVs
    dc->CSSetUnorderedAccessViews(0, 4, &particleVelocityBufferUAV, 0);//Same as UAVs
    dc->CSSetUnorderedAccessViews(0, 5, &particleDensityBufferUAV, 0);//Same as UAVs
    dc->CSSetUnorderedAccessViews(0, 6, &spatialIndicesBufferUAV, 0);//Same as UAVs
    dc->CSSetUnorderedAccessViews(0, 7, &spatialOffsetsBufferUAV, 0);//Same as UAVs*/
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

void ComputeShader::createPositionsBuffer(ID3D11Device* pd3dDevice, int numParticles)
{
    //PARTICLE POSITION BUFFER-----------------------------------------------------------------------
    //Creating a buffer to output the data from the compute shader
    D3D11_BUFFER_DESC positionBufferDesc = {};//Creating a buffer description to create the buffer from
    positionBufferDesc.ByteWidth = numParticles * sizeof(XMFLOAT3);//sizeofT should be the particle data struct. Setting the size of the buffer to whatever amount is needed
    positionBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    positionBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;//Setting how the buffer works
    positionBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    positionBufferDesc.StructureByteStride = sizeof(XMFLOAT3);//make this the size of particle data//Setting the stride that the buffer needs to take when reading each element from memory
    pd3dDevice->CreateBuffer(&positionBufferDesc, nullptr, &particlePositionBuffer);//Creates the buffer

    // Create SRV - Lets you read from the Buffer created
    D3D11_SHADER_RESOURCE_VIEW_DESC positionSrvDesc = {};
    positionSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
    positionSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    positionSrvDesc.Buffer.ElementWidth = numParticles;
    pd3dDevice->CreateShaderResourceView(*&particlePositionBuffer, &positionSrvDesc, &particlePositionBufferSRV);//Creates the shader resource view from the buffer so you can read values

    // Create UAV - Lets you write from the compute shader
    D3D11_UNORDERED_ACCESS_VIEW_DESC positionUavDesc = {};
    positionUavDesc.Format = DXGI_FORMAT_UNKNOWN;
    positionUavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    positionUavDesc.Buffer.NumElements = numParticles;
    pd3dDevice->CreateUnorderedAccessView(*&particlePositionBuffer, &positionUavDesc, &particlePositionBufferUAV);//Creates the unordered access view so you can write to the buffer
}

void ComputeShader::createPredictedPositionsBuffer(ID3D11Device* pd3dDevice, int numParticles)
{
    //PARTICLE PREDICTED POSITION BUFFER-----------------------------------------------------------------------
   //Creating a buffer to output the data from the compute shader
    D3D11_BUFFER_DESC positionPredBufferDesc = {};//Creating a buffer description to create the buffer from
    positionPredBufferDesc.ByteWidth = numParticles * sizeof(XMFLOAT3);//sizeofT should be the particle data struct. Setting the size of the buffer to whatever amount is needed
    positionPredBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    positionPredBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;//Setting how the buffer works
    positionPredBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    positionPredBufferDesc.StructureByteStride = sizeof(XMFLOAT3);//make this the size of particle data//Setting the stride that the buffer needs to take when reading each element from memory
    pd3dDevice->CreateBuffer(&positionPredBufferDesc, nullptr, &particlePredictedPositionBuffer);//Creates the buffer

    // Create SRV - Lets you read from the Buffer created
    D3D11_SHADER_RESOURCE_VIEW_DESC positionPredSrvDesc = {};
    positionPredSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
    positionPredSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    positionPredSrvDesc.Buffer.ElementWidth = numParticles;
    pd3dDevice->CreateShaderResourceView(*&particlePredictedPositionBuffer, &positionPredSrvDesc, &particlePredictedPositionBufferSRV);//Creates the shader resource view from the buffer so you can read values

    // Create UAV - Lets you write from the compute shader
    D3D11_UNORDERED_ACCESS_VIEW_DESC positionPredUavDesc = {};
    positionPredUavDesc.Format = DXGI_FORMAT_UNKNOWN;
    positionPredUavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    positionPredUavDesc.Buffer.NumElements = numParticles;
    pd3dDevice->CreateUnorderedAccessView(*&particlePredictedPositionBuffer, &positionPredUavDesc, &particlePredictedPositionBufferUAV);//Creates the unordered access view so you can write to the buffer
}

void ComputeShader::createVelocityBuffer(ID3D11Device* pd3dDevice, int numParticles)
{
    //PARTICLE VELOCITY BUFFER-----------------------------------------------------------------------
    //Creating a buffer to output the data from the compute shader
    D3D11_BUFFER_DESC velocityBufferDesc = {};//Creating a buffer description to create the buffer from
    velocityBufferDesc.ByteWidth = numParticles * sizeof(XMFLOAT3);//sizeofT should be the particle data struct. Setting the size of the buffer to whatever amount is needed
    velocityBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    velocityBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;//Setting how the buffer works
    velocityBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    velocityBufferDesc.StructureByteStride = sizeof(XMFLOAT3);//make this the size of particle data//Setting the stride that the buffer needs to take when reading each element from memory
    pd3dDevice->CreateBuffer(&velocityBufferDesc, nullptr, &particleVelocityBuffer);//Creates the buffer

    // Create SRV - Lets you read from the Buffer created
    D3D11_SHADER_RESOURCE_VIEW_DESC velocitySrvDesc = {};
    velocitySrvDesc.Format = DXGI_FORMAT_UNKNOWN;
    velocitySrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    velocitySrvDesc.Buffer.ElementWidth = numParticles;
    pd3dDevice->CreateShaderResourceView(*&particleVelocityBuffer, &velocitySrvDesc, &particleVelocityBufferSRV);//Creates the shader resource view from the buffer so you can read values

    // Create UAV - Lets you write from the compute shader
    D3D11_UNORDERED_ACCESS_VIEW_DESC velocityUavDesc = {};
    velocityUavDesc.Format = DXGI_FORMAT_UNKNOWN;
    velocityUavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    velocityUavDesc.Buffer.NumElements = numParticles;
    pd3dDevice->CreateUnorderedAccessView(*&particleVelocityBuffer, &velocityUavDesc, &particleVelocityBufferUAV);//Creates the unordered access view so you can write to the buffer
}

void ComputeShader::createDensityBuffer(ID3D11Device* pd3dDevice, int numParticles)
{
    //PARTICLE DENSITY BUFFER-----------------------------------------------------------------------
    //Creating a buffer to output the data from the compute shader
    D3D11_BUFFER_DESC densityBufferDesc = {};//Creating a buffer description to create the buffer from
    densityBufferDesc.ByteWidth = numParticles * sizeof(XMFLOAT2);//sizeofT should be the particle data struct. Setting the size of the buffer to whatever amount is needed
    densityBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    densityBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;//Setting how the buffer works
    densityBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    densityBufferDesc.StructureByteStride = sizeof(XMFLOAT2);//make this the size of particle data//Setting the stride that the buffer needs to take when reading each element from memory
    pd3dDevice->CreateBuffer(&densityBufferDesc, nullptr, &particleDensityBuffer);//Creates the buffer

    // Create SRV - Lets you read from the Buffer created
    D3D11_SHADER_RESOURCE_VIEW_DESC densitySrvDesc = {};
    densitySrvDesc.Format = DXGI_FORMAT_UNKNOWN;
    densitySrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    densitySrvDesc.Buffer.ElementWidth = numParticles;
    pd3dDevice->CreateShaderResourceView(*&particleDensityBuffer, &densitySrvDesc, &particleDensityBufferSRV);//Creates the shader resource view from the buffer so you can read values

    // Create UAV - Lets you write from the compute shader
    D3D11_UNORDERED_ACCESS_VIEW_DESC densityUavDesc = {};
    densityUavDesc.Format = DXGI_FORMAT_UNKNOWN;
    densityUavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    densityUavDesc.Buffer.NumElements = numParticles;
    pd3dDevice->CreateUnorderedAccessView(*&particleDensityBuffer, &densityUavDesc, &particleDensityBufferUAV);//Creates the unordered access view so you can write to the buffer
}

void ComputeShader::createSpatialIndicesBuffer(ID3D11Device* pd3dDevice, int numParticles)
{
    //SPATIAL INDICES BUFFER-----------------------------------------------------------------------
    //Creating a buffer to output the data from the compute shader
    D3D11_BUFFER_DESC spatialIndicesBufferDesc = {};//Creating a buffer description to create the buffer from
    spatialIndicesBufferDesc.ByteWidth = numParticles * sizeof(XMFLOAT3);//sizeofT should be the particle data struct. Setting the size of the buffer to whatever amount is needed
    spatialIndicesBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    spatialIndicesBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;//Setting how the buffer works
    spatialIndicesBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    spatialIndicesBufferDesc.StructureByteStride = sizeof(XMFLOAT3);//make this the size of particle data//Setting the stride that the buffer needs to take when reading each element from memory
    pd3dDevice->CreateBuffer(&spatialIndicesBufferDesc, nullptr, &spatialIndicesBuffer);//Creates the buffer

    // Create SRV - Lets you read from the Buffer created
    D3D11_SHADER_RESOURCE_VIEW_DESC spatialIndicesSrvDesc = {};
    spatialIndicesSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
    spatialIndicesSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    spatialIndicesSrvDesc.Buffer.ElementWidth = numParticles;
    pd3dDevice->CreateShaderResourceView(*&spatialIndicesBuffer, &spatialIndicesSrvDesc, &spatialIndicesBufferSRV);//Creates the shader resource view from the buffer so you can read values

    // Create UAV - Lets you write from the compute shader
    D3D11_UNORDERED_ACCESS_VIEW_DESC spatialIndicesUavDesc = {};
    spatialIndicesUavDesc.Format = DXGI_FORMAT_UNKNOWN;
    spatialIndicesUavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    spatialIndicesUavDesc.Buffer.NumElements = numParticles;
    pd3dDevice->CreateUnorderedAccessView(*&spatialIndicesBuffer, &spatialIndicesUavDesc, &spatialIndicesBufferUAV);//Creates the unordered access view so you can write to the buffer
}

void ComputeShader::createSpatialOffsetsBuffer(ID3D11Device* pd3dDevice, int numParticles)
{
    //SPATIAL OFFSETS BUFFER-----------------------------------------------------------------------
    //Creating a buffer to output the data from the compute shader
    D3D11_BUFFER_DESC spatialOffsetsBufferDesc = {};//Creating a buffer description to create the buffer from
    spatialOffsetsBufferDesc.ByteWidth = numParticles * sizeof(int);//sizeofT should be the particle data struct. Setting the size of the buffer to whatever amount is needed
    spatialOffsetsBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    spatialOffsetsBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;//Setting how the buffer works
    spatialOffsetsBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    spatialOffsetsBufferDesc.StructureByteStride = sizeof(int);//make this the size of particle data//Setting the stride that the buffer needs to take when reading each element from memory
    pd3dDevice->CreateBuffer(&spatialOffsetsBufferDesc, nullptr, &spatialOffsetsBuffer);//Creates the buffer

    // Create SRV - Lets you read from the Buffer created
    D3D11_SHADER_RESOURCE_VIEW_DESC spatialOffsetsSrvDesc = {};
    spatialOffsetsSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
    spatialOffsetsSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    spatialOffsetsSrvDesc.Buffer.ElementWidth = numParticles;
    pd3dDevice->CreateShaderResourceView(*&spatialOffsetsBuffer, &spatialOffsetsSrvDesc, &spatialOffsetsBufferSRV);//Creates the shader resource view from the buffer so you can read values

    // Create UAV - Lets you write from the compute shader
    D3D11_UNORDERED_ACCESS_VIEW_DESC spatialOffsetsUavDesc = {};
    spatialOffsetsUavDesc.Format = DXGI_FORMAT_UNKNOWN;
    spatialOffsetsUavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    spatialOffsetsUavDesc.Buffer.NumElements = numParticles;
    pd3dDevice->CreateUnorderedAccessView(*&spatialOffsetsBuffer, &spatialOffsetsUavDesc, &spatialOffsetsBufferUAV);//Creates the unordered access view so you can write to the buffer
}
