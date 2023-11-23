#include "SPH_Particle.h"

SPH_Particle::SPH_Particle(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution, int lsize, float ldensity, float lmass):SphereMesh(device, deviceContext, lresolution)
{
	resolution = lresolution;
	particleData.size = lsize;
	particleData.density = ldensity;
	particleData.mass = lmass;
	particleData.velocity = XMFLOAT3(0.f,0.f,0.f);
}

SPH_Particle::~SPH_Particle()
{
}

void SPH_Particle::setStartPosition(XMFLOAT3 pos)
{
	particleData.startPosition = pos;
	particleData.currentPosition = particleData.startPosition;
}
