// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"
#include "WaterShader.h"
#include "SunShader.h"
#include "PlaneMeshTessellated.h"
#include "DepthShader.h"

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

private:

	//----------------------------------------------------------------

	//Values to get the camera's forward/lookat vector
	XMVECTOR view;//lookat vector
	XMMATRIX viewM;//view matrix
	XMFLOAT3 forward;//used to store the lookat vector as a float3

	//Meshes------------------------------------------------------
	WaterShader* waterShader;
	PlaneMeshTessellated* water;

	SunShader* sunShader;
	SphereMesh* sun;

	SphereMesh* spotlightMesh;
	
	//----------------------------------------------------------------
	//Lighting-------------------------------------------------------
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



};

#endif