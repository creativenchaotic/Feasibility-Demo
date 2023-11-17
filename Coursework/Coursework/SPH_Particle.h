#pragma once
#include "SphereMesh.h"

class SPH_Particle :
    public SphereMesh
{
public:
    SPH_Particle(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution, int lsize, float ldensity, float lmass);
    ~SPH_Particle();


protected:
    int resolution;
    int size;
    float density;
    float mass;
};

