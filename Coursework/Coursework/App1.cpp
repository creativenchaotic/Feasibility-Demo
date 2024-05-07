
#include "App1.h"
#include <imGUI/imgui_internal.h>

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	windowWidth = screenWidth;
	windowHeight = screenHeight;

	// Initalise scene variables.
	renderSettings[0] = "Render with PBR";
	renderSettings[1] = "World Position";
	renderSettings[2] = "Normals";
	renderSettings[3] = "Ray-Box Intersection";

	simRenderType[0] = "3D Texture with Static Particles";
	simRenderType[1] = "3D Texture with SPH Particles";
	simRenderType[2] = "SDFs in Pixel Shader with Static Particles";
	simRenderType[3] = "SDFs in Pixel Shader with SPH Particles";

	currentNumParticles = simulationSettings.numParticles;

	//OBJECTS AND SHADERS------------------------------------------------------------------------------
	// Create Mesh objects
	sdfSurface = new PlaneMeshTessellated(renderer->getDevice(), renderer->getDeviceContext(), 2);
	sdfRenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight, 0, 0);


	//Creating shaders
	sphParticleShader = new SPHShader(renderer->getDevice(), hwnd);

	sphSimulationComputeShaderFirstPass = new ComputeShader(renderer->getDevice(), hwnd);
	sphSimulationSpatialHashing = new SPHSimSpatialHashing(renderer->getDevice(), hwnd);
	bitonicMergesort = new BitonicMergesort(renderer->getDevice(), hwnd);
	spatialOffsetCalculationComputeShader = new OffsetCalculationComputeShader(renderer->getDevice(), hwnd);
	sphSimulationComputeShaderSecondPass = new SPHSimulationComputeShaderSecondPass(renderer->getDevice(), hwnd);
	sphSimulationPressurePass = new SPHSimPressureForcePass(renderer->getDevice(), hwnd);
	sphSimViscosityPass = new SPHSimViscosityPass(renderer->getDevice(), hwnd);
	sphFinalPass = new SPHSimulationUpdatePositionsFinalPass(renderer->getDevice(), hwnd);

	sdfShader = new SDFTestShader(renderer->getDevice(), hwnd);
	sdfComputeShader = new SDFComputeShader(renderer->getDevice(), hwnd);

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

	initialiseSPHParticles();//DO THIS LAST. It initialises the SPH particles and places them in the scene
}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.---------------

	if (directionalLight) {
		delete directionalLight;
		directionalLight = 0;
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

	if(sdfShader)
	{
		delete sdfShader;
		sdfShader = 0;
	}

	if(sdfSurface)
	{
		delete sdfSurface;
		sdfSurface = 0;
	}

	if(sdfRenderTexture)
	{
		delete sdfRenderTexture;
		sdfRenderTexture = 0;
	}

	if(sdfComputeShader)
	{
		delete sdfComputeShader;
		sdfComputeShader = 0;
	}

	if(sphFinalPass)
	{
		delete sphFinalPass;
		sphFinalPass = 0;
	}

	if (sphSimViscosityPass)
	{
		delete sphSimViscosityPass;
		sphSimViscosityPass = 0;
	}

	if (sphSimulationPressurePass)
	{
		delete sphSimulationPressurePass;
		sphSimulationPressurePass = 0;
	}

	if(sphSimulationSpatialHashing)
	{
		delete sphSimulationSpatialHashing;
		sphSimulationSpatialHashing = 0;
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


void App1::rebuildSPHParticles()//Used for when the number of particles is changed
{
	simulationParticles.clear();
	simulationParticlesData.clear();
	particlePositionSampleData.clear();

	isFirstIteration = 1.0f;

	sphSimulationComputeShaderFirstPass->release();
	sphSimulationSpatialHashing->release();
	bitonicMergesort->release();
	spatialOffsetCalculationComputeShader->release();
	sphSimulationComputeShaderSecondPass->release();
	sphSimulationPressurePass->release();
	sphSimViscosityPass->release();
	sphFinalPass->release();
	sdfComputeShader->release();


	initialiseSPHParticles();
	
}


void App1::initialiseSPHParticles()	//Setting the positions of the SPH particles when initialising them
{

	//Placing the SPH particles as a grid based on how many particles there should be per axis
	if (currentNumParticles!=0) {

		int currentParticle = 0;

		for (int x = 0; x < simulationSettings.numParticlesPerAxis; x++) {
			for (int y = 0; y < simulationSettings.numParticlesPerAxis; y++) {
				for (int z = 0; z < simulationSettings.numParticlesPerAxis; z++) {

					sphParticle = new SPH_Particle(renderer->getDevice(), renderer->getDeviceContext(), 10);

					float tx = x / (simulationSettings.numParticlesPerAxis - 1.f);
					float ty = y / (simulationSettings.numParticlesPerAxis - 1.f);
					float tz = z / (simulationSettings.numParticlesPerAxis - 1.f);

					//sizeOfSpawner is the size of the spawn bow that the particles get spawned in
					float px = (tx - 0.5f) * simulationSettings.sizeOfSpawner.x + simulationSettings.particlesSpawnCenter.x;
					float py = (ty - 0.5f) * simulationSettings.sizeOfSpawner.y + simulationSettings.particlesSpawnCenter.y;
					float pz = (tz - 0.5f) * simulationSettings.sizeOfSpawner.z + simulationSettings.particlesSpawnCenter.z;

					sphParticle->setParticleNum(currentParticle);
					sphParticle->setStartPosition(XMFLOAT3(px, py, pz));//Setting the start position of the particles
					simulationParticles.push_back(sphParticle);//Adding the particle created to a vector of SPH particles
					simulationParticlesData.push_back(sphParticle->particleData);//Adding the SPH particle data to a vector of SPH particle datas

					particlePositionSampleData.push_back(XMFLOAT4(px, py, pz, 0));//Setting the start position of the particles

					currentParticle++;
				}
			}
		}
	}

	//----------------------------------------------------------
	//CREATING OUTPUT UAVs FOR THE COMPUTE SHADERS
	// 
	//sphSimulationComputeShader->createBuffer(renderer->getDevice(), currentNumParticles, &simulationParticlesData);
	sphSimulationComputeShaderFirstPass->createOutputUAVs(renderer->getDevice(), currentNumParticles, &simulationParticlesData);
	sphSimulationSpatialHashing->createOutputUAVs(renderer->getDevice(), currentNumParticles);
	bitonicMergesort->createOutputUAVs(renderer->getDevice(), currentNumParticles);
	spatialOffsetCalculationComputeShader->createOutputUAVs(renderer->getDevice(), currentNumParticles);
	sphSimulationComputeShaderSecondPass->createOutputUAVs(renderer->getDevice(), currentNumParticles);
	sphSimulationPressurePass->createOutputUAVs(renderer->getDevice(), currentNumParticles);
	sphSimViscosityPass->createOutputUAVs(renderer->getDevice(), currentNumParticles);
	sphFinalPass->createOutputUAVs(renderer->getDevice(), currentNumParticles);
	sphFinalPass->createInitialDataSRV(renderer->getDevice(), &particlePositionSampleData);
	sdfComputeShader->createOutputUAVs(renderer->getDevice(), &particlePositionSampleData);

}

//-------------------------------------------
//Used for the bitonic mergesort
int App1::NextPowerOfTwo(int value)//From Unity's Mathf.NextPowerOfTwo on Github
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
	sphSimulationComputeShaderFirstPass->setSimulationDataSRV(renderer->getDeviceContext(), sphFinalPass->getComputeShaderOutput());//Output data from the final compute pass of the SPH simulation gets fed back into the start of the simulation
	//Passes simulation values into compute shader buffer
	sphSimulationComputeShaderFirstPass->setSimulationConstants(renderer->getDeviceContext(), currentNumParticles, simulationSettings.gravity, time, simulationSettings.collisionDamping, simulationSettings.smoothingRadius, simulationSettings.targetDensity, simulationSettings.pressureMultiplier, simulationSettings.nearPressureMultiplier, simulationSettings.viscosityStrength, simulationSettings.edgeForce, simulationSettings.edgeForceDst, boundingBox.Top, boundingBox.Bottom, boundingBox.LeftSide, boundingBox.RightSide, boundingBox.Back, boundingBox.Front, isFirstIteration, sampleWater.sampleWaterState);
	//Dispatches the shader
	sphSimulationComputeShaderFirstPass->compute(renderer->getDeviceContext(), currentNumParticles, 1, 1);
	sphSimulationComputeShaderFirstPass->unbind(renderer->getDeviceContext());

	//SPATIAL HASHING------------------------------------------------
	sphSimulationSpatialHashing->setShaderParameters(renderer->getDeviceContext());
	sphSimulationSpatialHashing->setSimulationDataSRV(renderer->getDeviceContext(), sphSimulationComputeShaderFirstPass->getComputeShaderOutput());//Passing output from bitonic mergesort and calculating offsets compute shader to sph simulation second pass
	sphSimulationSpatialHashing->setSimulationConstants(renderer->getDeviceContext(), currentNumParticles, simulationSettings.gravity, time, simulationSettings.collisionDamping, simulationSettings.smoothingRadius, simulationSettings.targetDensity, simulationSettings.pressureMultiplier, simulationSettings.nearPressureMultiplier, simulationSettings.viscosityStrength, simulationSettings.edgeForce, simulationSettings.edgeForceDst, boundingBox.Top, boundingBox.Bottom, boundingBox.LeftSide, boundingBox.RightSide, boundingBox.Back, boundingBox.Front, simulationSettings.localToWorld, simulationSettings.worldToLocal, sampleWater.sampleWaterState);
	sphSimulationSpatialHashing->compute(renderer->getDeviceContext(), currentNumParticles, 1, 1);
	sphSimulationSpatialHashing->unbind(renderer->getDeviceContext());
	
	//BITONIC MERGESORT------------------------------------------------
		//Sets the output UAV for the compute shader
	bitonicMergesort->setShaderParameters(renderer->getDeviceContext());
	//Sets the SRV for the compute shader
	bitonicMergesort->setSimulationDataSRV(renderer->getDeviceContext(), sphSimulationSpatialHashing->getComputeShaderOutput());
	
	// Launch each step of the sorting algorithm (once the previous step is complete)
	// Number of steps = [log2(n) * (log2(n) + 1)] / 2
	// where n = nearest power of 2 that is greater or equal to the number of inputs
	int numStages = (int)log2(NextPowerOfTwo(currentNumParticles));

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


	//SPH SIMULATION SECOND PASS (DENSITY CALCULATIONS)-----------------
	sphSimulationComputeShaderSecondPass->setShaderParameters(renderer->getDeviceContext());
	sphSimulationComputeShaderSecondPass->setSimulationConstants(renderer->getDeviceContext(), currentNumParticles, simulationSettings.gravity, time, simulationSettings.collisionDamping, simulationSettings.smoothingRadius, simulationSettings.targetDensity, simulationSettings.pressureMultiplier, simulationSettings.nearPressureMultiplier, simulationSettings.viscosityStrength, simulationSettings.edgeForce, simulationSettings.edgeForceDst, boundingBox.Top, boundingBox.Bottom, boundingBox.LeftSide, boundingBox.RightSide, boundingBox.Back, boundingBox.Front,simulationSettings.localToWorld, simulationSettings.worldToLocal, sampleWater.sampleWaterState);
	sphSimulationComputeShaderSecondPass->setSimulationDataSRV(renderer->getDeviceContext(),sphSimulationSpatialHashing->getComputeShaderOutput(), bitonicMergesort->getComputeShaderOutput(), spatialOffsetCalculationComputeShader->getComputeShaderOutput());//Passing output from bitonic mergesort and calculating offsets compute shader to sph simulation second pass
	sphSimulationComputeShaderSecondPass->compute(renderer->getDeviceContext(), currentNumParticles, 1, 1);
	sphSimulationComputeShaderSecondPass->unbind(renderer->getDeviceContext());

	//PRESSURE FORCE-----------------
	sphSimulationPressurePass->setShaderParameters(renderer->getDeviceContext());
	sphSimulationPressurePass->setSimulationConstants(renderer->getDeviceContext(), currentNumParticles, simulationSettings.gravity, time, simulationSettings.collisionDamping, simulationSettings.smoothingRadius, simulationSettings.targetDensity, simulationSettings.pressureMultiplier, simulationSettings.nearPressureMultiplier, simulationSettings.viscosityStrength, simulationSettings.edgeForce, simulationSettings.edgeForceDst, boundingBox.Top, boundingBox.Bottom, boundingBox.LeftSide, boundingBox.RightSide, boundingBox.Back, boundingBox.Front, simulationSettings.localToWorld, simulationSettings.worldToLocal, sampleWater.sampleWaterState);
	sphSimulationPressurePass->setSimulationDataSRV(renderer->getDeviceContext(), sphSimulationComputeShaderSecondPass->getComputeShaderOutput());
	sphSimulationPressurePass->compute(renderer->getDeviceContext(), currentNumParticles, 1, 1);
	sphSimulationPressurePass->unbind(renderer->getDeviceContext());

	//VISCOSITY-----------------
	sphSimViscosityPass->setShaderParameters(renderer->getDeviceContext());
	sphSimViscosityPass->setSimulationConstants(renderer->getDeviceContext(), currentNumParticles, simulationSettings.gravity, time, simulationSettings.collisionDamping, simulationSettings.smoothingRadius, simulationSettings.targetDensity, simulationSettings.pressureMultiplier, simulationSettings.nearPressureMultiplier, simulationSettings.viscosityStrength, simulationSettings.edgeForce, simulationSettings.edgeForceDst, boundingBox.Top, boundingBox.Bottom, boundingBox.LeftSide, boundingBox.RightSide, boundingBox.Back, boundingBox.Front, simulationSettings.localToWorld, simulationSettings.worldToLocal, sampleWater.sampleWaterState);
	sphSimViscosityPass->setShaderParameters(renderer->getDeviceContext());
	sphSimViscosityPass->setSimulationDataSRV(renderer->getDeviceContext(), sphSimulationPressurePass->getComputeShaderOutput());
	sphSimViscosityPass->compute(renderer->getDeviceContext(), currentNumParticles, 1, 1);
	sphSimViscosityPass->unbind(renderer->getDeviceContext());

	//UPDATE FINAL POSITIONS-----------------
	sphFinalPass->setShaderParameters(renderer->getDeviceContext());
	sphFinalPass->setSimulationConstants(renderer->getDeviceContext(), currentNumParticles, simulationSettings.gravity, time, simulationSettings.collisionDamping, simulationSettings.smoothingRadius, simulationSettings.targetDensity, simulationSettings.pressureMultiplier, simulationSettings.nearPressureMultiplier, simulationSettings.viscosityStrength, simulationSettings.edgeForce, simulationSettings.edgeForceDst, boundingBox.Top, boundingBox.Bottom, boundingBox.LeftSide, boundingBox.RightSide, boundingBox.Back, boundingBox.Front, simulationSettings.localToWorld, simulationSettings.worldToLocal, sampleWater.sampleWaterState);
	sphFinalPass->setShaderParameters(renderer->getDeviceContext());
	sphFinalPass->setSimulationDataSRV(renderer->getDeviceContext(), sphSimViscosityPass->getComputeShaderOutput());
	sphFinalPass->setWaveParameters(renderer->getDeviceContext(), time, sampleWater.amplitude1, sampleWater.frequency1, sampleWater.waveSpeed1, sampleWater.waveDirection1, sampleWater.amplitude2, sampleWater.frequency2, sampleWater.waveSpeed2, sampleWater.waveDirection2, sampleWater.amplitude3, sampleWater.frequency3, sampleWater.waveSpeed3, sampleWater.waveDirection3, sampleWater.steepness, sampleWater.sampleWaterState);
	sphFinalPass->compute(renderer->getDeviceContext(), currentNumParticles, 1, 1);
	sphFinalPass->unbind(renderer->getDeviceContext());

	isFirstIteration = 0.0f;
}

void App1::renderSceneShaders(float time)
{
	// Clear the scene. (default orange colour)
	if (currentRenderSettingForShader != RenderSettings::WorldPosition || currentRenderSettingForShader != RenderSettings::WorldPosition) {
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


	//Setting lights on and off
	if (guiSettings.isLightOn) {
		directionalLight->setDiffuseColour(directionalLightValues.lightColour.x, directionalLightValues.lightColour.y, directionalLightValues.lightColour.z, directionalLightValues.lightColour.w);
	}
	else {
		directionalLight->setDiffuseColour(0, 0, 0, 1);
	}

	
	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();
	XMMATRIX sph_particleScaleMatrix = XMMatrixScaling(simulationSettings.particleScale, simulationSettings.particleScale, simulationSettings.particleScale);

	if (!sampleWater.isSampleWater) {
		//SPH PARTICLES---------------------------------------------------------------------------
		if (guiSettings.displaySPHSimulationParticles) {
			renderer->setAlphaBlending(true);

			for (int i = 0; i < currentNumParticles; i++) {

				XMMATRIX particlePosMatrix = XMMatrixTranslation(simulationParticles[i]->particleData.position.x, simulationParticles[i]->particleData.position.y, simulationParticles[i]->particleData.position.z);

				simulationParticles[i]->sendData(renderer->getDeviceContext());
				sphParticleShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix * sph_particleScaleMatrix * particlePosMatrix, viewMatrix, projectionMatrix, XMFLOAT4(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z, 0.0F), currentRenderSettingForShader);
				sphParticleShader->setLightingParameters(renderer->getDeviceContext(), directionalLight);
				sphParticleShader->setMaterialValues(renderer->getDeviceContext(), waterMaterial.materialRoughness, waterMaterial.metallicFactor, waterMaterial.baseReflectivity);
				sphParticleShader->setSimulationDataSRV(renderer->getDeviceContext(), sphFinalPass->getComputeShaderOutput());
				sphParticleShader->setParticleIndex(renderer->getDeviceContext(), i);
				sphParticleShader->render(renderer->getDeviceContext(), simulationParticles[i]->getIndexCount());
				sphParticleShader->unbind(renderer->getDeviceContext());
			}

			renderer->setAlphaBlending(false);
		}
	}

	if (guiSettings.displaySDFs) {
		//SDF Compute Shader-----------------------------------------------------------------------------------------
		sdfComputeShader->setShaderParameters(renderer->getDeviceContext());
		sdfComputeShader->setBufferConstants(renderer->getDeviceContext(), currentNumParticles, sdfVal.blendAmount, sdfVal.stride, boundingBox.Back, currentSimTypeRendered);
		sdfComputeShader->setSimulationDataSRV(renderer->getDeviceContext(), sphFinalPass->getComputeShaderOutput());
		sdfComputeShader->compute(renderer->getDeviceContext(), 768 / 32, 768 / 32, 768);
		sdfComputeShader->unbind(renderer->getDeviceContext());


		//-----------------------------------------------------------------------------------------------------------


		//SDF TEST----------------------------------------------------------------------------------
		//Set the render target to be the RtT and clear it
		sdfRenderTexture->setRenderTarget(renderer->getDeviceContext());
		sdfRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 1.0f, 1.0f);
		//Setting camera
		camera->update();

		// Reset the render target back to the original back buffer and not the render to texture anymore.
		renderer->setBackBufferRenderTarget();

		//RENDER TO TEXTURE----------------------------------------------------------------------------------
		renderer->setZBuffer(false);
		XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
		XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering

		renderer->setAlphaBlending(true);

		orthoMesh->sendData(renderer->getDeviceContext());
		sdfShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, camera->getPosition(), time, sdfRenderTexture->getShaderResourceView());
		sdfShader->setParticlePositionsSRV(renderer->getDeviceContext(), sdfComputeShader->getComputeShaderOutput(), sdfComputeShader->getTexture3D());
		sdfShader->setSDFParameters(renderer->getDeviceContext(), sdfVal.blendAmount, currentNumParticles, currentRenderSettingForShader, currentSimTypeRendered);
		sdfShader->setLightingParameters(renderer->getDeviceContext(), directionalLight);
		sdfShader->setMaterialValues(renderer->getDeviceContext(), waterMaterial.materialRoughness, waterMaterial.metallicFactor, waterMaterial.baseReflectivity);
		sdfShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
		sdfShader->unbind(renderer->getDeviceContext());

		renderer->setAlphaBlending(false);
	}

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


	sphSimulationComputePass();//Runs the SPH simulation compute shaders

	renderSceneShaders(time);//Renders the actual water simulation in the scene

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
		ImGui::TextWrapped("This is the artefact for my project 'Evaluating Realistic Water Surface Creation in 3D Water Simulations for Video Games using Smoothed Particle Hydrodynamics (SPH)'");
		ImGui::Spacing();
		ImGui::TextWrapped("This artefact aims to construct the surface of the Smoothed Particle Hydrodynamics simulation by using Signed Distance Fields (SDFs). The SDFs were computed in two different ways: using sphere tracing and SDF calculations in the pixel shader, and precomputing the SDFs and storing them in a 3D texture before rendering them using sphere tracing.\n\nTo figure out where in space to render the 3D Texture, a Ray-Box intersection was used. If the sphere tracing ray intersects with the box, the 3D Texture gets rendered.");
		ImGui::Spacing();
		ImGui::TextWrapped("Camera Controls\nWASD: Move Camera\nEQ: Raise/Lower Camera");
	}

	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	ImGui::Text("Camera Position: %f, %f, %f", camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
	ImGui::Text("Camera Rotation: %f, %f, %f", camera->getRotation().x, camera->getRotation().y, camera->getRotation().z);
	if (ImGui::Button("Reset Camera Position")) {//Added in case the user gets lost in the scene when moving the camera
		camera->setPosition(-7.0f, -11.0f, -129.0f);
		camera->setRotation(0.0f, 0.0, 0.0f);
	}

	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	//------------------------------------------------------------------------
	//RENDER SETTINGS


	ImGui::Spacing();
	if (!guiSettings.hideInstructions) {
		ImGui::TextWrapped("Press 'Display SDFs' to select wether the SDFs or the SPH simulation is displayed.\n-Display SDFs: displays the surface generation\nDisplay SPH Simulation: only displays the SPH simulation particles without surface generation");
	}


	ImGui::Checkbox("Display SDFs", &guiSettings.displaySDFs);
	if (guiSettings.displaySDFs == true)
	{
		guiSettings.displaySPHSimulationParticles = false;
	}
	else
	{
		guiSettings.displaySPHSimulationParticles = true;
	}

	if (!sampleWater.isSampleWater) {
		ImGui::Checkbox("Display SPH simulation", &guiSettings.displaySPHSimulationParticles);
		if (guiSettings.displaySDFs == true)
		{
			guiSettings.displaySPHSimulationParticles = false;
		}
		else
		{
			guiSettings.displaySPHSimulationParticles = true;
		}
	}

	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	//Selecting a render method in the ImGui window
	if (!guiSettings.hideInstructions) {
		ImGui::TextWrapped("Render Settings used to display different aspects of the scene in different ways:\n-Render using PBR lighting calculations\n-Render the world-space position of the objects\n-Render the object normals\n-Render the Ray-Box Intersection ");
	}

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
	if (currentRenderSetting == "Render with PBR") {
		currentRenderSettingForShader = RenderSettings::PBR;
	}
	else if(currentRenderSetting == "World Position") {
		currentRenderSettingForShader = RenderSettings::WorldPosition;
	}
	else if(currentRenderSetting == "Normals") {
		currentRenderSettingForShader = RenderSettings::Normals;
	}
	else if(currentRenderSetting == "Ray-Box Intersection")
	{
		currentRenderSettingForShader = RenderSettings::Intersection;
	}

	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	if (guiSettings.displaySDFs) {

		if (!guiSettings.hideInstructions) {
			ImGui::TextWrapped("Different methods of rendering were set up to test the performance of rendering SDFs in different ways.\n-3D Texture with static particles renders the SPH simulation particles using the starting position of the particles\n-3D Texture with SPH particles renders the SDFs using the positions calculated by the SPH simulation\n-The SDF methods do the same as above but without using a 3D Texture");
		}

		//Selecting a render method in the ImGui window
		if (ImGui::BeginCombo("Type of Simulation", currentSimType)) {
			for (int i = 0; i < IM_ARRAYSIZE(simRenderType); i++) {

				bool isSelected = (currentSimType == simRenderType[i]);

				if (ImGui::Selectable(simRenderType[i], isSelected)) {
					currentSimType = simRenderType[i];
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();

		}

	}

	//Setting the current render method
	if (currentSimType == "3D Texture with Static Particles") {
		currentSimTypeRendered = RenderSimulationType::Texture3DStaticParticles;
	}
	else if (currentSimType == "3D Texture with SPH Particles") {
		currentSimTypeRendered = RenderSimulationType::Texture3DSPHSimulation;
	}
	else if (currentSimType == "SDFs in Pixel Shader with Static Particles") {
		currentSimTypeRendered = RenderSimulationType::PlainSDFsStatic;
	}
	else if (currentSimType == "SDFs in Pixel Shader with SPH Particles")
	{
		currentSimTypeRendered = RenderSimulationType::PlainSDFsSPHSimulation;
	}

	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	//------------------------------------------------------------------------
	//SKY
	if (ImGui::TreeNode("Sky")) {
		if (!guiSettings.hideInstructions) {
			ImGui::TextWrapped("Change Sky colour");
		}
		//SkyColour
		ImGui::ColorEdit4("Sky Colour", (float*)&skyColour);

		ImGui::TreePop();
	}

	//SDF---------------------------------------------------------------------
	if(ImGui::TreeNode("Signed Distance Fields"))
	{
		if (!guiSettings.hideInstructions) {
			ImGui::TextWrapped("Amount that the SDF shapes should be blended by using smooth union");
		}
		ImGui::SliderFloat("SDF Blending", &sdfVal.blendAmount, 0.01, 20);

		if (!guiSettings.hideInstructions) {
			ImGui::TextWrapped("Since the SPH simulation does not work correctly (though its still a good test to see how well the surface generation would run while using the simulation since all the equations needed for SPH are being calculated), a sample wave pattern was included to see what the surface generation would look like when combining it with particles which are moving.\nWhen this is toggled the SPH simulation stops running in the background and Gerstner Waves get used instead.");
		}
		ImGui::Checkbox("Use Sample Waves Instead of SPH", &sampleWater.isSampleWater);

		if (sampleWater.isSampleWater) {

			sampleWater.sampleWaterState = 1;

			if (ImGui::TreeNode("Water Manipulation")) {
				//To manipulate terrain with waves
				ImGui::SliderFloat("Wave steepness", &sampleWater.steepness, 0.00f, 2.f);
				ImGui::Text("WAVE 1");
				ImGui::SliderFloat("Amplitude 1", &sampleWater.amplitude1, 0, 10);
				ImGui::SliderFloat("Frequency 1", &sampleWater.frequency1, 0, 3.14);
				ImGui::SliderFloat("Speed 1", &sampleWater.waveSpeed1, 0, 30);
				ImGui::SliderFloat3("Wave 1 Direction (X,Y,Z)", (float*)&sampleWater.waveDirection1, -10.f, 10.f);
				ImGui::Spacing();
				ImGui::Text("WAVE 2");
				ImGui::SliderFloat("Amplitude 2", &sampleWater.amplitude2, 0, 10);
				ImGui::SliderFloat("Frequency 2", &sampleWater.frequency2, 0, 3.14);
				ImGui::SliderFloat("Speed 2", &sampleWater.waveSpeed2, 0, 30);
				ImGui::SliderFloat3("Wave 2 Direction (X,Y,Z)", (float*)&sampleWater.waveDirection2, -10.f, 10.f);
				ImGui::Spacing();
				ImGui::Text("WAVE 3");
				ImGui::SliderFloat("Amplitude 3", &sampleWater.amplitude3, 0, 10);
				ImGui::SliderFloat("Frequency 3", &sampleWater.frequency3, 0, 3.14);
				ImGui::SliderFloat("Speed 3", &sampleWater.waveSpeed3, 0, 30);
				ImGui::SliderFloat3("Wave 3 Direction (X,Y,Z)", (float*)&sampleWater.waveDirection3, -10.f, 10.f);

				ImGui::TreePop();
			}
		}
		else
		{
			sampleWater.sampleWaterState = 0;
		}

		ImGui::TreePop();
	}
	//------------------------------------------------------------------------
	//SPH
	if (ImGui::TreeNode("Smoothed Particle Hydrodynamics")) {

		if (!guiSettings.hideInstructions) {
			ImGui::TextWrapped("For changes to values in the simulation to be applied, press the Rebuild Simulation button");
		}
		//Boudning box for the simulation
		if (ImGui::Button("Rebuild SPH Simulation")) {
			currentNumParticles = simulationSettings.numParticles;
			rebuildSPHParticles();
		}
		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::SliderInt("Number of Particles per Axis", &simulationSettings.numParticlesPerAxis, 2, 20);
		simulationSettings.numParticles = simulationSettings.numParticlesPerAxis * simulationSettings.numParticlesPerAxis * simulationSettings.numParticlesPerAxis;
		ImGui::Text("Total Number of Particles: %i", simulationSettings.numParticles);


		//Spacing between particles and resolution
		//ImGui::SliderInt("Particle Size", &simulationSettings.particleScale, 1, 100);
		//ImGui::SliderFloat3("Particles Spawnpoint Centre", (float*) & simulationSettings.particlesSpawnCenter, -10,10);
		ImGui::SliderFloat3("Spawnpoint Size",(float*)& simulationSettings.sizeOfSpawner, 2, 20);
		ImGui::SliderFloat("Gravity", &simulationSettings.gravity, -20, 20);
		ImGui::SliderFloat("Collision Damping", &simulationSettings.collisionDamping, 0, 3);
		ImGui::SliderFloat("Smoothing Radius", &simulationSettings.smoothingRadius, 0, 1);
		ImGui::SliderFloat("Target Density", &simulationSettings.targetDensity, 0, 1000);
		ImGui::SliderFloat("Pressure Multiplier", &simulationSettings.pressureMultiplier, 0, 500);
		ImGui::SliderFloat("Near Pressure Multiplier", &simulationSettings.nearPressureMultiplier, 0, 20);
		ImGui::SliderFloat("Viscosity Strength", &simulationSettings.viscosityStrength, 0, 1);
		ImGui::SliderFloat("Edge Force", &simulationSettings.edgeForce, 0, 10);
		ImGui::SliderFloat("Edge Force Distance", &simulationSettings.edgeForceDst, 0, 10);

		ImGui::Dummy(ImVec2(0.0f,10.0f));

		ImGui::TreePop();
	}

	//------------------------------------------------------------------------
	//WAVES
		
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

	

	//------------------------------------------------------------------------
	//LIGHTS
	if (ImGui::TreeNode("Lights")) {

		ImGui::Text("DIRECTIONAL LIGHT");
		//Directional Light
		ImGui::ColorEdit4("Directional Light Diffuse Colour", (float*)&directionalLightValues.lightColour);
		/*ImGui::ColorEdit4("Directional Light Ambient Colour", (float*)&directionalLightAmbientColour);*/
		//ImGui::SliderFloat3("Directional Light Position", (float*)&directionalLightValues.lightPosition, -50,50);
		ImGui::SliderFloat3("Directional Light Direction", (float*)&directionalLightValues.lightDirection, -1.f, 1.f);
		ImGui::Checkbox("Turn on directional light", &guiSettings.isLightOn);

		ImGui::TreePop();
	}
	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}