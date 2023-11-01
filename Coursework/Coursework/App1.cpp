
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Initalise scene variables.
	
	//TEXTURES---------------------------------------------------------------------------------------
	// Load textures
	//Heightmaps
	textureMgr->loadTexture(L"map", L"res/height.png");
	textureMgr->loadTexture(L"map1", L"res/height2.png");
	textureMgr->loadTexture(L"cliffHeight",L"res/RockCliffHeight.png");
	textureMgr->loadTexture(L"mossHeight", L"res/MossHeight.png");
	textureMgr->loadTexture(L"snowHeight", L"res/SnowHeight.png");

	//Material Albedo Textures
	textureMgr->loadTexture(L"mossTexture", L"res/MossAlbedo.png");
	textureMgr->loadTexture(L"cliffTexture", L"res/RockCliffAlbedo.png");
	textureMgr->loadTexture(L"snowTexture", L"res/SnowAlbedo.png");

	//Material Roughness Textures
	textureMgr->loadTexture(L"cliffRoughness", L"res/RockCliffRoughness.png");
	textureMgr->loadTexture(L"mossRoughness", L"res/MossRoughness.png");
	textureMgr->loadTexture(L"snowRoughness", L"res/SnowRoughness.png");


	//OBJECTS AND SHADERS------------------------------------------------------------------------------
	// Create Mesh object and shader object
	plane = new PlaneMeshTessellated(renderer->getDevice(), renderer->getDeviceContext());
	water = new PlaneMeshTessellated(renderer->getDevice(), renderer->getDeviceContext());
	sun = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	spotlightMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());

	terrainShader = new TerrainManipulation(renderer->getDevice(), hwnd);
	waterShader = new WaterShader(renderer->getDevice(), hwnd);
	sunShader = new SunShader(renderer->getDevice(), hwnd);

	//LIGHTING ---------------------------------------------------------------------------------------
	// Confirgure directional light
	directionalLight = new Light();
	directionalLight->setDiffuseColour(directionalLightColour.x,directionalLightColour.y,directionalLightColour.z,directionalLightColour.w);
	directionalLight->setAmbientColour(directionalLightAmbientColour.x, directionalLightAmbientColour.y, directionalLightAmbientColour.z, directionalLightAmbientColour.w);
	directionalLight->setDirection(directionalLightDirection.x,directionalLightDirection.y,directionalLightDirection.z);
	directionalLight->setSpecularColour(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLight -> setSpecularPower(30);
	directionalLight->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 600.f);
	directionalLight->setPosition(directionalLightPosition.x, directionalLightPosition.y, directionalLightPosition.z);

	//Configure Spotlight
	spotlight = new Light();
	spotlight->setDiffuseColour(spotlightDiffuse.x, spotlightDiffuse.y, spotlightDiffuse.z, spotlightDiffuse.w);
	spotlight->setAmbientColour(spotlightAmbient.x, spotlightAmbient.y, spotlightAmbient.z, spotlightAmbient.w);
	spotlight->setDirection(spotlightDirection.x,spotlightDirection.y,spotlightDirection.z);
	spotlight->setSpecularColour(1.0f, 1.0f, 1.0f, 1.0f);
	spotlight->setSpecularPower(30);
	spotlight->generateProjectionMatrix(0.1f, 600.f);
	spotlight->setPosition(spotlightPosition.x,spotlightPosition.y,spotlightPosition.z);

	//SHADOWS---------------------------------------------------------------------------------------
	//Depth Map
	depthShader = new DepthShader(renderer->getDevice(), hwnd, false);
	heightmapDirectionalDepthShader = new DepthShader(renderer->getDevice(), hwnd, true);

	//Shadow Map
	directionalShadowMap = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
	spotlightShadowMap = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.---------------
	//Terrain meshes and shader
	if (plane)
	{
		delete plane;
		plane = 0;
	}

	if (water)
	{
		delete water;
		water = 0;
	}

	if (sun) {
		delete sun;
		sun = 0;
	}

	if (terrainShader)
	{
		delete terrainShader;
		terrainShader = 0;
	}

	if (waterShader) {
		delete waterShader;
		waterShader = 0;
	}

	if (directionalLight) {
		delete directionalLight;
		directionalLight = 0;
	}

	if (spotlight) {
		delete spotlight;
		spotlight = 0;
	}

	if (depthShader) {
		delete depthShader;
		depthShader = 0;
	}

	if (directionalShadowMap) {
		delete directionalShadowMap;
		directionalShadowMap = 0;
	}

	if (spotlightShadowMap) {
		delete spotlightShadowMap;
		spotlightShadowMap = 0;
	}

	if (heightmapDirectionalDepthShader){
		delete heightmapDirectionalDepthShader;
		heightmapDirectionalDepthShader = 0;
	}

	if (spotlightMesh) {
		delete spotlightMesh;
		spotlightMesh = 0;
	}
}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

//Directional Light Depth Texture Creation
void App1::directionalLightDepthPass()
{
	// Set the render target to be the render to texture.
	directionalShadowMap->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get the world, view, and projection matrices from the camera and d3d objects.
	directionalLight->generateViewMatrix();
	XMMATRIX lightViewMatrix = directionalLight->getViewMatrix();
	XMMATRIX lightProjectionMatrix = directionalLight->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX terrainScale = XMMatrixScaling(2.0F,2.0F,2.0F);

	plane->sendData(renderer->getDeviceContext());
	heightmapDirectionalDepthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * terrainScale, lightViewMatrix, lightProjectionMatrix );
	heightmapDirectionalDepthShader->render(renderer->getDeviceContext(), plane->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

//Spotlight Depth Texture Creation
void App1::spotlightDepthPass()
{
	// Set the render target to be the render to texture.
	spotlightShadowMap->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get the world, view, and projection matrices from the camera and d3d objects.
	spotlight->generateViewMatrix();
	XMMATRIX lightViewMatrix = spotlight->getViewMatrix();
	XMMATRIX lightProjectionMatrix = spotlight->getProjectionMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX terrainScale = XMMatrixScaling(2.0F, 2.0F, 2.0F);

	plane->sendData(renderer->getDeviceContext());
	heightmapDirectionalDepthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * terrainScale, lightViewMatrix, lightProjectionMatrix);
	heightmapDirectionalDepthShader->render(renderer->getDeviceContext(), plane->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

//Final scene render
bool App1::render()
{
	directionalLightDepthPass();
	spotlightDepthPass();

	// Clear the scene. (default blue colour)
	renderer->beginScene(skyColour.x, skyColour.y, skyColour.z, skyColour.w);

	// Generate the view matrix based on the camera's position.
	camera->update();

	//Getting the lookat vector
	viewM = camera->getViewMatrix();
	view = viewM.r[2];
	XMStoreFloat3(&forward, view);

	//Add delta time
	time += timer->getTime();

	//Update light values
	directionalLight->setDirection(directionalLightDirection.x, directionalLightDirection.y, directionalLightDirection.z);
	directionalLight->setPosition(directionalLightPosition.x, directionalLightPosition.y, directionalLightPosition.z);
	spotlight->setDirection(spotlightDirection.x,spotlightDirection.y,spotlightDirection.z);
	spotlight->setPosition(spotlightPosition.x, spotlightPosition.y, spotlightPosition.z);


	//Setting lights on and off
	if (isDirectionalLightOn) {
		directionalLight->setDiffuseColour(directionalLightColour.x, directionalLightColour.y, directionalLightColour.z, 1);
	}
	else {
		directionalLight->setDiffuseColour(0, 0, 0, 1);
	}
	if (isSpotlightOn) {
		spotlight->setDiffuseColour(spotlightDiffuse.x, spotlightDiffuse.y, spotlightDiffuse.z, 1);
	}
	else {
		spotlight->setDiffuseColour(0, 0, 0, 1);
	}

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();
	XMMATRIX translateSun = XMMatrixTranslation(directionalLightPosition.x, directionalLightPosition.y + 10.f, directionalLightPosition.z);
	XMMATRIX scaleSun = XMMatrixScaling(3.0f,3.0f,3.0f);
	XMMATRIX shadowMapScaleMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX translateSpotlight = XMMatrixTranslation(spotlightPosition.x, spotlightPosition.y, spotlightPosition.z);

	//Pass values into shaders
	plane->sendData(renderer->getDeviceContext());
	if (isMap1) {
		terrainShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"map"), textureMgr->getTexture(L"cliffHeight"), textureMgr->getTexture(L"cliffTexture"), textureMgr->getTexture(L"mossHeight"), textureMgr->getTexture(L"mossTexture"), textureMgr->getTexture(L"snowHeight"), textureMgr->getTexture(L"snowTexture"), textureMgr->getTexture(L"cliffRoughness"), textureMgr->getTexture(L"mossRoughness"), textureMgr->getTexture(L"snowRoughness"), directionalLight, spotlight);
	}
	if (isMap2) {
		terrainShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"map1"), textureMgr->getTexture(L"cliffHeight"), textureMgr->getTexture(L"cliffTexture"), textureMgr->getTexture(L"mossHeight"), textureMgr->getTexture(L"mossTexture"), textureMgr->getTexture(L"snowHeight"), textureMgr->getTexture(L"snowTexture"), textureMgr->getTexture(L"cliffRoughness"), textureMgr->getTexture(L"mossRoughness"), textureMgr->getTexture(L"snowRoughness"), directionalLight, spotlight);
	}
	terrainShader->setLightingParameters(renderer->getDeviceContext(), directionalLight, spotlight, -1.0f, 1.0f, sizeSpotlight);
	terrainShader->setShadowValues(renderer->getDeviceContext(), directionalShadowMap->getDepthMapSRV(), spotlightShadowMap->getDepthMapSRV());
	terrainShader->setAttenuationFactors(renderer->getDeviceContext(), attenuationValues);
	terrainShader->setMaterialValues(renderer->getDeviceContext(), grassMetallic, grassBaseReflectivity, rockMetallic, rockBaseReflectivity, snowMetallic, snowBaseReflectivity, sandMetallic, sandBaseReflectivity);
	terrainShader->render(renderer->getDeviceContext(), plane->getIndexCount());

	renderer->setAlphaBlending(true);
	water->sendData(renderer->getDeviceContext());
	if (isMap1) {
		waterShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"map"), waterSpecular, XMFLOAT4(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z, 0.0F));
	}
	if (isMap2) {
		waterShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"map1"), waterSpecular, XMFLOAT4(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z, 0.0F));
	}
	waterShader->setWaveParameters(renderer->getDeviceContext(), time, waterAmpl1, waterFreq1, waterSpeed1, waterDirection1, waterAmpl2, waterFreq2, waterSpeed2, waterDirection2, waterAmpl3, waterFreq3, waterSpeed3, waterDirection3, steepness, waterHeight);
	waterShader->setLightingParameters(renderer->getDeviceContext(), directionalLight, spotlight, -1.0f, 1.0f, sizeSpotlight);
	waterShader->setAttenuationFactors(renderer->getDeviceContext(), attenuationValues);
	waterShader->setMaterialValues(renderer->getDeviceContext(), waterRoughness, waterMetallic, waterBaseReflectivity);
	waterShader->render(renderer->getDeviceContext(), water->getIndexCount());
	renderer->setAlphaBlending(false);	

	if (isDirectionalLightOn) {
		sun->sendData(renderer->getDeviceContext());
		sunShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * translateSun * scaleSun, viewMatrix, projectionMatrix);
		sunShader->render(renderer->getDeviceContext(), sun->getIndexCount());
	}

	if (isSpotlightOn) {
		spotlightMesh->sendData(renderer->getDeviceContext());
		sunShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * translateSpotlight, viewMatrix, projectionMatrix);
		sunShader->render(renderer->getDeviceContext(), spotlightMesh->getIndexCount());
	}

	// Render GUI
	gui();

	// Present the rendered scene to the screen.
	renderer->endScene();

	return true;
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	//Change map
	//ImGui::Checkbox("Change map", &isDirectionalLightOn);
	
	//SKY
	if (ImGui::TreeNode("Sky")) {
		//SkyColour
		ImGui::ColorEdit4("Sky Colour", (float*)&skyColour);

		ImGui::TreePop();
	}

	//MAP
	if (ImGui::TreeNode("Map")) {
		ImGui::Text("Leave only 1 checked box to activate\nthe heightmap with the check ");

		ImGui::Checkbox("Map 1", &isMap1);

		if (isMap2 && !isMap1) {
			isMap1 = false;
		}
		if (isMap1 && !isMap2) {
			isMap2 = false;
		}

		ImGui::Checkbox("Map 2", &isMap2);

		if (isMap2 && !isMap1) {
			isMap1 = false;
		}
		if (isMap1 && !isMap2) {
			isMap2 = false;
		}

		if (!isMap1 && !isMap2) {
			isMap1 = true;
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Mountain")) {
		if (ImGui::TreeNode("Grass Material")) {

			ImGui::SliderFloat("Grass Metallic Amount", &grassMetallic, 0.001, 1);
			ImGui::SliderFloat("Grass Base Reflectivity", &grassBaseReflectivity, 0.001, 1);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Rock Material")) {

			ImGui::SliderFloat("Rock Metallic Amount", &rockMetallic, 0.001, 1);
			ImGui::SliderFloat("Rock Base Reflectivity", &rockBaseReflectivity, 0.001, 1);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Snow Material")) {

			ImGui::SliderFloat("Snow Metallic Amount", &snowMetallic, 0.001, 1);
			ImGui::SliderFloat("Snow Base Reflectivity", &snowBaseReflectivity, 0.001, 1);

			ImGui::TreePop();
		}

		/*if (ImGui::TreeNode("Sand Material")) {

			ImGui::SliderFloat("Sand Metallic Amount", &sandMetallic, 0.001, 1);
			ImGui::SliderFloat("Sand Base Reflectivity", &sandBaseReflectivity, 0.001, 1);

			ImGui::TreePop();
		}*/

		ImGui::TreePop();
	}


	//WAVES
	if (ImGui::TreeNode("Water")) {
		if (ImGui::TreeNode("Water Manipulation")) {
			//To manipulate terrain with waves
			ImGui::SliderFloat("Wave steepness", &steepness, 0.00f, 2.f);
			ImGui::SliderFloat("Water Height", &waterHeight, 0, 20.f);
			ImGui::Text("WAVE 1");
			ImGui::SliderFloat("Amplitude 1", &waterAmpl1, 0, 10);
			ImGui::SliderFloat("Frequency 1", &waterFreq1, 0, 3.14);
			ImGui::SliderFloat("Speed 1", &waterSpeed1, 0, 30);
			ImGui::SliderFloat3("Wave 1 Direction (X,Y,Z)", (float*)&waterDirection1, -10.f, 10.f);
			ImGui::Spacing();
			ImGui::Text("WAVE 2");
			ImGui::SliderFloat("Amplitude 2", &waterAmpl2, 0, 10);
			ImGui::SliderFloat("Frequency 2", &waterFreq2, 0, 3.14);
			ImGui::SliderFloat("Speed 2", &waterSpeed2, 0, 30);
			ImGui::SliderFloat3("Wave 2 Direction (X,Y,Z)", (float*)&waterDirection2, -10.f, 10.f);
			ImGui::Spacing();
			ImGui::Text("WAVE 3");
			ImGui::SliderFloat("Amplitude 3", &waterAmpl3, 0, 10);
			ImGui::SliderFloat("Frequency 3", &waterFreq3, 0, 3.14);
			ImGui::SliderFloat("Speed 3", &waterSpeed3, 0, 30);
			ImGui::SliderFloat3("Wave 3 Direction (X,Y,Z)", (float*)&waterDirection3, -10.f, 10.f);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Water Material")) {
			ImGui::SliderFloat("Water Roughness", &waterRoughness, 0.001, 1);
			ImGui::SliderFloat("Water Metallic Amount", &waterMetallic, 0.001, 1);
			ImGui::SliderFloat("Water Base Reflectivity", &waterBaseReflectivity, 0.001, 1);

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	//LIGHTS
	if (ImGui::TreeNode("Lights")) {

		ImGui::Text("DIRECTIONAL LIGHT");
		//Directional Light
		ImGui::ColorEdit4("Directional Light Diffuse Colour", (float*)&directionalLightColour);
		/*ImGui::ColorEdit4("Directional Light Ambient Colour", (float*)&directionalLightAmbientColour);*/
		ImGui::SliderFloat3("Directional Light Position", (float*)&directionalLightPosition, -50,50);
		ImGui::SliderFloat3("Directional Light Direction", (float*)&directionalLightDirection, -1.f, 1.f);
		ImGui::Checkbox("Turn on directional light", &isDirectionalLightOn);

		ImGui::Spacing();
		ImGui::Text("SPOTLIGHT");
		//Spotlight
		ImGui::ColorEdit4("Spotlight Diffuse Colour", (float*)&spotlightDiffuse);
		/*ImGui::ColorEdit4("Spotlight Ambient Colour", (float*)&spotlightAmbient);*/
		ImGui::SliderFloat3("Spotlight Position", (float*)&spotlightPosition, -50.f, 50.f);
		ImGui::SliderFloat3("Spotlight Direction", (float*)&spotlightDirection, -1.f, 1.f);
		ImGui::SliderFloat("Size Spotlight", &sizeSpotlight, 0, 1.57);
		ImGui::Checkbox("Turn on Spotlight", &isSpotlightOn);

		//Attenuation values
		ImGui::SliderFloat("Constant Factor", &attenuationValues.x, 0, 1);
		ImGui::SliderFloat("Linear Factor", &attenuationValues.y, 0, 1);
		ImGui::SliderFloat("Quadratic Factor", &attenuationValues.z, 0, 1);

		ImGui::TreePop();
	}
	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}


