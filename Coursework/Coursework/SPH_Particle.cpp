#include "SPH_Particle.h"

SPH_Particle::SPH_Particle(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution, int lsize):SphereMesh(device, deviceContext, lresolution)
{
	resolution = lresolution;
	particleData.size = lsize;
}

SPH_Particle::~SPH_Particle()
{
}

void SPH_Particle::setStartPosition(XMFLOAT3 pos)
{
	particleData.startPosition = pos;
	particleData.currentPosition = particleData.startPosition;
}
