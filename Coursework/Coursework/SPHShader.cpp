#include "SPHShader.h"
//Basic Shader to render the SPH simulation particles

SPHShader::SPHShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"SPHParticleShader_vs.cso", L"SPHParticleShader_ps.cso");
}


SPHShader::~SPHShader()
{

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

	if (materialBuffer) {
		materialBuffer->Release();
		materialBuffer = 0;

	}

	if (lightBuffer) {
		lightBuffer->Release();
		lightBuffer = 0;
	}

	if (cameraBuffer) {
		cameraBuffer->Release();
		cameraBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void SPHShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC materialBufferDesc;
	D3D11_BUFFER_DESC particleIndexBufferDesc;

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

	// Setup camera buffer
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);

	// Setup material buffer
	materialBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialBufferDesc.ByteWidth = sizeof(MaterialBufferType);
	materialBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	materialBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	materialBufferDesc.MiscFlags = 0;
	materialBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&materialBufferDesc, NULL, &materialBuffer);


	// Setup particle index buffer
	particleIndexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	particleIndexBufferDesc.ByteWidth = sizeof(ParticleIndexBufferType);
	particleIndexBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	particleIndexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	particleIndexBufferDesc.MiscFlags = 0;
	particleIndexBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&particleIndexBufferDesc, NULL, &particleIndexBuffer);

}

void SPHShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, XMFLOAT4 cameraPos, RenderSettings renderSetting)
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

	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &cameraBuffer);
}

void SPHShader::setLightingParameters(ID3D11DeviceContext* deviceContext, Light* light)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	//Additional
	// Send light data to pixel shader
	LightBufferType* lightPtr;
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;

	lightPtr->diffuse = light->getDiffuseColour();
	lightPtr->ambient = light->getAmbientColour();
	lightPtr->direction = XMFLOAT4(light->getDirection().x, light->getDirection().y, light->getDirection().z, 0.f);
	lightPtr->lightPosition = XMFLOAT4(light->getPosition().x, light->getPosition().y, light->getPosition().z, 0.0f);

	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);
}

void SPHShader::setMaterialValues(ID3D11DeviceContext* deviceContext, float roughness, float metallic, float reflectivity)
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
	deviceContext->PSSetConstantBuffers(2, 1, &materialBuffer);
}

void SPHShader::setParticleIndex(ID3D11DeviceContext* deviceContext, int index)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	//Adding material values to PS
	ParticleIndexBufferType* particleIndexPtr;
	deviceContext->Map(particleIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	particleIndexPtr = (ParticleIndexBufferType*)mappedResource.pData;
	particleIndexPtr->particleIndex = index;
	deviceContext->Unmap(particleIndexBuffer, 0);
	deviceContext->VSSetConstantBuffers(2, 1, &particleIndexBuffer);
}

void SPHShader::setSimulationDataSRV(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* computeShaderSRV)
{
	deviceContext->VSSetShaderResources(1, 1, &computeShaderSRV);
}
