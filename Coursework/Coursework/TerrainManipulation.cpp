#include "TerrainManipulation.h"

TerrainManipulation::TerrainManipulation(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"TerrainManipulation_vs.cso", L"TerrainManipulation_ps.cso");
}


TerrainManipulation::~TerrainManipulation()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the sampler state.
	if (sampleState2)
	{
		sampleState2->Release();
		sampleState2 = 0;
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

	if (cameraBuffer)
	{
		cameraBuffer->Release();
		cameraBuffer = 0;

	}
	// Release the light constant buffer.
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}

	if (materialBuffer) {
		materialBuffer->Release();
		materialBuffer = 0;
	}

	if (shadowSampler) {
		shadowSampler->Release();
		shadowSampler = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void TerrainManipulation::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_SAMPLER_DESC samplerDesc2;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC attenuationBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC materialBufferDesc;
	D3D11_SAMPLER_DESC shadowSamplerDesc;

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

	//SETTING UP SECOND SAMPLER-------------------------------------------------------------------------------------------------
	// Create a texture sampler state description.
	samplerDesc2.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc2.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc2.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc2.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc2.MipLODBias = 0.0f;
	samplerDesc2.MaxAnisotropy = 1;
	samplerDesc2.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc2.MinLOD = 0;
	samplerDesc2.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc2, &sampleState2);

	//SETTING UP SHADOW SAMPLER--------------------------------------------------------------------------------------------------
	// Sampler for shadow map sampling.
	shadowSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.BorderColor[0] = 1.0f;
	shadowSamplerDesc.BorderColor[1] = 1.0f;
	shadowSamplerDesc.BorderColor[2] = 1.0f;
	shadowSamplerDesc.BorderColor[3] = 1.0f;
	renderer->CreateSamplerState(&shadowSamplerDesc, &shadowSampler);

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

	//Creating camera buffer description
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);

	//Creating attenuation buffer description
	attenuationBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	attenuationBufferDesc.ByteWidth = sizeof(AttenuationBufferType);
	attenuationBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	attenuationBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	attenuationBufferDesc.MiscFlags = 0;
	attenuationBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&attenuationBufferDesc, NULL, &attenuationBuffer);


	//Creating material buffer description
	materialBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialBufferDesc.ByteWidth = sizeof(MaterialBufferType);
	materialBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	materialBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	materialBufferDesc.MiscFlags = 0;
	materialBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&materialBufferDesc, NULL, &materialBuffer);
}


void TerrainManipulation::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* texture2, ID3D11ShaderResourceView* texture3, ID3D11ShaderResourceView* texture4, ID3D11ShaderResourceView* texture5, ID3D11ShaderResourceView* texture6, ID3D11ShaderResourceView* texture7, ID3D11ShaderResourceView* texture8, ID3D11ShaderResourceView* texture9, ID3D11ShaderResourceView* texture10, Light* directionalLight, Light* spotlight)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	XMMATRIX tworld, tview, tproj;


	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);
	XMMATRIX tLightViewMatrix = XMMatrixTranspose(directionalLight->getViewMatrix());
	XMMATRIX tLightProjectionMatrix = XMMatrixTranspose(directionalLight->getOrthoMatrix());
	XMMATRIX spotlightViewMatrix = XMMatrixTranspose(spotlight->getViewMatrix());
	XMMATRIX spotlightProjectionMatrix = XMMatrixTranspose(spotlight->getProjectionMatrix());

	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	dataPtr->lightView = tLightViewMatrix;
	dataPtr->lightProjection = tLightProjectionMatrix;
	dataPtr->spotlightView = spotlightViewMatrix;
	dataPtr->spotlightProjection = spotlightProjectionMatrix;

	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetShaderResources(1, 1, &texture3);//cliff texture
	deviceContext->PSSetShaderResources(2, 1, &texture5);//moss texture
	deviceContext->PSSetShaderResources(3, 1, &texture7);//snow texture
	deviceContext->PSSetShaderResources(4, 1, &texture8);//cliff roughness texture
	deviceContext->PSSetShaderResources(5, 1, &texture9);//moss roughness texture
	deviceContext->PSSetShaderResources(6, 1, &texture10);//snow roughness texture
	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(1, 1, &sampleState2);

	

	// Set shader texture resource in the pixel shader.
	deviceContext->VSSetShaderResources(0, 1, &texture);
	deviceContext->VSSetShaderResources(1, 1, &texture2);//cliff heightmap
	deviceContext->VSSetShaderResources(2, 1, &texture4);//moss heightmap
	deviceContext->VSSetShaderResources(3, 1, &texture6);//snow heightmap
	deviceContext->VSSetSamplers(0, 1, &sampleState);
	deviceContext->VSSetSamplers(1, 1, &sampleState2);


}

void TerrainManipulation::setAttenuationFactors(ID3D11DeviceContext* deviceContext, XMFLOAT3 attenuationFactor)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;


	//Attenuation values
	AttenuationBufferType* attenuationPtr;
	deviceContext->Map(attenuationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	attenuationPtr = (AttenuationBufferType*)mappedResource.pData;
	attenuationPtr->constantFactor = attenuationFactor.x;
	attenuationPtr->linearFactor = attenuationFactor.y;
	attenuationPtr->quadraticFactor = attenuationFactor.z;
	attenuationPtr->padding2 = 0.0f;
	deviceContext->Unmap(attenuationBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &attenuationBuffer);
}

void TerrainManipulation::setLightingParameters(ID3D11DeviceContext* deviceContext, Light* light, Light* light2, float lightType, float lightType2, float spotlightSize)
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

	lightPtr->spotlightSize = XMFLOAT4(spotlightSize, 0.f, 0.f, 0.f);

	lightPtr->lightType[0] = lightType;
	lightPtr->lightType[1] = lightType2;
	lightPtr->lightType[2] = 0.f;
	lightPtr->lightType[3] = 0.f;



	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);
}

void TerrainManipulation::setCameraPosition(ID3D11DeviceContext* deviceContext, XMFLOAT3 cameraPos)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;


	//Adding camera buffer to vertex shader
	CameraBufferType* cameraPtr;
	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->cameraPosition = XMFLOAT4(cameraPos.x,cameraPos.y,cameraPos.z,0.f);
	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &cameraBuffer);
	deviceContext->PSSetConstantBuffers(2, 1, &cameraBuffer);
}

void TerrainManipulation::setMaterialValues(ID3D11DeviceContext* deviceContext, float grassMetallic, float grassReflectivity, float rockMetallic, float rockReflectivity, float snowMetallic, float snowReflectivity, float sandMetallic, float sandReflectivity)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	//Adding material buffer to vertex shader
	MaterialBufferType* materialPtr;
	deviceContext->Map(materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	materialPtr = (MaterialBufferType*)mappedResource.pData;

	//Grass
	materialPtr->grassBaseReflectivity = XMFLOAT4(grassReflectivity,0.F,0.F,0.F);
	materialPtr->grassMetallic = XMFLOAT4(grassMetallic,0.f,0.f,0.f);

	//Rock
	materialPtr->rockReflectivity = XMFLOAT4(rockReflectivity,0.F,0.F,0.F);
	materialPtr->rockMetallic = XMFLOAT4(rockMetallic,0.F,0.F,0.F);

	//Snow
	materialPtr->snowReflectivity = XMFLOAT4(snowReflectivity, 0.F,0.F,0.F);
	materialPtr->snowMetallic = XMFLOAT4(snowMetallic,0.F,0.F,0.F);

	//Sand
	materialPtr->sandReflectivity = XMFLOAT4(sandReflectivity,0.F,0.F,0.F);//UNUSED
	materialPtr->sandMetallic = XMFLOAT4(sandMetallic,0.F,0.F,0.F);//UNUSED

	deviceContext->Unmap(materialBuffer, 0);
	deviceContext->PSSetConstantBuffers(3, 1, &materialBuffer);
}

void TerrainManipulation::setShadowValues(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* depthTxtr, ID3D11ShaderResourceView* depthTxtr2)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	deviceContext->PSSetShaderResources(7, 1, &depthTxtr);
	deviceContext->PSSetShaderResources(8, 1, &depthTxtr2);
	deviceContext->PSSetSamplers(2, 1, &shadowSampler);

}
