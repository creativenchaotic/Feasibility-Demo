#pragma once

enum class RenderSettings {
	RenderColours, WorldPosition, Normals, SignedDistanceField
};

struct SPHSimulationValues {
	int numParticlesPerAxis = 20;
	int numParticles = numParticlesPerAxis * numParticlesPerAxis * numParticlesPerAxis;
	int particleResolution = 10;
	float particleSpacing = 10.f;
	int particleScale = 1;
	float sizeOfSpawner = 36.7f;
	XMFLOAT3 particlesSpawnCenter = XMFLOAT3(0.f,0.f,0.f);

	float gravity = -10.f;
	float collisionDamping = 0.95f;
	float smoothingRadius = 0.2f;
	float targetDensity = 630.f;
	float pressureMultiplier = 288.f;
	float nearPressureMultiplier = 2.25f;
	float viscosityStrength = 0.001f;
	float edgeForce = 0.0f;
	float edgeForceDst = 0.0f;

	float timeScale = 1.f;
	int iterationsPerFrame = 3;
	float deltaTime;
};

struct SimulationBoundingBox {
	float Front = 0;
	float Back = 100;
	float LeftSide = 0;
	float RightSide = 100;
	float Top = 100;
	float Bottom = 0;
};

struct LightValues {
	XMFLOAT4 lightColour = XMFLOAT4(0.9f, 0.77f, 0.41f, 1.0f);
	XMFLOAT4 lightAmbientColour = XMFLOAT4(0.59, 0.49, 0.33, 1.0f);
	XMFLOAT3 lightPosition = XMFLOAT3(24.359, 11.538, 50.000);
	XMFLOAT3 lightDirection = XMFLOAT3(0.538f, -0.385f, -1.f);
	XMFLOAT4 lightSpecularColour = XMFLOAT4(1.0F, 1.0F, 1.0F, 1.0F);
};

struct PBRMaterialValues {
	float materialRoughness = 0.088f;
	float metallicFactor = 0.039f;
	float baseReflectivity = 0.681f;
};

struct GUISettings {
	bool hideInstructions = true;
	bool displaySPHSimulationParticles = true;
	bool displayWaterSurface = false;
	bool isLightOn = true;
};

struct SDFValues
{
	float blendAmount = 2.0f;
};