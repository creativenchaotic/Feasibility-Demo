#include "WaterShader.h"


WaterShader::WaterShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"WaterShader_vs.cso", L"WaterShader_ps.cso");
}


WaterShader::~WaterShader()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	// Release the light constant buffer.
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}

	// Release the time buffer.
	if (timeBuffer)
	{
		timeBuffer->Release();
		timeBuffer = 0;
	}

	if (cameraBuffer) {

		cameraBuffer->Release();
		cameraBuffer = 0;
	}

	if (attenuationBuffer) {
		attenuationBuffer->Release();
		attenuationBuffer = 0;
	}

	if (materialBuffer) {
		materialBuffer->Release();
		materialBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void WaterShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC timeBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC attenuationBufferDesc;
	D3D11_BUFFER_DESC materialBufferDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

	// Setup light buffer--------------------------------------------------------------------------------------------------------
	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

	// Setup time buffer
	timeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	timeBufferDesc.ByteWidth = sizeof(TimeBufferType);
	timeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	timeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	timeBufferDesc.MiscFlags = 0;
	timeBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&timeBufferDesc, NULL, &timeBuffer);

	// Setup camera buffer
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);

	// Setup attenuation buffer
	attenuationBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	attenuationBufferDesc.ByteWidth = sizeof(AttenuationBufferType);
	attenuationBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	attenuationBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	attenuationBufferDesc.MiscFlags = 0;
	attenuationBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&attenuationBufferDesc, NULL, &attenuationBuffer);

	// Setup material buffer
	materialBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialBufferDesc.ByteWidth = sizeof(MaterialBufferType);
	materialBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	materialBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	materialBufferDesc.MiscFlags = 0;
	materialBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&materialBufferDesc, NULL, &materialBuffer);
}


void WaterShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, XMFLOAT4 waterSpecular, XMFLOAT4 cameraPos, RenderSettings renderSetting)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	XMMATRIX tworld, tview, tproj;


	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);


	//Send camera data to pixel shader
	CameraBufferType* cameraPtr;
	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->cameraPosition = cameraPos;
	switch (renderSetting) {
		case RenderSettings::RenderColours:
			cameraPtr->renderSettings = XMFLOAT4(-1.f, 0.0f, 0.0f, 0.0f);
			break;
		case RenderSettings::WorldPosition:
			cameraPtr->renderSettings = XMFLOAT4(0.f, 0.0f, 0.0f, 0.0f);
			break;
		case RenderSettings::Normals:
			cameraPtr->renderSettings = XMFLOAT4(1.f, 0.0f, 0.0f, 0.0f);
			break;
	}

	deviceContext->Unmap(cameraBuffer, 0 );
	deviceContext->PSSetConstantBuffers(1,1,&cameraBuffer);
}

void WaterShader::setWaveParameters(ID3D11DeviceContext* deviceContext, float deltaTime, float ampl, float freq, float speed, XMFLOAT3 direction, float ampl2, float freq2, float speed2, XMFLOAT3 direction2, float ampl3, float freq3, float speed3, XMFLOAT3 direction3, float steepnessFactor, float waterHeight)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	//Additional
	// Send time data to vertex shader
	TimeBufferType* timePtr;
	deviceContext->Map(timeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	timePtr = (TimeBufferType*)mappedResource.pData;

	timePtr->time = deltaTime;
	timePtr->amplitude1 = ampl;
	timePtr->frequency1 = freq;
	timePtr->speed1 = speed;

	timePtr->direction1 = XMFLOAT4(direction.x,direction.y,direction.z,0.0f);
	
	timePtr->amplitude2 = ampl2;
	timePtr->frequency2 = freq2;
	timePtr->speed2 = speed2;
	timePtr->steepnessFactor = steepnessFactor;

	timePtr->direction2 = XMFLOAT4(direction2.x, direction2.y, direction2.z, 0.0f);

	timePtr->amplitude3 = ampl3;
	timePtr->frequency3 = freq3;
	timePtr->speed3 = speed3;
	timePtr->waterHeight = waterHeight;

	timePtr->direction3 = XMFLOAT4(direction3.x, direction3.y, direction3.z, 0.0f);

	deviceContext->Unmap(timeBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &timeBuffer);
	deviceContext->PSSetConstantBuffers(2,1,&timeBuffer);
}

void WaterShader::setLightingParameters(ID3D11DeviceContext* deviceContext, Light* light, Light* light2, float lightType, float lightType2, float spotlightSize)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	//Additional
	// Send light data to pixel shader
	LightBufferType* lightPtr;
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->diffuse[0] = light->getDiffuseColour();
	lightPtr->diffuse[1] = light2->getDiffuseColour();

	lightPtr->ambient[0] = light->getAmbientColour();
	lightPtr->ambient[1] = light2->getAmbientColour();

	lightPtr->direction[0] = XMFLOAT4(light->getDirection().x, light->getDirection().y, light->getDirection().z, 0.f);
	lightPtr->direction[1] = XMFLOAT4(light2->getDirection().x, light2->getDirection().y, light2->getDirection().z, 0.f);

	lightPtr->lightPosition[0] = XMFLOAT4(light->getPosition().x, light->getPosition().y, light->getPosition().z, 0.0f);
	lightPtr->lightPosition[1] = XMFLOAT4(light2->getPosition().x, light2->getPosition().y, light2->getPosition().z, 0.0f);

	lightPtr->lightType[0] = lightType;
	lightPtr->lightType[1] = lightType2;
	lightPtr->lightType[2] = 0.f;
	lightPtr->lightType[3] = 0.f;

	lightPtr->spotlightSize = XMFLOAT4(spotlightSize, 0.f, 0.f, 0.f);

	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);
}

void WaterShader::setAttenuationFactors(ID3D11DeviceContext* deviceContext, XMFLOAT3 attenuationFactor)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;


	//Adding attenuation values to PS
	AttenuationBufferType* attenuationPtr;
	deviceContext->Map(attenuationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	attenuationPtr = (AttenuationBufferType*)mappedResource.pData;
	attenuationPtr->constantFactor = attenuationFactor.x;
	attenuationPtr->linearFactor = attenuationFactor.y;
	attenuationPtr->quadraticFactor = attenuationFactor.z;
	attenuationPtr->padding2 = 0.0f;
	deviceContext->Unmap(attenuationBuffer, 0);
	deviceContext->PSSetConstantBuffers(3, 1, &attenuationBuffer);
}

void WaterShader::setMaterialValues(ID3D11DeviceContext* deviceContext, float roughness, float metallic, float reflectivity)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;


	//Adding material values to PS
	MaterialBufferType* materialPtr;
	deviceContext->Map(materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	materialPtr = (MaterialBufferType*)mappedResource.pData;
	materialPtr->baseReflectivity = reflectivity;
	materialPtr->metallic = metallic;
	materialPtr->roughness = roughness;
	materialPtr->padding = 0.f;
	deviceContext->Unmap(materialBuffer, 0);
	deviceContext->PSSetConstantBuffers(4, 1, &materialBuffer);
}
