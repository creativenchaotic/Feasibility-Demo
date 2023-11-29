#pragma once

enum class RenderSettings {
	RenderColours, WorldPosition, Normals
};

struct SPHSimulationValues {
	int numParticlesPerAxis = 10;
	int numParticles = numParticlesPerAxis * numParticlesPerAxis * numParticlesPerAxis;
	int particleResolution = 10;
	float particleSpacing = 10.f;
	int particleScale = 1;
	float sizeOfSpawner = 3.7f;
	XMFLOAT3 particlesSpawnCenter = XMFLOAT3(0.f,0.f,0.f);

	float gravity = -10.f;
	float collisionDamping = 0.05f;
	float smoothingRadius = 0.2f;
	float targetDensity = 0.0f;
	float pressureMultiplier;
	float nearPressureMultiplier;
	float viscosityStrength;

	float timeScale = 1.f;
	bool fixedTimeStep;
	int iterationsPerFrame;
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