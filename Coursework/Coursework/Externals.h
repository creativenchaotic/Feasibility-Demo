#pragma once

enum class RenderSettings {
	PBR, WorldPosition, Normals, Intersection
};

enum class RenderSimulationType
{
	Texture3DStaticParticles, Texture3DSPHSimulation, PlainSDFsStatic, PlainSDFsSPHSimulation
};

struct SPHSimulationValues {
	int numParticlesPerAxis = 2;
	int numParticles = numParticlesPerAxis * numParticlesPerAxis * numParticlesPerAxis;
	float gravity = -10.f;
	float deltaTime;

	float collisionDamping = 0.95f;
	float smoothingRadius = 0.2f;
	float targetDensity = 630.f;
	float pressureMultiplier = 288.f;

	float nearPressureMultiplier;
	float viscosityStrength = 0.001f;
	float edgeForce = 0.0f;
	float edgeForceDst = 0.0f;

	XMFLOAT3 sizeOfSpawner = XMFLOAT3(10.f,10.f,10.f);
	XMFLOAT3 particlesSpawnCenter = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3 spawnRotation = XMFLOAT3(0.f,0.f,0.f);

	XMFLOAT3 sizeOfBoundingBox = XMFLOAT3(20.f, 20.f, 20.f);
	XMFLOAT3 boundingBoxRotation = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3 boundingBoxPosition = XMFLOAT3(0.f, 0.f, 0.f);

	XMMATRIX boundingScale = XMMatrixScaling(sizeOfBoundingBox.x, sizeOfBoundingBox.y, sizeOfBoundingBox.z);
	XMMATRIX boundingRot = XMMatrixRotationRollPitchYaw(boundingBoxRotation.x, boundingBoxRotation.y, boundingBoxRotation.z);
	XMMATRIX boundingPos = XMMatrixTranslation(particlesSpawnCenter.x, particlesSpawnCenter.y, particlesSpawnCenter.z);
	XMMATRIX spawnWorldMatrix = boundingRot * boundingScale * boundingPos;

	XMMATRIX invScale = XMMatrixScaling(1.f/ sizeOfBoundingBox.x, 1.f / sizeOfBoundingBox.y, 1.f / sizeOfBoundingBox.z);

	XMMATRIX localToWorld = spawnWorldMatrix;
	XMMATRIX worldToLocal = (boundingPos *-1)*(invScale)*XMMatrixTranspose(boundingRot);

	int particleScale = 1;

};

struct SimulationBoundingBox {
	float Back = 20.f;
	float Front = -Back;
	float LeftSide = -Back;
	float RightSide = Back;
	float Top = Back;
	float Bottom = -Back;
};

struct LightValues {
	XMFLOAT4 lightColour = XMFLOAT4(0.9f, 0.77f, 0.41f, 1.0f);
	XMFLOAT4 lightAmbientColour = XMFLOAT4(0.59, 0.49, 0.33, 1.0f);
	XMFLOAT3 lightPosition = XMFLOAT3(24.359, 11.538, 50.000);
	XMFLOAT3 lightDirection = XMFLOAT3(0.538f, 0.011f, 0.118f);
	XMFLOAT4 lightSpecularColour = XMFLOAT4(1.0F, 1.0F, 1.0F, 1.0F);
};

struct PBRMaterialValues {
	float materialRoughness = 0.106f;
	float metallicFactor = 0.001f;
	float baseReflectivity = 0.515f;
};

struct GUISettings {
	bool hideInstructions = false;
	bool displaySPHSimulationParticles = false;
	bool isLightOn = true;
	bool displaySDFs = true;
};

struct SDFValues
{
	float blendAmount = 16.954f;
	float particleSize = 1.0f;
};

struct SampleWaterValues
{
	XMFLOAT3 waveDirection1 = XMFLOAT3(3.812f,0.0f,-0.638f);
	XMFLOAT3 waveDirection2 = XMFLOAT3(0.12f,0.0f,8.952f);
	XMFLOAT3 waveDirection3 = XMFLOAT3(2.0,0.0,-2.889);
	float amplitude1 = 0.585f;
	float amplitude2 = 0.245f;
	float amplitude3 = 0.447f;
	float frequency1 = 0.358f;
	float frequency2 = 0.199f;
	float frequency3 = 0.139f;
	float waveSpeed1 = 1.519f;
	float waveSpeed2 = 0.19f;
	float waveSpeed3 = 0.19f;
	float steepness = 2.0f;
	bool isSampleWater = false;
	int sampleWaterState = 0;
};