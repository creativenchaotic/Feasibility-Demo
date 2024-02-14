// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"
#include "WaterShader.h"
#include "SunShader.h"
#include "PlaneMeshTessellated.h"
#include "Externals.h"
#include "SPH_Particle.h"
#include "SPHShader.h"
#include "ComputeShader.h"
#include "SPHSimulationComputeShaderSecondPass.h"
#include "BitonicMergesort.h"
#include "OffsetCalculationComputeShader.h"
#include "SDFTestShader.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();

	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);
	bool frame();
	
protected:
	bool render();
	void gui();

	void rebuildWaterPlane();//Used to rebuild the water plane on top of the particles
	void rebuildSPHParticles();//Used to rebuild the SPH particles when the number of particles changes
	void initialiseSPHParticles();//Used to initialise the SPH particles: lays them out in a grid
	void sphSimulationComputePass();//Runs compute shaders used in the SPH sim

	void renderSceneShaders();//Used to render the particles and the plane in the scene

	int NextPowerOfTwo(int value);//Used for bitonic mergesort
	float logarithm(int x, int base);//Used for bitonic mergesort

	void runSimulationStep(float frameTime);//Sebastian Lague runs his simulation multiple times per frame so I included the function to have it just in case


private:

	SDFTestShader* sdfShader;
	PlaneMeshTessellated* sdfSurface;

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
	SPHShader* sphParticleShader;//Used to render the particles in the scene
	ComputeShader* sphSimulationComputeShaderFirstPass;//Initial SPH sim compute shader pass
	SPHSimulationComputeShaderSecondPass* sphSimulationComputeShaderSecondPass;//Second SPH sim compute shader pass
	BitonicMergesort* bitonicMergesort;
	OffsetCalculationComputeShader* spatialOffsetCalculationComputeShader;

	//UI Values for SPH
	SPHSimulationValues simulationSettings;

	SimulationBoundingBox boundingBox;

	std::vector<SPH_Particle*> simulationParticles;
	std::vector<ParticleData> simulationParticlesData;
	SPH_Particle* sphParticle;

	//----------------------------------------------------------------
	//LIGHTING--------------------------------------------------------
	XMFLOAT3 attenuationValues = XMFLOAT3(0.5f,0.125f,0.0f);

	//Directional Light
	Light* directionalLight;
	LightValues directionalLightValues;
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



	//----------------------------------------------------------------
	// Changing background colour
	XMFLOAT4 skyColour = XMFLOAT4(0.92f, 0.57f, 0.26f, 1.0f);

	//delta time
	float time;



	//----------------------------------------------------------------
	//WATER MANIPULATION----------------------------------------------
	//to manipulate water with waves
	//Only used on the plane for the feasibility demo. Not to be used for the final water surface
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
	PBRMaterialValues waterMaterial;

	int sceneWidth = 1200;
	int sceneHeight = 1200;

	XMFLOAT3 waterTranslationGUI = XMFLOAT3(-97.647f,-3.529f, -83.721f);



	//----------------------------------------------------------------
	//DISPLAYING DIFFERENT RENDERING SETTINGS

	const char* renderSettings[4];
	const char* currentRenderSetting = "Render Colours";
	RenderSettings currentRenderSettingForShader;


	int currentNumParticles = simulationSettings.numParticles;


	GUISettings guiSettings;
	float isFirstIteration = 1.0f;

	int windowWidth;
	int windowHeight;
};

#endif