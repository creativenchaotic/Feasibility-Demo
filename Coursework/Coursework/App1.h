// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"
#include "WaterShader.h"
#include "SunShader.h"
#include "PlaneMeshTessellated.h"
#include "DepthShader.h"
#include "RenderSettings.h"
#include "SPH_Particle.h"
#include "SPHShader.h"
#include "ComputeShader.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);
	bool frame();
	void rebuildWaterPlane();
	void rebuildSPHParticles();
	void initialiseSPHParticles();
	void sphSimulationComputePass();
	void renderSceneShaders();


protected:
	bool render();
	void gui();

private:

	//----------------------------------------------------------------
	//CAMERA----------------------------------------------------------
	//Values to get the camera's forward/lookat vector
	XMVECTOR view;//lookat vector
	XMMATRIX viewM;//view matrix
	XMFLOAT3 forward;//used to store the lookat vector as a float3



	//----------------------------------------------------------------
	//MESHES----------------------------------------------------------
	//Water Plane
	WaterShader* waterShader;
	PlaneMeshTessellated* water;
	int waterPlaneResolution = 200;

	//----------------------------------------------
	//Sun
	SunShader* sunShader;
	SphereMesh* sun;
	SphereMesh* spotlightMesh;

	//----------------------------------------------
	//SPH
	SPHShader* sphParticleShader;
	ComputeShader* sphSimulationComputeShader;

	//UI Values for SPH
	int sphParticleResolution = 10;
	int numParticles = 10;
	float spacing = 10;
	XMFLOAT2 bb_topAndBottomOfSimulation;//bb refers to Bounding Box
	XMFLOAT2 bb_frontAndBackOfSim;//front and back faces of the bounding box for the simulation
	XMFLOAT2 bb_sidesOfSim;//sides of the bounding box for the simulation
	float gravity = 9.8f;
	float dampingFactor = 0.2f;
	float restDensity = 0.0f;
	float smoothingRadius = 1.f;
	float particleMass = 1.0f;

	std::vector<SPH_Particle*> simulationParticles;
	std::vector<ParticleData> simulationParticlesData;
	SPH_Particle* sphParticle;

	//----------------------------------------------------------------
	//LIGHTING--------------------------------------------------------
	XMFLOAT3 attenuationValues = XMFLOAT3(0.5f,0.125f,0.0f);

	//Directional Light
	Light* directionalLight;
	XMFLOAT4 directionalLightColour = XMFLOAT4(0.9f, 0.77f, 0.41f, 1.0f);
	XMFLOAT4 directionalLightAmbientColour = XMFLOAT4(0.59, 0.49, 0.33, 1.0f);
	XMFLOAT3 directionalLightPosition = XMFLOAT3(24.359, 11.538, 50.000);
	XMFLOAT3 directionalLightDirection = XMFLOAT3(0.538f, -0.385f, -1.f);
	bool isDirectionalLightOn = true;
	float isDirectionalLightOnParam = 1.0f;
	
	//Spotlight
	Light* spotlight;
	XMFLOAT4 spotlightDiffuse = XMFLOAT4(1.0f, 0.f, 1.0, 1.0f);
	XMFLOAT4 spotlightAmbient = XMFLOAT4(0.18, 0.14, 0.14, 1.0f);
	XMFLOAT4 spotlightDirection = XMFLOAT4(0.538,-0.744f, 0.463,0.f);
	XMFLOAT3 spotlightPosition = XMFLOAT3(11.538,29.487,32.927);
	float sizeSpotlight = 0.82f;
	bool isSpotlightOn = false;
	float isSpotlightOnParam = 0.0f;
	float PI = 3.14159265358979f;

	//Specular
	XMFLOAT4 waterSpecular = XMFLOAT4(1.0F,1.0F,1.0F,1.0F);


	//----------------------------------------------------------------
	// Changing background colour
	XMFLOAT4 skyColour = XMFLOAT4(0.92f, 0.57f, 0.26f, 1.0f);

	//delta time
	float time;



	//----------------------------------------------------------------
	//WATER MANIPULATION----------------------------------------------
	//to manipulate water with waves
	float steepness = 2.f;
	float waterHeight = 3.291f;

	float waterAmpl1= 0.585f;
	float waterFreq1= 0.358f;
	float waterSpeed1= 1.519f;
	XMFLOAT3 waterDirection1 = XMFLOAT3(3.012f,0.0f,-0.638);

	float waterAmpl2 = 0.245f;
	float waterFreq2 = 0.199f;
	float waterSpeed2 = 0.190f;
	XMFLOAT3 waterDirection2 = XMFLOAT3(0.12f, 0.0f, 0.952f);

	float waterAmpl3 = 0.447f;
	float waterFreq3 = 0.139f;
	float waterSpeed3 = 0.190;
	XMFLOAT3 waterDirection3 = XMFLOAT3(2.0f, 0.0f, -2.889f);

	//Water lighting values
	float waterRoughness = 0.088f;
	float waterMetallic = 0.039f;
	float waterBaseReflectivity = 0.681f;

	int sceneWidth = 1200;
	int sceneHeight = 1200;

	XMFLOAT3 waterTranslationGUI = XMFLOAT3(-97.647f,-3.529f, -83.721f);



	//----------------------------------------------------------------
	//DISPLAYING DIFFERENT RENDERING SETTINGS

	const char* renderSettings[3];
	const char* currentRenderSetting = "Render Colours";
	RenderSettings currentRenderSettingForShader;

	bool hideInstructions = false;
	bool displaySPHSimulation = true;
	bool displayWaterPlane = false;
	int currentNumParticles = numParticles;
	int particleScale = 1;
	int particlesPerRow;
	int particlesPerColumn;
};

#endif