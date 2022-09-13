// Light shader.h
// Basic single light shader setup
#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;


class TessellationShader : public BaseShader
{

public:
	struct MatrixBufferType			//Sent to the vertex shader
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;

		XMMATRIX dirView;
		XMMATRIX dirProj;
		XMMATRIX spotView;
		XMMATRIX spotProj;
	};

	struct HSBufferType				//Sent to the hull shader
	{
		
		XMFLOAT3 dist_tessellation;
		float tess_resolution_to_shader;
	};
	struct LightBufferType			//Sent to the pixel shader
	{		
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT3 direction;
		float padding;	

		float angle;
		XMFLOAT3 spotPos;
		XMFLOAT4 spotDiff;
		XMFLOAT3 spotDir;
		float padding2;		
		
		XMFLOAT3 pointPos;
		float padding3;
		XMFLOAT4 pointDiff;
		
	};
	TessellationShader(ID3D11Device* device, HWND hwnd);
	~TessellationShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, XMFLOAT3 dist_tess,float tess_res, ID3D11ShaderResourceView* texture, Light* light, Light* spotLight,float spotangle,Light *pointLight, ID3D11ShaderResourceView* depthMap, ID3D11ShaderResourceView* depthMap2);

private:
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename);

private:			//-------------------All the different buffers needed
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* HSBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* lightBuffer;
	ID3D11SamplerState* sampleStateShadow;

};
