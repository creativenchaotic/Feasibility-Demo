#include "SPH_Particle.h"

SPH_Particle::SPH_Particle(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution):SphereMesh(device, deviceContext, lresolution)
{
	resolution = 10;
}

SPH_Particle::~SPH_Particle()
{
}

void SPH_Particle::setStartPosition(XMFLOAT3 pos)
{
	particleData.position = pos;
}

void SPH_Particle::setParticleNum(int num)
{
	particleData.particleNum = num;
}
