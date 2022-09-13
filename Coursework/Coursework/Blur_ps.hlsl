Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

cbuffer ScreenSizeBuffer : register(b0)     //Variables in the buffer (passed from the MotionBlur.cpp and MotionBlur.h
{
	float screenWidth;
    float screenHeight;
	float2 motionVec;
    float blur;
    float isMouseP;
    float2 padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
	//float2 temp=float2(10, 10);
    float4 colour = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float texelSizeX = 1.0f / screenWidth;          // Determine the floating point size of a texel for a screen with this specific width.
    float texelSizeY = 1.0f / screenHeight;    
    int motionVecLength = length(motionVec);       //Strenght of blurring is equal to the length vector mouse-center of screen (higher the mouse movement, stronger the blur)
    float weight_ =(motionVecLength * 2+ 1); // weight of the neighbouring pixel that will contribute to the blur, (+1 because when we divide it by one, 1/0 is not possible).
                                             //We multiply the motionVecLength by two because we need it on both sides (negative and positive in the loop below)
    
    weight_ = 1.0f/(weight_);                //Divided by one is because the further away the pixel the less influence it is supposed to have on the pixel in consideration
    if(blur!=0)     //Blurring is enabled
    {
        if (isMouseP==true)    //If right mouse is pressed, start the blurring
        {
            float blurDirX;     //Holds the direction of the blurring
            float blurDirY;
            float2 dir = motionVec;     //Direction of blurring is equal to the vector between the mouse and the middle of the screen (mousePos - midScreen)
            blurDirX = dir.x;
            blurDirY = dir.y;
            for (float i = -1 * motionVecLength; i <= motionVecLength; i++)   //loop to blur the texels goes from negative to positive of the length of the motion vector
            {
                //Add the neighbouring pixel/s value to the final colour of the pixel in consideration
                colour += shaderTexture.Sample(SampleType, input.tex + float2(texelSizeX * i * blurDirX, texelSizeY * i * blurDirY)) *weight_;
                
            }
        }
        else        //If blurring is not enabled, return normal texture
        {
            colour += shaderTexture.Sample(SampleType, input.tex);
        }
    }
    else
    {
        colour += shaderTexture.Sample(SampleType, input.tex);
    }
    saturate(colour);       //Colour is saturated and then returned
    
	// Set the alpha channel to one.
    colour.a = 1.0f;
    return colour;
}

