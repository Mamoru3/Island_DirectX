// Motion blur shader handler
// Loads motionBlur shaders (vs and ps)
// Passes screen height to shaders, for sample coordinate calculation
#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

class MotionBlur : public BaseShader
{
private:

	struct ScreenSizeBufferType		//Struct passed to the buffer in the MotionBlur_ps
	{
		float screenWidth;
		float screenHeight;
		XMFLOAT2 motionVec;
		float blur;
		float isKeyHeld;
		XMFLOAT2 padding;
	};

public:

	MotionBlur(ID3D11Device* device, HWND hwnd,int width, int height);
	~MotionBlur();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, float width,float height ,XMFLOAT2 mouseVec, float blur, float isRightMousePressed);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
								//Creation of buffers which will be passed to the shader
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* screenSizeBuffer;
};
