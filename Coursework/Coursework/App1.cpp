// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);
	//Tessellation
	//--------------------------------------------------------Textures and models loading
	textureMgr->loadTexture(L"height", L"res/Heightmap.jpg");
	textureMgr->loadTexture(L"bunny", L"res/bunny.png");
	textureMgr->loadTexture(L"brick", L"res/brick1.DDS");
	textureMgr->loadTexture(L"sea", L"res/temp.jpg");
	textureMgr->loadTexture(L"pandaTexture", L"res/pandaTex.jpeg");

	pandaModel = new AModel(renderer->getDevice(), "res/panda.obj");

	//Meshes and shader initialization
	shader = new TessellationShader(renderer->getDevice(), hwnd);
	textureShader = new TextureShader(renderer->getDevice(), hwnd);

	water = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	WaterShader = new waterShader(renderer->getDevice(), hwnd);
	terrain = new TessellationQuadMesh(renderer->getDevice(), renderer->getDeviceContext(), tess_res);

	MotionBlurShader = new MotionBlur(renderer->getDevice(), hwnd,screenWidth,screenHeight);
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);	// Full screen size
	//Initialise required render textures. We need a RT for the first pass at the scene (unblurred), to store the motion blur.
	renderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	//Motion blur renderTexture
	MotionBlurTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	
	//Meshes initialisation and attributes
	sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	sphereMain = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	sphereSizeMain = XMFLOAT3(1.0f, 1.f, 1.0f);
	spherePosMain = XMFLOAT3(30, 8, 40);

	pandaPos = XMFLOAT3(20, 8, -27);
	pandaSize = XMFLOAT3(0.3, 0.3, 0.3);

	//Tessellation values are set
	tess_res = 10;
	dist_tess = XMFLOAT3(0, 0, 0);
	//Directional light (main light) is created and initialized
	light = new Light();
	dirLightDirection = XMFLOAT3(0.0f, -1.f, -0.6f);
	dirLightColour=XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f);
	//light->setDirection(-1.0f, -0.2f, 0.0f);
	light->setPosition(0.f, 6.0f, 0.f);
	light->setDiffuseColour(dirLightColour.x, dirLightColour.y, dirLightColour.z,dirLightColour.w);
	light->setDirection(dirLightDirection.x, dirLightDirection.y, dirLightDirection.z);
	light->generateOrthoMatrix(150, 150, -40, 100);
	light->setAmbientColour(0.6, 0.6, 0.6, 1);

	//Shadow map width and height
	int shadowmapWidth = 2048;
	int shadowmapHeight = 2048;

	//Dimension of the orthomesh
	int sceneWidth = 100;
	int sceneHeight = 100;
	
	//Camera first position and rotatio to have a nice view of the scene
	camera->setPosition(25, 40, -20);
	camera->setRotation(45, 0, 0);
	


	// Initialise point light and set its attributes
	pointLight = new Light();
	pointCol = XMFLOAT4(0.6f, 0.5f, 0.0f, 0.0f);
	pointPos = XMFLOAT3(10, 11.f, 10.0f);
	pointLight->setDiffuseColour(pointCol.x,pointCol.y,pointCol.z,pointCol.w);
	pointLight->setPosition(pointPos.x,pointPos.y,pointPos.z);

	// Initialise spotlight and set its attributes
	spotLight = new Light();
	spotPos = XMFLOAT3(33, 15, 40);
	spotCol = XMFLOAT4(1, 1, 1, 1);
	spotDir = XMFLOAT3(0.0f, -0.99f, 0.0f);
	spotLight->setDiffuseColour(spotCol.x, spotCol.y, spotCol.z, spotCol.w);
	spotLight->setDirection(spotDir.x, spotDir.y, spotDir.z);
	spotLight->setPosition(spotPos.x, spotPos.y, spotPos.z);
	spotLight->generateOrthoMatrix(150, 150, -30, 100);

	//mouse position to calculate motion blur
	mousePos.x = input->getMouseX();
	mousePos.y = input->getMouseY();

	//Var to determine if post processing is on
	postProc = 0.0f;
	//Starting angle of the spotlight
	spotAngle = 0.80;
	
	//Depth shader and shadow shader initialised
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);

	//Shadow maps for directional and spotlight
	dirShadowMap = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
	spotShadowMap = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

	//Orthomeshes to debug shadows and lights for spotlight and directional light
	dirShadowMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), 200, 200, -screenWidth / 2.7, screenHeight / 2.7);
	spotShadowMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), 200, 200, screenWidth / 2.7, screenHeight / 2.7);
}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();
	// Release the Direct3D object.
}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}
	
	return true;
}

bool App1::render()
{
	// Render scene
	

	if (blurBool == 1)
		// First pass to store the normal scene for the motion blur
	{
		firstPass();

		MotionBlurPass();
	}
	depthPass();
	totTime += timer->getTime();

	// Render final pass to frame buffer
	finalPass();


	return true;

}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	if (ImGui::CollapsingHeader("Tessellation:"))		//Control tessellation from ImGui
	{
		ImGui::SliderFloat("Tessellation resolution, ", &tess_res, 5, 64);
	}
	else		//If tessellation collapsing header is closed set automatically the tessellation resolution to 1
		tess_res = 1;

	if(ImGui::CollapsingHeader("Lights"))		//Lights collapsing header
	{
		ImGui::Checkbox("Point Light", &pointLightBool);		//Control point Light
		if (pointLightBool == true)
		{
			ImGui::SliderFloat3("Point Light Colour", (float*)&pointCol, 0, 1);
			ImGui::SliderFloat3("PointLight position", (float*)&pointPos, -50, 50);

		}
		ImGui::Checkbox("Directional Light", &dirLightVar);			//Control directional light
		if (dirLightVar)
		{
			ImGui::SliderFloat3("Direction directional light", (float*)&dirLightDirection, -1, 1);
			ImGui::SliderFloat4("Directional light colour", (float*)&dirLightColour, 0, 1);

		}
		ImGui::Checkbox("Spotlight", &spotLightVar);				//Control spotlight
		if (spotLightVar)
		{
			ImGui::SliderFloat("SpotLight angle ", &spotAngle, 0, 1);
			ImGui::SliderFloat4("SpotLight Colour", (float*)&spotCol, 0, 1);
			ImGui::SliderFloat3("SpotLight Direction", (float*)&spotDir, -0.99, 0.99);
			ImGui::SliderFloat3("SpotLight position", (float*)&spotPos, -50, 50);
		}
	}

	if (ImGui::CollapsingHeader("Geometry"))		//Geometry positions
	{
		ImGui::SliderFloat3("Sphere position", (float*)&spherePosMain, -100, 100);

		ImGui::SliderFloat3("Panda position", (float*)&pandaPos, -40, 40);
	}

	if (ImGui::CollapsingHeader("PostProcessing"))	//Control Motion Blur
	{
			ImGui::Checkbox("Motion Blur", &blurBool);
			if (blurBool == true)
				blur = 0.01;
			else
				blur = 0;
	}
	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void App1::firstPass()		//First pass, prepares normal scene non blurred to be passed to the blurPass to be blurred, necessary to 
{
	// Set the render target to be the render to texture and clear it
	renderTexture->setRenderTarget(renderer->getDeviceContext());
	renderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 1.0f, 1.0f);
	// Clear the scene. (default blue colour)
	//renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	//Pass updated camera position to the tessellation shader to dynamically tessellate the terrain
	camera->update();
	float cam_x = camera->getPosition().x;
	float cam_y = camera->getPosition().y;
	float cam_z = camera->getPosition().z;
	dist_tess = XMFLOAT3(cam_x, cam_y, cam_z);		

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, quadRot;
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();


		// Send geometry data, set shader parameters, render object with shader
		worldMatrix = renderer->getWorldMatrix();
		terrain->sendData(renderer->getDeviceContext());
		shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, dist_tess, tess_res, textureMgr->getTexture(L"height"), light, spotLight, spotAngle, pointLight, dirShadowMap->getDepthMapSRV(), spotShadowMap->getDepthMapSRV());
		shader->render(renderer->getDeviceContext(), terrain->getIndexCount());

								//Position of the sphere in the scene
		worldMatrix = renderer->getWorldMatrix();
		XMMATRIX worldMatrixSphere = renderer->getWorldMatrix();
		XMMATRIX ShadowSphereSize = XMMatrixScaling(sphereSizeMain.x, sphereSizeMain.y, sphereSizeMain.z);	// Render floor
		XMMATRIX SpherePosition = XMMatrixTranslation(spherePosMain.x, spherePosMain.y, spherePosMain.z);
		//worldMatrixSphere = XMMatrixMultiply(worldMatrixSphere, SpherePosition);
		worldMatrixSphere *= SpherePosition;
		worldMatrixSphere *= ShadowSphereSize;
		// Send geometry data, set shader parameters, render object with shader
		sphereMain->sendData(renderer->getDeviceContext());
		shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrixSphere, viewMatrix, projectionMatrix, textureMgr->getTexture(L"bunny"), dirShadowMap->getDepthMapSRV(), spotShadowMap->getDepthMapSRV(), light, spotLight,pointLight, spotAngle);
		shadowShader->render(renderer->getDeviceContext(), sphereMain->getIndexCount());


		XMMATRIX worldM;		//Position of the sphere which casts the spotlight
		worldM = renderer->getWorldMatrix();
		XMMATRIX spherePos = XMMatrixTranslation(spotPos.x, spotPos.y, spotPos.z);
		worldM = XMMatrixMultiply(worldM, spherePos);
		//SpotLight sphere, send to the texture shader
		sphere->sendData(renderer->getDeviceContext());
		textureShader->setShaderParameters(renderer->getDeviceContext(), worldM, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"));
		textureShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

								//Water terrain, sent to the WaterShader.cpp
		XMMATRIX waterPos = XMMatrixTranslation(0.0, 2.5, 50);
		XMMATRIX waterScale = XMMatrixScaling(0.5, 0.5, 0.2);
		worldMatrix = renderer->getWorldMatrix();
		//worldM += waterPos;
		worldMatrix = XMMatrixMultiply(worldMatrix, waterPos);
		worldMatrix = XMMatrixMultiply(worldMatrix, waterScale);
		// Send geometry data, set shader parameters, render object with shader
		water->sendData(renderer->getDeviceContext());
		WaterShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"sea"), timer, &totTime, dirShadowMap->getDepthMapSRV(), spotShadowMap->getDepthMapSRV(), light, spotLight, pointLight, spotAngle);
		WaterShader->render(renderer->getDeviceContext(), water->getIndexCount());

								//Panda model, sent to the shadowShader (needs to have shadows), transformed
		worldM = renderer->getWorldMatrix();
		XMMATRIX worldMPanda = renderer->getWorldMatrix();		
		XMMATRIX PandaScale = XMMatrixScaling(pandaSize.x, pandaSize.y, pandaSize.z);	// Apply transforms to panda and send the data to the shadowShader
		XMMATRIX PandaPos = XMMatrixTranslation(pandaPos.x,pandaPos.y,pandaPos.z);
		XMMATRIX PandaRot = XMMatrixRotationY(30);
		worldMPanda *= PandaPos;
		worldMPanda *= PandaScale;
		worldMPanda *= PandaRot;

		pandaModel->sendData(renderer->getDeviceContext());
		shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMPanda, viewMatrix, projectionMatrix, textureMgr->getTexture(L"pandaTexture"), dirShadowMap->getDepthMapSRV(), spotShadowMap->getDepthMapSRV(), light, spotLight, pointLight, spotAngle);
		shadowShader->render(renderer->getDeviceContext(), pandaModel->getIndexCount());

	renderer->setBackBufferRenderTarget();
	// Render GUI
	// Present the rendered scene to the screen.
}

void App1::MotionBlurPass()			//Blur applied to the render to texture non blurred
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	//Variables needed to calculate the center of the screen, passed to the shader.cpp for the motion blur
	float screenSizeX = (float)MotionBlurTexture->getTextureWidth()/2;
	float screenSizeY = (float)MotionBlurTexture->getTextureHeight()/2;


	MotionBlurTexture->setRenderTarget(renderer->getDeviceContext());
	MotionBlurTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	// Get the ortho matrix from the render to texture since texture has different dimensions being that it is smaller.
	orthoMatrix = MotionBlurTexture->getOrthoMatrix();

	//Updates the mouse position variable, passed to the shader.cpp
	mousePos.x = input->getMouseX();
	mousePos.y = input->getMouseY();
	XMFLOAT2 mouseVec;
	if (input->isRightMouseDown())		//If right mouse is down, calculate the vector between the mouse position and the center of the screen (direction of blur)
	{
		isRightPressed = 1;
		mouseVec.x = mousePos.x- screenSizeX;
		mouseVec.y = mousePos.y- screenSizeY;
	}
	else                                //If right mouse is not pressed, no blur happens
	{
		isRightPressed = 0;
		mouseVec.x = 0;
		mouseVec.y = 0;
	}
	// Render for motion Blur
	renderer->setZBuffer(false);			//Disable z-buffer (no depth)
	orthoMesh->sendData(renderer->getDeviceContext());		//Render the blurred orthomesh to the screen
	MotionBlurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, renderTexture->getShaderResourceView(), screenSizeX, screenSizeY , mouseVec ,blur, isRightPressed);
	MotionBlurShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);		//enable z-buffer (depth)

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::depthPass()
{
	firstLightShadow();
	secondLightShadow();
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();

}


void App1::firstLightShadow()			//First light shadow data, send to the ShadowShader.h and ShadowShader.cpp
{
	dirShadowMap->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());
	// get the world, view, and projection matrices from the camera and d3d objects.
	light->generateViewMatrix();
	XMMATRIX lightViewMatrix = light->getViewMatrix();
	XMMATRIX lightProjectionMatrix = light->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	//Main terrain is rendered on the first light orthoMesh, displays first light point of view
	terrain->sendData(renderer->getDeviceContext());
	shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, dist_tess, tess_res, textureMgr->getTexture(L"height"), light, spotLight, spotAngle, pointLight, dirShadowMap->getDepthMapSRV(), spotShadowMap->getDepthMapSRV());
	shader->render(renderer->getDeviceContext(), terrain->getIndexCount());

	//sphere is rendered on the first light orthoMesh, displays first light point of view
	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX ShadowSpherePos = XMMatrixTranslation(spherePosMain.x, spherePosMain.y, spherePosMain.z);
	XMMATRIX ShadowSphereSize = XMMatrixScaling(sphereSizeMain.x, sphereSizeMain.y, sphereSizeMain.z);
	worldMatrix *= ShadowSpherePos;
	worldMatrix *= ShadowSphereSize;
	sphereMain->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), sphereMain->getIndexCount());

	//Water is rendered on the first light orthoMesh, displays first light point of view
	XMMATRIX waterPos = XMMatrixTranslation(0.0, 2.5, 50);
	XMMATRIX waterScale = XMMatrixScaling(0.5, 0.5, 0.2);
	worldMatrix = renderer->getWorldMatrix();
	//worldM += waterPos;
	worldMatrix = XMMatrixMultiply(worldMatrix, waterPos);
	worldMatrix = XMMatrixMultiply(worldMatrix, waterScale);
	water->sendData(renderer->getDeviceContext());
	WaterShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, textureMgr->getTexture(L"sea"), timer, &totTime, dirShadowMap->getDepthMapSRV(), spotShadowMap->getDepthMapSRV(), light, spotLight, pointLight, spotAngle);
	WaterShader->render(renderer->getDeviceContext(), water->getIndexCount());

	//Panda is rendered on the first light orthoMesh, displays first light point of view
	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX worldMPanda = renderer->getWorldMatrix();
	XMMATRIX PandaScale = XMMatrixScaling(pandaSize.x, pandaSize.y, pandaSize.z);
	XMMATRIX PandaPos = XMMatrixTranslation(pandaPos.x, pandaPos.y, pandaPos.z);
	XMMATRIX PandaRot = XMMatrixRotationY(30);
	worldMPanda *= PandaPos;
	worldMPanda *= PandaScale;
	worldMPanda *= PandaRot;

	pandaModel->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMPanda, lightViewMatrix, lightProjectionMatrix, textureMgr->getTexture(L"pandaTexture"), dirShadowMap->getDepthMapSRV(), spotShadowMap->getDepthMapSRV(), light, spotLight, pointLight, spotAngle);
	shadowShader->render(renderer->getDeviceContext(), pandaModel->getIndexCount());

}

void App1::secondLightShadow()
{
	spotShadowMap->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());
	spotLight->generateViewMatrix();
	//terrain is rendered on the second light orthoMesh, displays second light point of view
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX lightViewMatrix2 = spotLight->getViewMatrix();
	XMMATRIX lightProjectionMatrix2 = spotLight->getOrthoMatrix();

	terrain->sendData(renderer->getDeviceContext());
	shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix2, lightProjectionMatrix2, dist_tess, tess_res, textureMgr->getTexture(L"height"), light, spotLight, spotAngle, pointLight, dirShadowMap->getDepthMapSRV(), spotShadowMap->getDepthMapSRV());
	shader->render(renderer->getDeviceContext(), terrain->getIndexCount());

	//sphere is rendered on the second light orthoMesh, displays second light point of view
	XMMATRIX ShadowSpherePos = XMMatrixTranslation(spherePosMain.x, spherePosMain.y, spherePosMain.z);	// Render floor
	XMMATRIX ShadowSphereSize = XMMatrixScaling(sphereSizeMain.x, sphereSizeMain.y, sphereSizeMain.z);	// Render floor
	worldMatrix *= ShadowSpherePos;
	worldMatrix *= ShadowSphereSize;
	sphereMain->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix2, lightProjectionMatrix2);
	depthShader->render(renderer->getDeviceContext(), sphereMain->getIndexCount());

	//water is rendered on the second light orthoMesh, displays second light point of view

	XMMATRIX waterPos = XMMatrixTranslation(0.0, 2.5, 50);
	XMMATRIX waterScale = XMMatrixScaling(0.5, 0.5, 0.2);
	worldMatrix = renderer->getWorldMatrix();
	//worldM += waterPos;
	worldMatrix = XMMatrixMultiply(worldMatrix, waterPos);
	worldMatrix = XMMatrixMultiply(worldMatrix, waterScale);
	water->sendData(renderer->getDeviceContext());
	WaterShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix2, lightProjectionMatrix2, textureMgr->getTexture(L"sea"), timer, &totTime, dirShadowMap->getDepthMapSRV(), spotShadowMap->getDepthMapSRV(), light, spotLight, pointLight, spotAngle);
	WaterShader->render(renderer->getDeviceContext(), water->getIndexCount());

	//panda is rendered on the second light orthoMesh, displays second light point of view
	worldMatrix = renderer->getWorldMatrix();
	XMMATRIX worldMPanda = renderer->getWorldMatrix();
	XMMATRIX PandaScale = XMMatrixScaling(pandaSize.x, pandaSize.y, pandaSize.z);	// Render floor
	XMMATRIX PandaPos = XMMatrixTranslation(pandaPos.x, pandaPos.y, pandaPos.z);
	XMMATRIX PandaRot = XMMatrixRotationY(30);
	//worldMatrixSphere = XMMatrixMultiply(worldMatrixSphere, SpherePosition);
	worldMPanda *= PandaPos;
	worldMPanda *= PandaScale;
	worldMPanda *= PandaRot;

	pandaModel->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMPanda, lightViewMatrix2, lightProjectionMatrix2, textureMgr->getTexture(L"pandaTexture"), dirShadowMap->getDepthMapSRV(), spotShadowMap->getDepthMapSRV(), light, spotLight, pointLight, spotAngle);
	shadowShader->render(renderer->getDeviceContext(), pandaModel->getIndexCount());

}

void App1::nonBlurredSceneFinPass(XMMATRIX worldM, XMMATRIX viewM, XMMATRIX projM)			//Non blurred scene
{
	//Pass updated camera position to the tessellation shader to dynamically tessellate the terrain
	camera->update();
	float cam_x = camera->getPosition().x;
	float cam_y = camera->getPosition().y;
	float cam_z = camera->getPosition().z;
	dist_tess = XMFLOAT3(cam_x, cam_y, cam_z);

	//Dynamically tessellated terrain is send to the tessellationshader.h and .cpp along its data
	terrain->sendData(renderer->getDeviceContext());
	shader->setShaderParameters(renderer->getDeviceContext(), worldM, viewM, projM, dist_tess, tess_res, textureMgr->getTexture(L"height"), light, spotLight, spotAngle, pointLight, dirShadowMap->getDepthMapSRV(), spotShadowMap->getDepthMapSRV());
	shader->render(renderer->getDeviceContext(), terrain->getIndexCount());

	//Water is sent to the waterShader.h and waterShader.cpp
	XMMATRIX waterPos = XMMatrixTranslation(0.0, 2.5, 50);
	XMMATRIX waterScale = XMMatrixScaling(0.5, 0.5, 0.2);
	worldM = renderer->getWorldMatrix();
	//worldM += waterPos;
	worldM = XMMatrixMultiply(worldM, waterPos);
	worldM = XMMatrixMultiply(worldM, waterScale);
	water->sendData(renderer->getDeviceContext());
	WaterShader->setShaderParameters(renderer->getDeviceContext(), worldM, viewM, projM, textureMgr->getTexture(L"sea"), timer, &totTime, dirShadowMap->getDepthMapSRV(), spotShadowMap->getDepthMapSRV(), light, spotLight, pointLight, spotAngle);
	WaterShader->render(renderer->getDeviceContext(), water->getIndexCount());


	//Sphere is sent to the textureshader.h and .cpp
	worldM = renderer->getWorldMatrix();
	XMMATRIX spherePos = XMMatrixTranslation(spotPos.x, spotPos.y, spotPos.z);
	worldM = XMMatrixMultiply(worldM, spherePos);
	sphere->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldM, viewM, projM, textureMgr->getTexture(L"brick"));
	textureShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	worldM = renderer->getWorldMatrix();
	XMMATRIX worldMatrixSphere = renderer->getWorldMatrix();
	XMMATRIX ShadowSphereSize = XMMatrixScaling(sphereSizeMain.x, sphereSizeMain.y, sphereSizeMain.z);	// Render floor
	XMMATRIX SpherePosition = XMMatrixTranslation(spherePosMain.x, spherePosMain.y, spherePosMain.z);
	//worldMatrixSphere = XMMatrixMultiply(worldMatrixSphere, SpherePosition);
	worldMatrixSphere *= SpherePosition;
	worldMatrixSphere *= ShadowSphereSize;

	sphereMain->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrixSphere, viewM, projM, textureMgr->getTexture(L"bunny"), dirShadowMap->getDepthMapSRV(), spotShadowMap->getDepthMapSRV(), light, spotLight, pointLight, spotAngle);
	shadowShader->render(renderer->getDeviceContext(), sphereMain->getIndexCount());

	//panda is sent to the textureshader.h and .cpp
	worldM = renderer->getWorldMatrix();
	XMMATRIX worldMPanda = renderer->getWorldMatrix();
	XMMATRIX PandaScale = XMMatrixScaling(pandaSize.x, pandaSize.y, pandaSize.z);	// Render floor
	XMMATRIX PandaPos = XMMatrixTranslation(pandaPos.x, pandaPos.y, pandaPos.z);
	XMMATRIX PandaRot = XMMatrixRotationY(30);
	//worldMatrixSphere = XMMatrixMultiply(worldMatrixSphere, SpherePosition);
	worldMPanda *= PandaPos;
	worldMPanda *= PandaScale;
	worldMPanda *= PandaRot;

	pandaModel->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMPanda, viewM, projM, textureMgr->getTexture(L"pandaTexture"), dirShadowMap->getDepthMapSRV(), spotShadowMap->getDepthMapSRV(), light, spotLight, pointLight, spotAngle);
	shadowShader->render(renderer->getDeviceContext(), pandaModel->getIndexCount());
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	// RENDER THE RENDER TEXTURE SCENE
	// Requires 2D rendering and an ortho mesh.
	//renderer->setZBuffer(false);
	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX viewMatrix, projectionMatrix;
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();


	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering
	//Lights attributes updated.
	spotLight->setDiffuseColour(spotCol.x, spotCol.y, spotCol.z, spotCol.w);
	spotLight->setDirection(spotDir.x, spotDir.y, spotDir.z);
	spotLight->setPosition(spotPos.x, spotPos.y, spotPos.z);
	pointLight->setDiffuseColour(pointCol.x ,pointCol.y, pointCol.z,0.0f);
	pointLight->setPosition(pointPos.x, pointPos.y, pointPos.z);
	light->setDirection(dirLightDirection.x, dirLightDirection.y, dirLightDirection.z);

	light->setDiffuseColour(dirLightColour.x, dirLightColour.y, dirLightColour.z,1.0f);

	if (blurBool == true)		//If the blurring is active, render the orthomesh that holds the blurring information 
	{
		orthoMesh->sendData(renderer->getDeviceContext());
		textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, MotionBlurTexture->getShaderResourceView());
		textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	}
	else                       //If blurring is not active, render normal scene along the two light view orthomeshes
	{
		nonBlurredSceneFinPass(worldMatrix, viewMatrix, projectionMatrix);

		worldMatrix = renderer->getWorldMatrix();
		XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
		XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering

		orthoRender(worldMatrix,orthoViewMatrix,orthoMatrix);
	}

	// Render GUI
	gui();
	// Present the rendered scene to the screen.
	renderer->endScene();
}


void App1::orthoRender(XMMATRIX worldM, XMMATRIX orthoV, XMMATRIX orthoM)
{
	//Render the two light view ortho meshes.
	renderer->setZBuffer(false);
	dirShadowMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldM, orthoV, orthoM, dirShadowMap->getDepthMapSRV());
	textureShader->render(renderer->getDeviceContext(), dirShadowMesh->getIndexCount());
	renderer->setZBuffer(true);

	renderer->setZBuffer(false);
	spotShadowMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldM, orthoV, orthoM, spotShadowMap->getDepthMapSRV());
	textureShader->render(renderer->getDeviceContext(), spotShadowMesh->getIndexCount());
	renderer->setZBuffer(true);
}
