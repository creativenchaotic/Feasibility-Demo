#include "SPH_Particle.h"

SPH_Particle::SPH_Particle(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution, int lsize, float ldensity, float lmass):SphereMesh(device, deviceContext, lresolution)
{
	resolution = lresolution;
	size = lsize;
	density = ldensity;
	mass = lmass;
}

SPH_Particle::~SPH_Particle()
{
}

void SPH_Particle::setStartPosition(XMFLOAT3 pos)
{
	startPosition = pos;
	currentPosition = startPosition;
}
