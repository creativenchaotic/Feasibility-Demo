#pragma once

enum class RenderSettings {
	RenderColours, WorldPosition, Normals
};

struct SPHSimulationValues {
	int numParticles;
	int numParticlesPerAxis;
	int particleResolution;
	float particleSpacing;
	int particleScale;

	float gravity = -10.f;
	float collisionDamping = 0.05f;
	float smoothingRadius = 0.2f;
	float targetDensity;
	float pressureMultiplier;
	float nearPressureMultiplier;
	float viscosityStrength;

	float timeScale = 1.f;
	bool fixedTimeStep;
	int iterationsPerFrame;
};

struct LightValues {
	XMFLOAT4 lightColour;
	XMFLOAT4 lightAmbientColour;
	XMFLOAT3 lightPosition;
	XMFLOAT3 lightDirection;
	XMFLOAT4 lightSpecularColour;
};

struct PBRMaterialValues {
	float materialRoughness;
	float metallicFactor;
	float baseReflectivity;
};

struct GUISettings {
	bool hideInstructions = true;
	bool displaySPHSimulationParticles = true;
	bool displayWaterSurface = false;
	bool isLightOn = true;
};