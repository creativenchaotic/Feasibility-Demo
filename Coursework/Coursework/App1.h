// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"
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

#include "SDFComputeShader.h"

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

	void rebuildSPHParticles();//Used to rebuild the SPH particles when the number of particles changes
	void initialiseSPHParticles();//Used to initialise the SPH particles: lays them out in a grid
	void sphSimulationComputePass();//Runs compute shaders used in the SPH sim

	void renderSceneShaders(float time);//Used to render the particles and the plane in the scene

	int NextPowerOfTwo(int value);//Used for bitonic mergesort
	float logarithm(int x, int base);//Used for bitonic mergesort


private:

	SDFComputeShader* sdfComputeShader;
	std::vector<XMFLOAT4> particlePositionSampleData;

	//---------------------------------------------------------------
	SDFTestShader* sdfShader;
	PlaneMeshTessellated* sdfSurface;
	SDFValues sdfVal;

	OrthoMesh* orthoMesh;
	RenderTexture* sdfRenderTexture;

	//----------------------------------------------------------------
	//CAMERA----------------------------------------------------------
	//Values to get the camera's forward/lookat vector
	XMVECTOR view;//lookat vector
	XMMATRIX viewM;//view matrix
	XMFLOAT3 forward;//used to store the lookat vector as a float3



	//----------------------------------------------------------------
	//MESHES---------------------------------------------------------
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

	float PI = 3.14159265358979f;


	//Water lighting values
	PBRMaterialValues waterMaterial;

	int sceneWidth = 1200;
	int sceneHeight = 1200;


	//----------------------------------------------------------------
	// Changing background colour
	XMFLOAT4 skyColour = XMFLOAT4(0.92f, 0.57f, 0.26f, 1.0f);

	//delta time
	float time;


	//----------------------------------------------------------------
	//DISPLAYING DIFFERENT RENDERING SETTINGS

	const char* renderSettings[4];
	const char* currentRenderSetting = "Render with PBR";
	RenderSettings currentRenderSettingForShader;

	const char* simRenderType[4];
	const char* currentSimType = "3D Texture with Static Particles";
	RenderSimulationType currentSimTypeRendered;


	int currentNumParticles = simulationSettings.numParticles;


	GUISettings guiSettings;
	float isFirstIteration = 1.0f;

	int windowWidth;
	int windowHeight;
};

#endif