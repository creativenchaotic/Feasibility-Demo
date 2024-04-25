#include "SDFTestShader.h"

SDFTestShader::SDFTestShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"SDFTestShader_vs.cso", L"SDFTestShader_ps.cso");
}

SDFTestShader::~SDFTestShader()
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

	if(sdfBuffer)
	{
		sdfBuffer->Release();
		sdfBuffer = 0;

	}

	if(cameraBuffer)
	{
		cameraBuffer->Release();
		cameraBuffer = 0;
	}

	if(sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	if (sampleState3D)
	{
		sampleState3D->Release();
		sampleState3D = 0;
	}


	//Release base shader components
	BaseShader::~BaseShader();
}

void SDFTestShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC sdfBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_SAMPLER_DESC samplerDesc3D;

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

	//Camera Buffer
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);

	//SDF Values Buffer
	sdfBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	sdfBufferDesc.ByteWidth = sizeof(SDFValuesBufferType);
	sdfBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	sdfBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	sdfBufferDesc.MiscFlags = 0;
	sdfBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&sdfBufferDesc, NULL, &sdfBuffer);

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

	// Create the texture sampler state.
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

	// Create a texture sampler state description.
	samplerDesc3D.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc3D.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc3D.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc3D.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc3D.MipLODBias = 0.0f;
	samplerDesc3D.MaxAnisotropy = 1;
	samplerDesc3D.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc3D.MinLOD = 0;
	samplerDesc3D.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	renderer->CreateSamplerState(&samplerDesc3D, &sampleState3D);
}

void SDFTestShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, XMFLOAT3 cameraVector, float delta, ID3D11ShaderResourceView* renderTexture)
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

	CameraBufferType* cameraDataPtr;
	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraDataPtr = (CameraBufferType*)mappedResource.pData;
	cameraDataPtr->cameraPosition = XMFLOAT4(cameraVector.x, cameraVector.y, cameraVector.z, 1.f);
	cameraDataPtr->timer = XMFLOAT4(delta, 0.f,0.f,0.f);
	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &cameraBuffer);
	deviceContext->PSSetConstantBuffers(0, 1, &cameraBuffer);

	// Set shader texture and sampler resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &renderTexture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(1, 1, &sampleState3D);
}

void SDFTestShader::setParticlePositionsSRV(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* computeShaderSRV, ID3D11ShaderResourceView* texture3d)
{
	deviceContext->PSSetShaderResources(1, 1, &computeShaderSRV);
	deviceContext->PSSetShaderResources(2, 1, &texture3d);
}

void SDFTestShader::setSDFParameters(ID3D11DeviceContext* deviceContext, float blendVal, float numParticles)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	SDFValuesBufferType* dataPtr;
	deviceContext->Map(sdfBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (SDFValuesBufferType*)mappedResource.pData;
	dataPtr->blendingAmount = XMFLOAT4(blendVal, 0.f,0.f,0.f);
	dataPtr->numParticles = XMFLOAT4(numParticles, 0.f, 0.f, 0.f);
	deviceContext->Unmap(sdfBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &sdfBuffer);

}


