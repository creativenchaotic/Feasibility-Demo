#pragma once
#include "SphereMesh.h"

class SPH_Particle :
    public SphereMesh
{
public:
    SPH_Particle(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution, int lsize, float ldensity, float lmass);
    ~SPH_Particle();

    void setStartPosition(XMFLOAT3 pos);

    int resolution;
    int size;
    float density;
    float mass;
    XMFLOAT3 startPosition;
    XMFLOAT3 currentPosition;
    XMFLOAT3 velocity;
    float gravity = 9.8f;
    float bounceDampingFactor;
};

