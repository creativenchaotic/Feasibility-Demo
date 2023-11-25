#pragma once
#include "SphereMesh.h"

struct ParticleData {
    int size;
    XMFLOAT3 padding = XMFLOAT3(0.f,0.f,0.f);
    XMFLOAT3 startPosition = XMFLOAT3(0.0f,0.0f,0.0f);
    float density = 0.0f;
    XMFLOAT3 currentPosition = XMFLOAT3(0.0f,0.0f,0.0f);
    float mass = 0.0f;
    XMFLOAT3 velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);;
    float bounceDampingFactor =0.0f;
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

