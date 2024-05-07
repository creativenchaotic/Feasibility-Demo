#include "SPHSimulationUpdatePositionsFinalPass.h"
#include "SPH_Particle.h"


SPHSimulationUpdatePositionsFinalPass::SPHSimulationUpdatePositionsFinalPass(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
    initShader(L"SPHSim_UpdatePositionsFinal.cso", NULL);
}

SPHSimulationUpdatePositionsFinalPass::~SPHSimulationUpdatePositionsFinalPass()
{
    release();

    if (simulationConstantsBuffer)
    {
        simulationConstantsBuffer->Release();
        simulationConstantsBuffer = 0;
    }

    // Release the time buffer.
    if (timeBuffer)
    {
        timeBuffer->Release();
        timeBuffer = 0;
    }
}

void SPHSimulationUpdatePositionsFinalPass::initShader(const wchar_t* cfile, const wchar_t* blank)
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

    D3D11_BUFFER_DESC timeBufferDesc;

    // Setup time buffer
    timeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    timeBufferDesc.ByteWidth = sizeof(TimeBufferType);
    timeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    timeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    timeBufferDesc.MiscFlags = 0;
    timeBufferDesc.StructureByteStride = 0;
    renderer->CreateBuffer(&timeBufferDesc, NULL, &timeBuffer);
}

void SPHSimulationUpdatePositionsFinalPass::createOutputUAVs(ID3D11Device* pd3dDevice, int numParticles)//Called each time the number of particles is changed
{
    //Creating a buffer to output the data from the compute shader
    D3D11_BUFFER_DESC bufferDesc = {};//Creating a buffer description to create the buffer from
    bufferDesc.ByteWidth = numParticles * sizeof(ParticleData);//sizeofT should be the particle data struct. Setting the size of the buffer to whatever amount is needed
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;//Setting how the buffer works
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(ParticleData);//make this the size of particle data//Setting the stride that the buffer needs to take when reading each element from memory
    pd3dDevice->CreateBuffer(&bufferDesc, nullptr, &particlesComputeShaderOutput);//Creates the buffer

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

}

void SPHSimulationUpdatePositionsFinalPass::createInitialDataSRV(ID3D11Device* pd3dDevice, std::vector<XMFLOAT4>* particles)
{
    //Creating a buffer to output the data from the compute shader
    D3D11_BUFFER_DESC bufferDesc = {};//Creating a buffer description to create the buffer from
    bufferDesc.ByteWidth = particles->size() * sizeof(XMFLOAT4);//sizeofT should be the particle data struct. Setting the size of the buffer to whatever amount is needed
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;//Setting how the buffer works
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(XMFLOAT4);//make this the size of particle data//Setting the stride that the buffer needs to take when reading each element from memory
    D3D11_SUBRESOURCE_DATA bufferInitData;
    bufferInitData.pSysMem = particles->data();//Initial data that is getting passed into the buffer
    pd3dDevice->CreateBuffer(&bufferDesc, (particles) ? &bufferInitData : nullptr, &particlesInitialData);//Creates the buffer

    // Create SRV - Lets you read from the Buffer created
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = particles->size();
    pd3dDevice->CreateShaderResourceView(*&particlesInitialData, &srvDesc, &particlesInitialDataReadable);//Creates the shader resource view from the buffer so you can read values

}

void SPHSimulationUpdatePositionsFinalPass::setSimulationConstants(ID3D11DeviceContext* deviceContext, int numParticlesVal, float gravityVal, float delta, float bounceDamping, float smoothingRadiusVal, float targetDensityVal, float pressureMultiplierVal, float nearPressureMultVal, float viscosity, float edgeForceVal, float edgeForceDistanceVal, float bb_Top, float bb_Bottom, float bb_LeftSide, float bb_rightSide, float bb_Back, float bb_front, XMMATRIX localToWorld, XMMATRIX worldToLocal, int isSampleWave)
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

    simulationConstPtr->isSampleWave = isSampleWave;

    simulationConstPtr->boundingBoxTop = bb_Top;
    simulationConstPtr->boundingBoxBack = bb_Back;
    simulationConstPtr->boundingBoxBottom = bb_Bottom;
    simulationConstPtr->boundingBoxFront = bb_front;
    simulationConstPtr->boundingBoxLeftSide = bb_LeftSide;
    simulationConstPtr->boundingBoxRightSide = bb_rightSide;

    simulationConstPtr->worldToLocal = worldToLocal;
    simulationConstPtr->localToWorld = localToWorld;

    deviceContext->Unmap(simulationConstantsBuffer, 0);
    deviceContext->CSSetConstantBuffers(0, 1, &simulationConstantsBuffer);
}

void SPHSimulationUpdatePositionsFinalPass::setShaderParameters(ID3D11DeviceContext* dc)
{
    dc->CSSetUnorderedAccessViews(0, 1, &particlesOutputWritable, 0);//Same as UAVs
    dc->CSSetShaderResources(1, 1, &particlesInitialDataReadable);
}


void SPHSimulationUpdatePositionsFinalPass::unbind(ID3D11DeviceContext* dc)
{
    ID3D11ShaderResourceView* nullSRV[] = { NULL };
    dc->CSSetShaderResources(0, 1, nullSRV);
    dc->CSSetShaderResources(1, 1, nullSRV);

    // Unbind output from compute shader
    ID3D11UnorderedAccessView* nullUAV[] = { NULL };
    dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

    ID3D11Buffer* nullBuffer[] = { NULL };
    dc->CSSetConstantBuffers(0, 1, nullBuffer);
    dc->CSSetConstantBuffers(1, 1, nullBuffer);

    // Disable Compute Shader
    dc->CSSetShader(nullptr, nullptr, 0);
}

void SPHSimulationUpdatePositionsFinalPass::setSimulationDataSRV(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* sphSimViscosityPassOutput)
{
    deviceContext->CSSetShaderResources(0, 1, &sphSimViscosityPassOutput);
}

void SPHSimulationUpdatePositionsFinalPass::release()
{

    if(particlesComputeShaderOutput)
    {
        particlesComputeShaderOutput->Release();
        particlesComputeShaderOutput = 0;
    }

    if (particlesOutputReadable)
    {
        particlesOutputReadable->Release();
        particlesOutputReadable = 0;
    }

    if (particlesOutputWritable)
    {
        particlesOutputWritable->Release();
        particlesOutputWritable = 0;
    }

    if (particlesInitialData)
    {
        particlesInitialData->Release();
        particlesInitialData = 0;
    }

    if (particlesInitialDataReadable)
    {
        particlesInitialDataReadable->Release();
        particlesInitialDataReadable = 0;
    }


}



void SPHSimulationUpdatePositionsFinalPass::setWaveParameters(ID3D11DeviceContext* deviceContext, float deltaTime, float ampl, float freq, float speed, XMFLOAT3 direction, float ampl2, float freq2, float speed2, XMFLOAT3 direction2, float ampl3, float freq3, float speed3, XMFLOAT3 direction3, float steepnessFactor, int isSampleWave)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    //Additional
    // Send time data to vertex shader
    TimeBufferType* timePtr;
    deviceContext->Map(timeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    timePtr = (TimeBufferType*)mappedResource.pData;

    timePtr->time = deltaTime;
    timePtr->amplitude1 = ampl;
    timePtr->frequency1 = freq;
    timePtr->speed1 = speed;

    timePtr->direction1 = XMFLOAT4(direction.x, direction.y, direction.z, 0.0f);

    timePtr->amplitude2 = ampl2;
    timePtr->frequency2 = freq2;
    timePtr->speed2 = speed2;
    timePtr->steepnessFactor = steepnessFactor;

    timePtr->direction2 = XMFLOAT4(direction2.x, direction2.y, direction2.z, 0.0f);

    timePtr->amplitude3 = ampl3;
    timePtr->frequency3 = freq3;
    timePtr->speed3 = speed3;

    timePtr->isSampleWave = isSampleWave;

    timePtr->direction3 = XMFLOAT4(direction3.x, direction3.y, direction3.z, 0.0f);

    deviceContext->Unmap(timeBuffer, 0);
    deviceContext->CSSetConstantBuffers(1, 1, &timeBuffer);

}

