//Shadow shader.h
#ifndef _SHADOWSHADER_H_
#define _SHADOWSHADER_H_

#include "DXF.h"

using namespace std;
using namespace DirectX;


class ShadowShader : public BaseShader
{
private:
	struct MatrixBufferType		//Struct to be sent to the buffer in shadow_vs
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX dirView;
		XMMATRIX dirProj;
		XMMATRIX spotView;
		XMMATRIX spotProj;
	};

	struct LightBufferType		//Struct to be sent to the buffer in shadow_ps
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
		float padding3;
		XMFLOAT4 pointDiff;
	};

public:

	ShadowShader(ID3D11Device* device, HWND hwnd);
	~ShadowShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView*depthMap, ID3D11ShaderResourceView* depthMap2, Light* light, Light* light2, Light* pointLight,float spotAngle);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	//-----------------Different buffers holding the information that has to be sent to the shaders.
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* lightBuffer;
};

#endif