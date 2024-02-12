
#include "App1.h"
#include <imGUI/imgui_internal.h>

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
	renderSettings[3] = "Signed Distance Fields";

	currentNumParticles = simulationSettings.numParticles;

	//OBJECTS AND SHADERS------------------------------------------------------------------------------
	// Create Mesh objects
	water = new PlaneMeshTessellated(renderer->getDevice(), renderer->getDeviceContext(), waterPlaneResolution);
	sun = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	spotlightMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	sdfTestSphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());


	//Creating shaders
	waterShader = new WaterShader(renderer->getDevice(), hwnd);
	sunShader = new SunShader(renderer->getDevice(), hwnd);
	sphParticleShader = new SPHShader(renderer->getDevice(), hwnd);
	sphSimulationComputeShaderFirstPass = new ComputeShader(renderer->getDevice(), hwnd);
	sphSimulationComputeShaderSecondPass = new SPHSimulationComputeShaderSecondPass(renderer->getDevice(), hwnd);
	bitonicMergesort = new BitonicMergesort(renderer->getDevice(), hwnd);
	spatialOffsetCalculationComputeShader = new OffsetCalculationComputeShader(renderer->getDevice(), hwnd);
	sdfShader = new SDFTestShader(renderer->getDevice(), hwnd);

	//LIGHTING ---------------------------------------------------------------------------------------
	// Configure directional light
	directionalLight = new Light();
	directionalLight->setDiffuseColour(directionalLightValues.lightColour.x, directionalLightValues.lightColour.y, directionalLightValues.lightColour.z, directionalLightValues.lightColour.w);
	directionalLight->setAmbientColour(directionalLightValues.lightAmbientColour.x, directionalLightValues.lightAmbientColour.y, directionalLightValues.lightAmbientColour.z, directionalLightValues.lightAmbientColour.w);
	directionalLight->setDirection(directionalLightValues.lightDirection.x, directionalLightValues.lightDirection.y, directionalLightValues.lightDirection.z);
	directionalLight->setSpecularColour(directionalLightValues.lightSpecularColour.x, directionalLightValues.lightSpecularColour.y, directionalLightValues.lightSpecularColour.z, directionalLightValues.lightSpecularColour.w);
	directionalLight -> setSpecularPower(30);
	directionalLight->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 600.f);
	directionalLight->setPosition(directionalLightValues.lightPosition.x, directionalLightValues.lightPosition.y, directionalLightValues.lightPosition.z);

	//Configure Spotlight
	spotlight = new Light();
	spotlight->setDiffuseColour(spotlightDiffuse.x, spotlightDiffuse.y, spotlightDiffuse.z, spotlightDiffuse.w);
	spotlight->setAmbientColour(spotlightAmbient.x, spotlightAmbient.y, spotlightAmbient.z, spotlightAmbient.w);
	spotlight->setDirection(spotlightDirection.x,spotlightDirection.y,spotlightDirection.z);
	spotlight->setSpecularColour(1.0f, 1.0f, 1.0f, 1.0f);
	spotlight->setSpecularPower(30);
	spotlight->generateProjectionMatrix(0.1f, 600.f);
	spotlight->setPosition(spotlightPosition.x,spotlightPosition.y,spotlightPosition.z);

	//initialiseSPHParticles();//DO THIS LAST. It initialises the SPH particles and places them in the scene
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

	if (sphParticle) {
		delete sphParticle;
		sphParticle = 0;
	}

	if (sphParticleShader) {
		delete sphParticleShader;
		sphParticleShader = 0;
	}

	if (sphSimulationComputeShaderFirstPass) {
		delete sphSimulationComputeShaderFirstPass;
		sphSimulationComputeShaderFirstPass = 0;
	}

	if (sphSimulationComputeShaderSecondPass) {
		delete sphSimulationComputeShaderSecondPass;
		sphSimulationComputeShaderSecondPass = 0;
	}

	if (bitonicMergesort) {
		delete bitonicMergesort;
		bitonicMergesort = 0;
	}

	if (spatialOffsetCalculationComputeShader) {
		delete spatialOffsetCalculationComputeShader;
		spatialOffsetCalculationComputeShader = 0;
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


void App1::rebuildWaterPlane()//Feasibility Demo water plane that is used just to see what it might look like in the end
{
	delete water;
	water = new PlaneMeshTessellated(renderer->getDevice(), renderer->getDeviceContext(), waterPlaneResolution);
}


void App1::rebuildSPHParticles()//Used for when the number of particles is changed
{
	simulationParticles.clear();

	initialiseSPHParticles();
	
}


void App1::initialiseSPHParticles()	//Setting the positions of the SPH particles when initialising them
{

	//Placing the SPH particles as a grid based on how many particles there should be per axis
	if (currentNumParticles!=0) {
		
		float particleSpacing = simulationSettings.particleSpacing;

		for (int x = 0; x < simulationSettings.numParticlesPerAxis; x++) {
			for (int y = 0; y < simulationSettings.numParticlesPerAxis; y++) {
				for (int z = 0; z < simulationSettings.numParticlesPerAxis; z++) {

					sphParticle = new SPH_Particle(renderer->getDevice(), renderer->getDeviceContext(), simulationSettings.particleResolution, simulationSettings.particleResolution);

					float tx = x / (simulationSettings.numParticlesPerAxis - 1.f);
					float ty = y / (simulationSettings.numParticlesPerAxis - 1.f);
					float tz = z / (simulationSettings.numParticlesPerAxis - 1.f);

					//sizeOfSpawner is the size of the spawn bow that the particles get spawned in
					float px = (tx - 0.5f) * simulationSettings.sizeOfSpawner + simulationSettings.particlesSpawnCenter.x;
					float py = (ty - 0.5f) * simulationSettings.sizeOfSpawner + simulationSettings.particlesSpawnCenter.y;
					float pz = (tz - 0.5f) * simulationSettings.sizeOfSpawner + simulationSettings.particlesSpawnCenter.z;

					sphParticle->setStartPosition(XMFLOAT3(px+20, py+20, pz+20));//Setting the start position of the particles
					simulationParticles.push_back(sphParticle);//Adding the particle created to a vector of SPH particles
					simulationParticlesData.push_back(sphParticle->particleData);//Adding the SPH particle data to a vector of SPH particle datas
				}
			}
		}
	}

	//----------------------------------------------------------
	//CREATING OUTPUT UAVs FOR THE COMPUTE SHADERS
	// 
	//sphSimulationComputeShader->createBuffer(renderer->getDevice(), currentNumParticles, &simulationParticlesData);
	sphSimulationComputeShaderFirstPass->createOutputUAVs(renderer->getDevice(), currentNumParticles, &simulationParticlesData);
	bitonicMergesort->createOutputUAVs(renderer->getDevice(), currentNumParticles);
	spatialOffsetCalculationComputeShader->createOutputUAVs(renderer->getDevice(), currentNumParticles);
	sphSimulationComputeShaderSecondPass->createOutputUAVs(renderer->getDevice(), currentNumParticles);

}

void App1::runSimulationStep(float frameTime)//Used to run the simulation multiple times per frame since Sebastian Lague does it in his simulation
{
	float timeStep = frameTime / simulationSettings.iterationsPerFrame * simulationSettings.timeScale;

	simulationSettings.deltaTime = timeStep;

	for (int i = 0; i < simulationSettings.iterationsPerFrame; i++)
	{
		sphSimulationComputePass();
	}
}

//-------------------------------------------
//Used for the bitonic mergesort
int App1::NextPowerOfTwo(int value)
{
	value -= 1;
	value |= value >> 16;
	value |= value >> 8;
	value |= value >> 4;
	value |= value >> 2;
	value |= value >> 1;
	return value + 1;
}

float App1::logarithm(int x, int base)
{
	float answer;
	answer = log10(x) / log10(base);
	return answer;
}
//-------------------------------------------



void App1::sphSimulationComputePass()//Runs all the compute shaders needed to run the SPH simulation
{
	//SPH SIMULATION FIRST PASS----------------------------------------
	//Sets the output UAV for the compute shader
	sphSimulationComputeShaderFirstPass->setShaderParameters(renderer->getDeviceContext());
	//Sets the SRV for the compute shader
	sphSimulationComputeShaderFirstPass->setSimulationDataSRV(renderer->getDeviceContext(), sphSimulationComputeShaderSecondPass->getComputeShaderOutput());//Output data from the final compute pass of the SPH simulation gets fed back into the start of the simulation
	//Passes simulation values into compute shader buffer
	sphSimulationComputeShaderFirstPass->setSimulationConstants(renderer->getDeviceContext(), currentNumParticles, simulationSettings.gravity, time, simulationSettings.collisionDamping, simulationSettings.smoothingRadius, simulationSettings.targetDensity, simulationSettings.pressureMultiplier, simulationSettings.nearPressureMultiplier, simulationSettings.viscosityStrength, simulationSettings.edgeForce, simulationSettings.edgeForceDst, boundingBox.Top, boundingBox.Bottom, boundingBox.LeftSide, boundingBox.RightSide, boundingBox.Back, boundingBox.Front, isFirstIteration);
	//Dispatches the shader
	sphSimulationComputeShaderFirstPass->compute(renderer->getDeviceContext(), currentNumParticles, 1, 1);
	sphSimulationComputeShaderFirstPass->unbind(renderer->getDeviceContext());

	
	//BITONIC MERGESORT------------------------------------------------
	//Sets the SRV for the compute shader
	bitonicMergesort->setSimulationDataSRV(renderer->getDeviceContext(), sphSimulationComputeShaderFirstPass->getComputeShaderOutput());
	//Sets the output UAV for the compute shader
	//bitonicMergesort->createDebugUAV(renderer->getDevice()); // UAV used for debugging. Contains a small number of unordered numbers to make sure the bitonic mergesort works 
	bitonicMergesort->setShaderParameters(renderer->getDeviceContext());
	//bitonicMergesort->setBitonicMergesortSettings(renderer->getDeviceContext(), currentNumParticles, 0, 0, 0);//Used for debugging to make sure the bitonic mergesort works
	//bitonicMergesort->compute(renderer->getDeviceContext(),currentNumParticles, 1, 1);//Used with line above to make sure that the bitonic mergesort works

	
	// Launch each step of the sorting algorithm (once the previous step is complete)
	// Number of steps = [log2(n) * (log2(n) + 1)] / 2
	// where n = nearest power of 2 that is greater or equal to the number of inputs
	int numStages = (int)logarithm(NextPowerOfTwo(currentNumParticles), 2);

	for (int stageIndex = 0; stageIndex < numStages; stageIndex++)
	{
		for (int stepIndex = 0; stepIndex < stageIndex + 1; stepIndex++)
		{
			int groupWidth = 1 << (stageIndex - stepIndex);
			int groupHeight = 2 * groupWidth - 1;

			//Set the size of the groups and steps in the bitonic mergesort compute shader
			bitonicMergesort->setBitonicMergesortSettings(renderer->getDeviceContext(), currentNumParticles, groupWidth, groupHeight, stepIndex);

			//Run the pair-wise sorting step
			bitonicMergesort->compute(renderer->getDeviceContext(), NextPowerOfTwo(currentNumParticles) / 2, 1, 1);
		}
	}
	
	bitonicMergesort->unbind(renderer->getDeviceContext());
	
	//SPATIAL OFFSET CALCULATION-----------------
	spatialOffsetCalculationComputeShader->setShaderParameters(renderer->getDeviceContext());
	spatialOffsetCalculationComputeShader->setSimulationDataSRV(renderer->getDeviceContext(), bitonicMergesort->getComputeShaderOutput());//Passing otuput from bitonic mergesort to calculating offsets compute shader to do calculations
	spatialOffsetCalculationComputeShader->setOffsetCalculationsSettings(renderer->getDeviceContext(), currentNumParticles);
	spatialOffsetCalculationComputeShader->compute(renderer->getDeviceContext(), currentNumParticles, 1, 1);
	spatialOffsetCalculationComputeShader->unbind(renderer->getDeviceContext());
	
	//SPH SIMULATION SECOND PASS-----------------
	sphSimulationComputeShaderSecondPass->setShaderParameters(renderer->getDeviceContext());
	sphSimulationComputeShaderSecondPass->setSimulationConstants(renderer->getDeviceContext(), currentNumParticles, simulationSettings.gravity, time, simulationSettings.collisionDamping, simulationSettings.smoothingRadius, simulationSettings.targetDensity, simulationSettings.pressureMultiplier, simulationSettings.nearPressureMultiplier, simulationSettings.viscosityStrength, simulationSettings.edgeForce, simulationSettings.edgeForceDst, boundingBox.Top, boundingBox.Bottom, boundingBox.LeftSide, boundingBox.RightSide, boundingBox.Back, boundingBox.Front);
	sphSimulationComputeShaderSecondPass->setSimulationDataSRV(renderer->getDeviceContext(),sphSimulationComputeShaderFirstPass->getComputeShaderOutput(), bitonicMergesort->getComputeShaderOutput(), spatialOffsetCalculationComputeShader->getComputeShaderOutput());//Passing output from bitonic mergesort and calculating offsets compute shader to sph simulation second pass
	sphSimulationComputeShaderSecondPass->compute(renderer->getDeviceContext(), currentNumParticles, 1, 1);
	sphSimulationComputeShaderSecondPass->unbind(renderer->getDeviceContext());

	isFirstIteration = 0.0f;
}

void App1::renderSceneShaders()
{
	// Clear the scene. (default orange colour)
	if (currentRenderSettingForShader != RenderSettings::SignedDistanceField) {
		renderer->beginScene(skyColour.x, skyColour.y, skyColour.z, skyColour.w);
	}
	else
	{
		renderer->beginScene(0, 0, 0, skyColour.w);
	}

	// Generate the view matrix based on the camera's position.
	camera->update();

	//Getting the lookat vector
	viewM = camera->getViewMatrix();
	view = viewM.r[2];
	XMStoreFloat3(&forward, view);



	//Update light values
	directionalLight->setDirection(directionalLightValues.lightDirection.x, directionalLightValues.lightDirection.y, directionalLightValues.lightDirection.z);
	directionalLight->setPosition(directionalLightValues.lightPosition.x, directionalLightValues.lightPosition.y, directionalLightValues.lightPosition.z);
	spotlight->setDirection(spotlightDirection.x, spotlightDirection.y, spotlightDirection.z);
	spotlight->setPosition(spotlightPosition.x, spotlightPosition.y, spotlightPosition.z);


	//Setting lights on and off
	if (guiSettings.isLightOn) {
		directionalLight->setDiffuseColour(directionalLightValues.lightColour.x, directionalLightValues.lightColour.y, directionalLightValues.lightColour.z, directionalLightValues.lightColour.w);
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
	XMMATRIX translateSun = XMMatrixTranslation(directionalLightValues.lightPosition.x, directionalLightValues.lightPosition.y + 10.f, directionalLightValues.lightPosition.z);
	XMMATRIX scaleSun = XMMatrixScaling(3.0f, 3.0f, 3.0f);
	XMMATRIX shadowMapScaleMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX translateSpotlight = XMMatrixTranslation(spotlightPosition.x, spotlightPosition.y, spotlightPosition.z);
	XMMATRIX translateWaterPlane = XMMatrixTranslation(waterTranslationGUI.x, waterTranslationGUI.y, waterTranslationGUI.z);
	XMMATRIX sph_particleScaleMatrix = XMMatrixScaling(simulationSettings.particleScale, simulationSettings.particleScale, simulationSettings.particleScale);
	XMMATRIX translateSDFTestSphere = XMMatrixTranslation(-12, -10, -300);
	/*
	//WATER PLANE-----------------------------------------------------------------------------
	//Currently only used for the feasibility demo to show what a water plane might look like once in the scene and simulating
	if (guiSettings.displayWaterSurface) {
		renderer->setAlphaBlending(true);
		water->sendData(renderer->getDeviceContext());
		waterShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * translateWaterPlane, viewMatrix, projectionMatrix, directionalLightValues.lightSpecularColour, XMFLOAT4(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z, 0.0F), currentRenderSettingForShader);
		waterShader->setWaveParameters(renderer->getDeviceContext(), time, waterAmpl1, waterFreq1, waterSpeed1, waterDirection1, waterAmpl2, waterFreq2, waterSpeed2, waterDirection2, waterAmpl3, waterFreq3, waterSpeed3, waterDirection3, steepness, waterHeight);
		waterShader->setLightingParameters(renderer->getDeviceContext(), directionalLight, spotlight, -1.0f, 1.0f, sizeSpotlight);
		waterShader->setAttenuationFactors(renderer->getDeviceContext(), attenuationValues);
		waterShader->setMaterialValues(renderer->getDeviceContext(), waterMaterial.materialRoughness, waterMaterial.metallicFactor, waterMaterial.baseReflectivity);
		waterShader->render(renderer->getDeviceContext(), water->getIndexCount());
		renderer->setAlphaBlending(false);
	}

	//SPH PARTICLES---------------------------------------------------------------------------
	if (guiSettings.displaySPHSimulationParticles) {
		renderer->setAlphaBlending(true);

		for (int i = 0; i < currentNumParticles; i++) {

			XMMATRIX particlePosMatrix = XMMatrixTranslation(simulationParticles[i]->particleData.startPosition.x, simulationParticles[i]->particleData.startPosition.y, simulationParticles[i]->particleData.startPosition.z);

			simulationParticles[i]->sendData(renderer->getDeviceContext());
			sphParticleShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * sph_particleScaleMatrix * particlePosMatrix, viewMatrix, projectionMatrix, XMFLOAT4(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z, 0.0F), currentRenderSettingForShader);
			sphParticleShader->setLightingParameters(renderer->getDeviceContext(), directionalLight);
			sphParticleShader->setMaterialValues(renderer->getDeviceContext(), waterMaterial.materialRoughness, waterMaterial.metallicFactor, waterMaterial.baseReflectivity);
			sphParticleShader->setSimulationDataSRV(renderer->getDeviceContext(), sphSimulationComputeShaderSecondPass->getComputeShaderOutput());
			sphParticleShader->setParticleIndex(renderer->getDeviceContext(), i);
			sphParticleShader->render(renderer->getDeviceContext(), simulationParticles[i]->getIndexCount());

		}

		renderer->setAlphaBlending(false);
	}*/


	//LIGHTING DEBUG SPHERES-------------------------------------------------------------------
	if (currentRenderSettingForShader!=RenderSettings::SignedDistanceField) {
		if (guiSettings.isLightOn) {
			sun->sendData(renderer->getDeviceContext());
			sunShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * translateSun * scaleSun, viewMatrix, projectionMatrix);
			sunShader->render(renderer->getDeviceContext(), sun->getIndexCount());
		}

		if (isSpotlightOn) {
			spotlightMesh->sendData(renderer->getDeviceContext());
			sunShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * translateSpotlight, viewMatrix, projectionMatrix);
			sunShader->render(renderer->getDeviceContext(), spotlightMesh->getIndexCount());
		}
	}

	//SDF TEST----------------------------------------------------------------------------------
	sdfTestSphere->sendData(renderer->getDeviceContext());
	sdfShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * translateSDFTestSphere, viewMatrix, projectionMatrix, camera->getPosition());
	sdfShader->render(renderer->getDeviceContext(), sdfTestSphere->getIndexCount());


	// Render GUI
	gui();

	// Present the rendered scene to the screen.
	renderer->endScene();
}


//Final scene render
bool App1::render()
{
	//Add delta time
	time += timer->getTime();

	//sphSimulationComputePass();//Runs the SPH simulation compute shaders

	renderSceneShaders();//Renders the actual water simulation in the scene

	return true;
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI

	ImGui::TextWrapped("Student Name: Melina Garcia Ayala\nStudent Number:2003132");
	ImGui::Checkbox("Hide Instructions", &guiSettings.hideInstructions);

	if (!guiSettings.hideInstructions) {
		ImGui::Dummy(ImVec2(0.0f, 10.0f));//Adds spacing
		ImGui::TextWrapped("This is a code sample for the Feasibility Demo of my project 'Evaluating Realistic Water Surface Creation in 3D Water Simulations for Video Games using Smoothed Particle Hydrodynamics (SPH)'");
		ImGui::Dummy(ImVec2(0.0f, 5.0f));
		ImGui::TextWrapped("Camera Controls\nLeft Mouse Button: Rotate Camera\nWASD: Move Camera\nEQ: Raise/Lower Camera");
	}

	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	ImGui::Text("Camera Position: %f, %f, %f", camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
	ImGui::Text("Camera Rotation: %f, %f, %f", camera->getRotation().x, camera->getRotation().y, camera->getRotation().z);
	if (ImGui::Button("Reset Camera Position")) {//Added in case the user gets lost in the scene when moving the camera
		camera->setPosition(64.0f, 38.0f, -96.0f);
		camera->setRotation(11.5f, -28.75f, 0.f);
	}

	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	//------------------------------------------------------------------------
	//RENDER SETTINGS
	if (!guiSettings.hideInstructions) {
		ImGui::TextWrapped("Render Settings used to display different aspects of the scene in different ways. For example: showing the normals of the objects' surfaces or showing where objects are placed in the world.");
	}
	//Selecting a render method in the ImGui window
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

	//Setting the current render method
	if (currentRenderSetting == "Render Colours") {
		currentRenderSettingForShader = RenderSettings::RenderColours;
	}
	else if(currentRenderSetting == "World Position") {
		currentRenderSettingForShader = RenderSettings::WorldPosition;
	}
	else if(currentRenderSetting == "Normals") {
		currentRenderSettingForShader = RenderSettings::Normals;
	}
	else if(currentRenderSetting == "Signed Distance Fields")
	{
		currentRenderSettingForShader = RenderSettings::SignedDistanceField;
	}



	//------------------------------------------------------------------------
	//SKY
	if (ImGui::TreeNode("Sky")) {
		//SkyColour
		ImGui::ColorEdit4("Sky Colour", (float*)&skyColour);

		ImGui::TreePop();
	}

	//------------------------------------------------------------------------
	//SPH
	if (ImGui::TreeNode("Smoothed Particle Hydrodynamics")) {
		if (!guiSettings.hideInstructions) {
			ImGui::TextWrapped("In the final project the SPH simulation should not be visible since the main focus of the project is the generation of the surface. For this reason there is a toggle to turn the SPH simulation rendering on or off. I added the possibility of still rendering it so that the user can ensure that the simulation is working correctly.");
			ImGui::Dummy(ImVec2(0.0f, 10.0f));
		}

		//Changing the number of particles in the simulation
		ImGui::Checkbox("Display SPH simulation", &guiSettings.displaySPHSimulationParticles);
		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		if (!guiSettings.hideInstructions) {
			ImGui::TextWrapped("For changes to values in the simulation to be applied, press the Rebuild Simulation button");
		}
		//Boudning box for the simulation
		if (ImGui::Button("Rebuild SPH Simulation")) {
			currentNumParticles = simulationSettings.numParticles;
			rebuildSPHParticles();
		}
		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::SliderInt("Number of Particles per Axis", &simulationSettings.numParticlesPerAxis, 1, 100);
		simulationSettings.numParticles = simulationSettings.numParticlesPerAxis * simulationSettings.numParticlesPerAxis * simulationSettings.numParticlesPerAxis;
		ImGui::Text("Total Number of Particles: %i", simulationSettings.numParticles);


		//Spacing between particles and resolution
		ImGui::SliderFloat("Particle Spacing", &simulationSettings.particleSpacing, 0, 20);
		ImGui::SliderInt("Particle Resolution",&simulationSettings.particleResolution,4, 10);
		ImGui::SliderInt("Particle Size", &simulationSettings.particleScale, 1, 100);
		ImGui::SliderFloat3("Particles Spawnpoint Centre", (float*) & simulationSettings.particlesSpawnCenter, -10,10);
		ImGui::SliderFloat("Spawnpoint Size", &simulationSettings.sizeOfSpawner, 2, 100);

		ImGui::SliderFloat("Gravity", &simulationSettings.gravity, -20, 20);
		ImGui::SliderFloat("Collision Damping", &simulationSettings.collisionDamping, 0, 3);
		ImGui::SliderFloat("Smoothing Radius", &simulationSettings.smoothingRadius, 0, 1);
		ImGui::SliderFloat("Target Density", &simulationSettings.targetDensity, 0, 1000);
		ImGui::SliderFloat("Pressure Multiplier", &simulationSettings.pressureMultiplier, 0, 500);
		ImGui::SliderFloat("Near Pressure Multiplier", &simulationSettings.nearPressureMultiplier, 0, 20);
		ImGui::SliderFloat("Viscosity Strength", &simulationSettings.viscosityStrength, 0, 1);
		//ImGui::SliderFloat("Edge Force", &simulationSettings.edgeForce, 0, 10);
		//ImGui::SliderFloat("Edge Force Distance", &simulationSettings.edgeForceDst, 0, 10);

		ImGui::Dummy(ImVec2(0.0f,10.0f));

		if (ImGui::TreeNode("Bounding Box for the Simulation")) {
			//Limits in Y-axis
			ImGui::SliderFloat("Bottom of Bounding Box", &boundingBox.Bottom, -100,100);
			ImGui::SliderFloat("Top of Bounding Box", &boundingBox.Top, -100, 100);
			//Limits on X-axis
			ImGui::SliderFloat("Left Side of Bounding Box", &boundingBox.LeftSide, -100, 100);
			ImGui::SliderFloat("Right Side of Bounding Box", &boundingBox.RightSide, -100, 100);
			//Limits on Z-axis
			ImGui::SliderFloat("Front of Bounding Box", &boundingBox.Front, -100, 100);
			ImGui::SliderFloat("Back of Bounding Box", &boundingBox.Back, -100, 100);

			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	//------------------------------------------------------------------------
	//WAVES
	if (ImGui::TreeNode("Water")) {
		ImGui::Checkbox("Display Water Plane", &guiSettings.displayWaterSurface);
		if (ImGui::TreeNode("Water Manipulation")) {
			if (!guiSettings.hideInstructions) {
				ImGui::TextWrapped("Currently the water plane is used to visualise what the final water surface might look like once the surface generation is implemented for the SPH. Currently, there is a 2D water simulation using Gerstner Waves. In the final project the plane will adapt to the surface of the SPH, for now it is just implemented with Gerstner Waves to help aid visually.");
			}
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
			ImGui::Dummy(ImVec2(0.0f, 10.0f));
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Water Material")) {
			if (!guiSettings.hideInstructions) {
				ImGui::TextWrapped("The water lighting is done using Physically Based Rendering. The values for rendering the material can be changed ");
			}
			ImGui::SliderFloat("Water Roughness", &waterMaterial.materialRoughness, 0.001, 1);
			ImGui::SliderFloat("Water Metallic Amount", &waterMaterial.metallicFactor, 0.001, 1);
			ImGui::SliderFloat("Water Base Reflectivity", &waterMaterial.baseReflectivity, 0.001, 1);
			ImGui::Dummy(ImVec2(0.0f, 10.0f));
			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	//------------------------------------------------------------------------
	//LIGHTS
	if (ImGui::TreeNode("Lights")) {

		ImGui::Text("DIRECTIONAL LIGHT");
		//Directional Light
		ImGui::ColorEdit4("Directional Light Diffuse Colour", (float*)&directionalLightValues.lightColour);
		/*ImGui::ColorEdit4("Directional Light Ambient Colour", (float*)&directionalLightAmbientColour);*/
		ImGui::SliderFloat3("Directional Light Position", (float*)&directionalLightValues.lightPosition, -50,50);
		ImGui::SliderFloat3("Directional Light Direction", (float*)&directionalLightValues.lightDirection, -1.f, 1.f);
		ImGui::Checkbox("Turn on directional light", &guiSettings.isLightOn);

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