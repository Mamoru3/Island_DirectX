#pragma once
//waterShader.h
#include "BaseShader.h"
#include "DXF.h"

using namespace std;
using namespace DirectX;

class waterShader : public BaseShader
{
public:
	struct MatrixBufferType		//Struct that has to be sent to the vertex shader
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX dirView;
		XMMATRIX dirProj;
		XMMATRIX spotView;
		XMMATRIX spotProj;
	};
	struct WaterBufferType		//Struct that has to be passed to the vertex shader
	{
		float time;
		float padding;
		XMFLOAT2 padding2;
	};
	struct LightBufferType		//Struct that has to be passed to the pixel shader
	{
		XMFLOAT4 ambient;
		XMFLOAT4 dirDiffuse;
		XMFLOAT3 dirDirection;
		float padding;
		float spotAngle;
		XMFLOAT3 spotPosition;
		XMFLOAT4 spotDiffuse;
		XMFLOAT3 spotDirection;
		float padding_2;
		XMFLOAT3 pointPos;
		float padding_3;
		XMFLOAT4 pointDiff;
	};
	

	waterShader(ID3D11Device* device, HWND hwnd);
	~waterShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, Timer* timer, float* totTime, ID3D11ShaderResourceView* depthMap, ID3D11ShaderResourceView* depthMap2, Light* dirLight, Light* spotLight,Light*pointLight, float spotAngle);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	//------------------------Buffers holding the information to be sent to the shaders
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* waterBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* lightBuffer;
};

