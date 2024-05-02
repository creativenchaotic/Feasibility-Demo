#pragma once

enum class RenderSettings {
	PBR, WorldPosition, Normals, Intersection
};

enum class RenderSimulationType
{
	Texture3DStaticParticles, Texture3DSPHSimulation, PlainSDFsStatic, PlainSDFsSPHSimulation
};

struct SPHSimulationValues {
	int numParticlesPerAxis = 3;
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
	XMFLOAT3 lightDirection = XMFLOAT3(0.538f, -0.385f, -1.f);
	XMFLOAT4 lightSpecularColour = XMFLOAT4(1.0F, 1.0F, 1.0F, 1.0F);
};

struct PBRMaterialValues {
	float materialRoughness = 0.017f;
	float metallicFactor = 0.001f;
	float baseReflectivity = 1.0f;
};

struct GUISettings {
	bool hideInstructions = true;
	bool displaySPHSimulationParticles = true;
	bool isLightOn = true;
};

struct SDFValues
{
	float blendAmount = 0.1f;
	int stride = 20;
};