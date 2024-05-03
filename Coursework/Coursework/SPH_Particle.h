#pragma once
#include "SphereMesh.h"

struct uint3 {
    uint32_t index = 0;
    uint32_t hash = 0;
    uint32_t key = 0;
};

struct ParticleData {
    int particleNum = 0;

    XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
    XMFLOAT3 predictedPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
    XMFLOAT3 velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
    XMFLOAT2 density = XMFLOAT2(0.0,0.0);
    uint3 spatialIndices;
    uint32_t spatialOffsets = 0;
};

class SPH_Particle :
    public SphereMesh
{
public:
    SPH_Particle(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution);
    ~SPH_Particle();

    void setStartPosition(XMFLOAT3 pos);
    void setParticleNum(int num);

    ParticleData particleData;
    int resolution;
};

