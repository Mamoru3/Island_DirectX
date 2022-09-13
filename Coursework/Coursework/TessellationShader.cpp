// tessellation shader.cpp
#include "tessellationshader.h"


TessellationShader::TessellationShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"tessellation_vs.cso", L"tessellation_hs.cso", L"tessellation_ds.cso", L"shadow_ps.cso");
}


TessellationShader::~TessellationShader()
{
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void TessellationShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{

	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC lightBufferDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Setup light buffer
// Setup the description of the light dynamic constant buffer that is in the pixel shader.
// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	D3D11_BUFFER_DESC HSBufferDesc;
	HSBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	HSBufferDesc.ByteWidth = sizeof(HSBufferType);
	HSBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HSBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HSBufferDesc.MiscFlags = 0;
	HSBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&HSBufferDesc, NULL, &HSBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

	// Setup light buffer
	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

	// Sampler for shadow map sampling.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	renderer->CreateSamplerState(&samplerDesc, &sampleStateShadow);
}

void TessellationShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);

	// Load other required shaders.
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
}


void TessellationShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, XMFLOAT3 dist_tess,float tess_res, ID3D11ShaderResourceView* texture,Light*light, Light* spotLight, float spotangle, Light* pointLight,ID3D11ShaderResourceView* depthMap, ID3D11ShaderResourceView* depthMap2)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);
	
	XMMATRIX dirViewMatrix = XMMatrixTranspose(light->getViewMatrix());
	XMMATRIX dirProjMatrix = XMMatrixTranspose(light->getOrthoMatrix());

	XMMATRIX spotViewMatrix = XMMatrixTranspose(spotLight->getViewMatrix());
	XMMATRIX spotProjMatrix = XMMatrixTranspose(spotLight->getOrthoMatrix());


	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	dataPtr->dirView = dirViewMatrix;
	dataPtr->dirProj= dirProjMatrix;
	dataPtr->spotView = spotViewMatrix;
	dataPtr->spotProj = spotProjMatrix;

	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->DSSetConstantBuffers(0, 1, &matrixBuffer);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);



	//Additional
	// Send light data to pixel shader
	LightBufferType* lightPtr;
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->ambient = light->getAmbientColour();
	lightPtr->diffuse = light->getDiffuseColour();
	lightPtr->direction = light->getDirection();
	lightPtr->angle = spotangle;

	lightPtr->padding = 1;
	lightPtr->padding2 = 0;

	lightPtr->spotDiff = spotLight->getDiffuseColour();
	lightPtr->spotDir = spotLight->getDirection();
	lightPtr->spotPos = spotLight->getPosition();

	lightPtr->pointDiff = pointLight->getDiffuseColour();
	lightPtr->pointPos = pointLight->getPosition();
	lightPtr->padding3 = 0.0f;

	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);


	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(HSBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	HSBufferType* edges_ = (HSBufferType*)mappedResource.pData;
	edges_->dist_tessellation.x = dist_tess.x;
	edges_->dist_tessellation.y = dist_tess.y;
	edges_->dist_tessellation.z = dist_tess.z;
	edges_->tess_resolution_to_shader = tess_res;
	deviceContext->Unmap(HSBuffer, 0);

	// Set shader texture resource in the domain shader.
	deviceContext->DSSetShaderResources(1, 1, &texture);

	deviceContext->DSSetSamplers(1, 1, &sampleState);

	// Set shader texture resource in the vertex shader.
	deviceContext->VSSetShaderResources(1, 1, &texture);
	deviceContext->VSSetSamplers(1, 1, &sampleState);

	// Set shader texture resource in the hull shader.
	deviceContext->HSSetConstantBuffers(1, 1, &HSBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetShaderResources(1, 1, &depthMap);
	deviceContext->PSSetShaderResources(2, 1, &depthMap2);
	deviceContext->PSSetSamplers(1, 1, &sampleStateShadow);

	deviceContext->PSSetSamplers(0, 1, &sampleState);
}


