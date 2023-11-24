#pragma once
#include "SphereMesh.h"

struct ParticleData {
    int size;
    XMFLOAT3 padding = XMFLOAT3(0.f,0.f,0.f);
    XMFLOAT3 startPosition;
    float density;
    XMFLOAT3 currentPosition;
    float mass;
    XMFLOAT3 velocity;
    float bounceDampingFactor;
};

class SPH_Particle :
    public SphereMesh
{
public:
    SPH_Particle(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution, int lsize, float ldensity, float lmass);
    ~SPH_Particle();

    void setStartPosition(XMFLOAT3 pos);

    ParticleData particleData;
    int resolution;
};

