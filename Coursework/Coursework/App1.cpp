
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Initalise scene variables.
	renderSettings[0] = "Render Colours";
	renderSettings[1] = "World Position";
	renderSettings[2] = "Normals";


	//OBJECTS AND SHADERS------------------------------------------------------------------------------
	// Create Mesh object and shader object
	water = new PlaneMeshTessellated(renderer->getDevice(), renderer->getDeviceContext(), waterPlaneResolution);
	sun = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	spotlightMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());

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

}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.---------------

	if (water)
	{
		delete water;
		water = 0;
	}

	if (sun) {
		delete sun;
		sun = 0;
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

void App1::rebuildWaterPlane()
{
	delete water;
	water = new PlaneMeshTessellated(renderer->getDevice(), renderer->getDeviceContext(), waterPlaneResolution);
}

//Final scene render
bool App1::render()
{
	// Clear the scene. (default orange colour)
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
	XMMATRIX translateWaterPlane = XMMatrixTranslation(waterTranslationGUI.x, waterTranslationGUI.y, waterTranslationGUI.z);

	renderer->setAlphaBlending(true);
	water->sendData(renderer->getDeviceContext());

	waterShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix*translateWaterPlane, viewMatrix, projectionMatrix, waterSpecular, XMFLOAT4(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z, 0.0F), currentRenderSettingForShader);
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
	ImGui::Text("Camera Position: %f, %f, %f", camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
	ImGui::Text("Camera Rotation: %f, %f, %f", camera->getRotation().x, camera->getRotation().y, camera->getRotation().z);

	//------------------------------------------------------------------------
	//RENDER SETTINGS
	if (ImGui::BeginCombo("Rendering Settings", currentRenderSetting)) {
		for (int i = 0; i < IM_ARRAYSIZE(renderSettings); i++) {

			bool isSelected = (currentRenderSetting == renderSettings[i]);

			if (ImGui::Selectable(renderSettings[i], isSelected)) {
				currentRenderSetting = renderSettings[i];
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();

	}

	if (currentRenderSetting == "Render Colours") {
		currentRenderSettingForShader = RenderSettings::RenderColours;
	}
	else if(currentRenderSetting == "World Position") {
		currentRenderSettingForShader = RenderSettings::WorldPosition;
	}
	else if(currentRenderSetting == "Normals") {
		currentRenderSettingForShader = RenderSettings::Normals;
	}



	//------------------------------------------------------------------------
	//SKY
	if (ImGui::TreeNode("Sky")) {
		//SkyColour
		ImGui::ColorEdit4("Sky Colour", (float*)&skyColour);

		ImGui::TreePop();
	}

	//------------------------------------------------------------------------
	//WAVES
	if (ImGui::TreeNode("Water")) {
		if (ImGui::TreeNode("Water Manipulation")) {
			//To manipulate terrain with waves
			ImGui::SliderInt("Plane resolution", &waterPlaneResolution, 10, 1000);
			if (ImGui::Button("Rebuild Water")) {
				rebuildWaterPlane();
			}
			ImGui::SliderFloat3("Translate Water Plane", (float*) & waterTranslationGUI, -100.f, 100.f);
			ImGui::SliderFloat("Wave Steepness", &steepness, 0.00f, 2.f);
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

	//------------------------------------------------------------------------
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


