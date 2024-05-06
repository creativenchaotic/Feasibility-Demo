#include "SPHSimulationUpdatePositionsFinalPass.h"
#include "SPH_Particle.h"


SPHSimulationUpdatePositionsFinalPass::SPHSimulationUpdatePositionsFinalPass(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
    initShader(L"SPHSim_UpdatePositionsFinal.cso", NULL);
}

SPHSimulationUpdatePositionsFinalPass::~SPHSimulationUpdatePositionsFinalPass()
{
    release();
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

void SPHSimulationUpdatePositionsFinalPass::setSimulationConstants(ID3D11DeviceContext* deviceContext, int numParticlesVal, float gravityVal, float delta, float bounceDamping, float smoothingRadiusVal, float targetDensityVal, float pressureMultiplierVal, float nearPressureMultVal, float viscosity, float edgeForceVal, float edgeForceDistanceVal, float bb_Top, float bb_Bottom, float bb_LeftSide, float bb_rightSide, float bb_Back, float bb_front, XMMATRIX localToWorld, XMMATRIX worldToLocal)
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

    simulationConstPtr->worldToLocal = worldToLocal;
    simulationConstPtr->localToWorld = localToWorld;

    deviceContext->Unmap(simulationConstantsBuffer, 0);
    deviceContext->CSSetConstantBuffers(0, 1, &simulationConstantsBuffer);
}

void SPHSimulationUpdatePositionsFinalPass::setShaderParameters(ID3D11DeviceContext* dc)
{
    dc->CSSetUnorderedAccessViews(0, 1, &particlesOutputWritable, 0);//Same as UAVs
}


void SPHSimulationUpdatePositionsFinalPass::unbind(ID3D11DeviceContext* dc)
{
    ID3D11ShaderResourceView* nullSRV[] = { NULL };
    dc->CSSetShaderResources(0, 1, nullSRV);

    // Unbind output from compute shader
    ID3D11UnorderedAccessView* nullUAV[] = { NULL };
    dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

    ID3D11Buffer* nullBuffer[] = { NULL };
    dc->CSSetConstantBuffers(0, 1, nullBuffer);

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

}

