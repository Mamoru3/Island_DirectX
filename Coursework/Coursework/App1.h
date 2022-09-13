// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"
#include "TessellationShader.h"
#include "TessellationQuadMesh.h"
#include "MotionBlur.h"
#include "TextureShader.h"
#include "Input.h"
#include "ShadowShader.h"
#include "DepthShader.h"
#include "waterShader.h"

class App1 : public BaseApplication
{
public:

	App1();				//Constructor
	~App1();			//Destructor		
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);		//Initialization

	bool frame();			

protected:
	bool render();		//Render function, passes are inside it
	void gui();			//ImGui function
	void orthoRender(XMMATRIX worldM, XMMATRIX orthoV, XMMATRIX orthoM);			//Renders the two orthoMeshes(light point of view on the scene)
	void firstPass();		//First pass function
	void MotionBlurPass();		//Motion blur function
	void depthPass();			//Depth pass function

	void firstLightShadow();	//First light shadows function, calculated and put in orthomesh
	void secondLightShadow();   //Second light shadows function, calculated and put in orthomesh
	void nonBlurredSceneFinPass(XMMATRIX worldM, XMMATRIX viewM, XMMATRIX projM);		//Normal scene rendered function (not blur)

	void finalPass();			//Final pass function, displays to screen (blur or non blur)
private:


	TessellationQuadMesh* terrain;		//Main terrain pointer, dynamically tessellated and manipulated
	PlaneMesh* water;					//Water plane, manipulated and animated
	waterShader* WaterShader;			//Pointer to the water shader
	TessellationShader* shader;			//Pointer to the tessellation shader
	RenderTexture* renderTexture;		//Render to texture variable pointer
	TextureShader* textureShader;		//general texture shader pointer (used on simple shapes, tessellated nor manipulated)
	SphereMesh* sphere;					//Normal sphere
	RenderTexture* MotionBlurTexture;	//Motion blur render to texture, where motion blur is applied
	MotionBlur* MotionBlurShader;		//Pointer to motion blur shader
	OrthoMesh* orthoMesh;				//Ortho mesh for blurring
	SphereMesh* sphereMain;				//SpotLight sphere (useful to debug)


	Light* light;						//Main light (directional)
	Light* spotLight;					//Spotlight
	Light* pointLight;

	ShadowMap* dirShadowMap;				//Shadow mapping for directional light
	ShadowMap* spotShadowMap;				//Shadow mapping for spotlight
	OrthoMesh* dirShadowMesh;				//directional light ortho mesh
	OrthoMesh* spotShadowMesh;				//spot light ortho mesh

	ShadowShader* shadowShader;				//Shadow shader pointer
	DepthShader* depthShader;				//depth shader pointer (directional light)

	Input* mouse;							//Mouse pointer to calculate motion blur 
	DepthShader* depthShader2;				//depth shader pointer (spot light)

	XMFLOAT4 tess_edges;					//edges for tessellation calculation 
	XMFLOAT3 dist_tess;						//distance for tessellation 
	XMFLOAT2 mousePos;						//Actual mouse position
	XMFLOAT2 prevMousePos;					//Previous mouse position

	XMFLOAT3 spotDir;				//---------------Attributes for spotlight
	XMFLOAT3 spotPos;
	XMFLOAT4 spotCol;

	XMFLOAT3 pointPos;				//---------------Point light attributes
	XMFLOAT4 pointCol;


	XMFLOAT3 dirLightDirection;				//---------------directional light attributes
	XMFLOAT4 dirLightColour;

	float spotAngle;			//Angle for spotlight
	float blur;					//Blur yes/no
	bool blurBool;				//Blur bool for imgui
	bool postProc;				//postProc yes/no
	bool pointLightBool;		//point light yes/no
	bool dirLightVar;			//dir light yes/no
	bool spotLightVar =false;   //spot light yes/no

	float isRightPressed;		//right button detection for motion blur
	
	bool tessellate;			//tesselate yes/no
	float tess_res;				//resolution for tessellation 1-64
	float totTime;				//Total time (wave manipulation


	XMFLOAT3 spherePosMain;			//Sphere attributes.
	XMFLOAT3 sphereSizeMain;

	AModel* pandaModel;				//3d model for panda
	XMFLOAT3 pandaSize;				//panda size
	XMFLOAT3 pandaPos;				//panda position

};

#endif