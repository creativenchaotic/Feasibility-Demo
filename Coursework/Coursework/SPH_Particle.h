#pragma once
#include "SphereMesh.h"

class SPH_Particle :
    public SphereMesh
{
public:
    SPH_Particle(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution, int lsize, float ldensity, float lmass);
    ~SPH_Particle();

    int resolution;
    int size;
    float density;
    float mass;
    XMFLOAT3 position;
    XMFLOAT3 velocity;
    float gravity = 9.8f;
    float bounceDampingFactor;
};

