The application developed is based on a single scene that, thanks to the use of shaders, 
demonstrates and features different graphics programming techniques.
Examples of lighting with shadows, dynamical tessellation, motion blur, and vertex 
manipulation are present. This is possible thanks to the programmable pipeline.
Moving around the scene on the x-axis with the W, A, S, D keys of the keyboard and on the y-axis with Q and E, the user can observe the different parts of the scene and the effects applied 
on them.
By holding the right button of the mouse, the player can rotate the camera and look around.
A Graphical User Interface is present on the left side of the screen called ImGui.
Using ImGui, the user can change different attributes of the elements present in the scene 
(lights positions, the strength of dynamical tessellation, etc.) to see the different results that 
this could generate.

Structure
The program features an object-oriented approach, where for each element in the scene there 
is a respective class, which then communicates with the different shader files (.hlsl).
The main class (App1.cpp and App1.h) has different purposes. Once the pointers to the classes 
are created in the header, they are initialised in the “Init()” function in the .cpp file, along with 
their starting attributes.
For each element that interacts with a shader, there is a setShaderParameters() function which 
communicates to its respective class.
Once the mentioned function is called, the data goes to the shader files, where the appropriate 
calculations are executed.
Each shader header holds a struct that will be sent to the respective shader .hlsl file. Here, the 
data will be received in a buffer, which is in a specific register. The data in the struct and the 
buffer must be in the same order. 
Data is packed into 4-bytes boundaries and HLSL packs data so that it does not cross 16-bytes, 
the appropriate padding variables must be created if the bytes are not enough.

Techniques
Motion Blur

The motion blur technique starts from App1.cpp, after creating and initializing the pointer to 
the MotionBlurShader class object, we initialize two render to textures (RTTs). One will store
the non-blurred scene, one will store the blurred one. The sizes of these render to texture will 
be equal to the size of the screen, as we want the blurred scene to be displayed in Fullscreen.
An orthographic mesh is created, which is simply a quad but built for render to texture 
processing. This orthomesh is going to hold the blurred scene and will render it.
Once this is done, the first pass function renders the scene on one of the two render to 
textures. It renders all the different meshes and elements of the scene.
After that, the render to texture where the normal scene is displayed is then used in the 
MotionBlurPass which, as the name indicates, blurs the scene.
Blurring stages.
In this pass, we set the render target to be the render to texture for the motion blur.
Two variables to calculate the centre of the screen are created. These are useful to calculate 
the vector between the mouse position and the centre of the screen; this vector will be the 
direction of the blur. We then create three different matrices, the worldMatrix, which is just 
the identity matrix, to which we will be able to apply different transformations.
The baseViewMatrix, which gets the default camera position for orthographic rendering and 
looks down on the Z-Axis; and finally, the orthoMatrix, which simply gets the orthographic 
matrix for this render to texture.
It is important to say that the orthographic projection makes it so that the objects do not shrink 
as we go farther.
Once this is done, there is the calculation of the vector between the centre of the screen and 
the position of the mouse, that gives us the vector that is used for the direction of the blur. This 
calculation will happen only when the user holds the right button of the mouse and therefore is
looking around.
The last part of this function includes the disabling of the Z-buffer, which means that no depth 
is calculated.
After this, the shader parameters are set, and the blurred scene is rendered on the orthoMesh.
Lastly, we re-enable the depth buffer.
MotionBlurPass()
In the setShaderParameters, we send our valuable information to the MotionBlur.h and .cpp.
All the .h and .cpp files follow the same structure.
For order's sake, all the different types of variables have specific names.
The structs names will end with “BufferType”, the descriptions for the buffers will end with 
“Desc”.
After the constructor and deconstructor, there is the setShaderParameters() function, which 
takes in all the data that we just sent from the App1.cpp.
There is also an initializing function that initializes the buffers’ descriptions.
The last elements in the header file are the different buffers.
MotionBlur.h
Moving on to the .cpp file, the first step is to initialize the shaders that will be used, in this case,
the “blur_vs” and “blur_ps”, vertex shader and pixel shader respectively.
In the initializing function for the buffers, all their attributes and properties are set.
Example of the matrixBufferDescription
In a buffer description, its usage and its width of bytes (which is equal to the size of the struct 
that will be sent to the shaders) are set. After other initializations, we finally create the buffer 
and assign it to the description that we just set up.
In the setShadersParameters() we set the parameters of each struct to be equal to the ones we 
pass from the App1.cpp.
For each buffer, we map it and unmap it. This must be done for each one of them.
It is important to remember that the data will be sent to a shader and a register that must be 
specified:
I.e: deviceContext->PSSetSamplers(0, 1, &sampleState);
This will be sent to the pixel shader in the 0 register. In this case, we are sending the 
sampleState for texturing.
Moving on to the Blur_vs file, we first find the buffer holding the information that we send from 
the MotionBlur class, in this case, we need the three different matrices.
There is a struct for the input, which is used to calculate the output, that will be passed to the 
blur_ps. 
In the main function of the blur_vs, there is the calculation of the vertex against the world, 
view, and projection matrices.
The output texture coordinates are set to be the same as the input ones and finally the output 
is returned and goes to the pixel shader.
The blur_ps is where all the blurring calculations happen.
In the main function, we initialize the colour, which at first will be black.
Two texelSize variables are created. They represent the distance between two neighbouring 
pixels and their value is obtained by dividing them by the screen resolution (x,y).
We specify the length of the motionBlurVector, useful for the for loop that will blur the 
neighbouring pixel and then we declare the weight.
The weight of the neighbouring pixel will be the length of the motionBlurVector * 2+1. This is 
because the blur is applied on both sides of the pixel that is being blurred.
I.e.: if the blur is 4, we need 2*2, two on one side(negative), two on the other(positive).
It is fundamental to add one to the motionBlurVector as if the vector is zero, a division by zero
is not possible.
The division by one is extremely important as we want the neighbouring pixel value to add up 
to 1 (in case more neighbouring pixels are implemented) and because the farthest pixel should 
have less influence than the closest one.
Finally, if the user is holding the right button of the mouse and moving the camera, the blurring 
will happen.
The for loop starts with i equal to the negative length of the blurring vector until it’s equal to 
the positive value of it. This allows us to blur both sides of each pixel, the positive and negative.
The blurring direction is the same one as the vector that we calculated in App1.cpp and then 
passed.
Blur_ps.
In the for loop, the x and y are multiplied by texelSize *i(which gives the texel in consideration) 
and then we assign it the blurring direction. Finally, we multiply all of this with the weight of the 
neighbouring pixel, which is what allows us to have the motion blurring.
If the user is not rotating the camera, the normal view of the scene is displayed.
Before returning the colour, we saturate it to clamp from 0 to 1.

Vertex Manipulation and Dynamic Tessellation
To achieve a vertex-manipulated plane with dynamic tessellation, a procedurally generated 
plane is needed. In this case, I chose to create a plane made of quads. This is needed as each 
quad must be tessellated to achieve a proper dynamic tessellation. In the App1 header, a 
pointer to the TessellationQuadMesh is created and it is initialized in the App1.cpp Init() 
function under the name “terrain”.
To tessellate the plane, we send the procedurally generated quad to the tessellationShader 
class.
Here, differently than the blurShader, 4 stages of the pipeline are programmed, the vertex 
shader, hull shader, domain shader and pixel shader. 
When we call setShaderParameters() in the App1 for the tessellationShader class, we send the 
required worldMatrix, viewMatrix and projectionMatrix, along with the texture that we want 
onto the plane and different variables for tessellating the plane (distance of tessellation and 
resolution of the tessellation).
With these variables, we also send the different lights and the shadowMaps for each light, but 
will be discussed later in the Lighting paragraph.
In the TessellationShader, three buffers hold the data for the tessellation, one takes care of the 
lighting, one of the different matrices and one is specific for the tessellation.
Once we send the data through, the vertex shader passes the information into the hull shader, 
which is the heart of the tessellation. In this shader, we receive the camera position and the 
tessellation resolution on the buffer “EdgeValues”, which is in the first register.
In the PatchConstantFunction (which is called for each patch) we calculate the vector of the 
distance between the camera and the different patch sides. This results in having 6 different 
distances, four for the different edges of the square and two for the inside edges of it.
Edges of each patch for dynamic tessellation
After obtaining these vectors, we extract the length (magnitude) out of them. The length of 
each vector is particularly important as that is the nucleus of the dynamical tessellation. 
Our tessellation factor for each side is going to be the length of each vector multiplied by the 
tessellation resolution, the higher the resolution, the more tessellated each edge is going to be.
Each tessellation factor is inverted (divided by one), not to make the tessellation too high.
Finally, each of the tessellation factors is then assigned to the output edges.
Calculation of dynamic tessellation
The final output function just passes on to the domain shader the input value for each patch 
and finally, the output is returned.
The domain shader receives data from both the hull shader and the tessellator such as the 
patch control points, the tessellation factor etc.
Here, in the matrix buffer, we receive not only the world matrix, view matrix and projection 
matrix but also the directional light and spotlight view matrices and projection matrices, which 
have been transposed in the TessellationShader.cpp.
Along with the main function, there is a getHeight function which is used to modify the height 
of the map based on the colours of the texture.
Analysing the main function, we create two variables to store the vertexes positions and the 
texture coordinates. For the vertexes positions, we lerp between first and second patch 
positions to get a point in the y coordinate and we do this again to obtain another pointer in 
the y coordinate between the third and fourth patch positions. Once these two points are 
obtained, we lerp between them on the x-axis to obtain the point on the x-axis.
The same is done for the texture coordinates.
Lerping and finding all the points for each patch.
The final calculations in the domain shader are to calculate the positions of the vertexes against 
the world, view, and projection matrices. The worldPosition is calculated before the calculation 
against the view matrix. Otherwise, it would no longer be the worldPosition.
Finally, we have the calculations of the vertices as viewed by the directional light and spotlight.
The output is then sent to the shadow_ps.
Before explaining the Lighting and therefore the shadow_ps, it is important to mention the 
waterShader class which takes care of the vertex manipulation for the water stream in the 
scene. The water stream is a resized plane, textured with a texture of water.
Once initialized in the App1.cpp and its parameters are set in the waterShader class, the data is 
sent to the water_vs where the vertex manipulation happens. The data sent contains not only 
all the different matrices needed for lighting and position but also the total time, based on delta 
time. 
Thanks to the use of the time and the application of a modified equation of a line, we obtain 
the effect of waves:
 input.position.y = 0.5 * sin ((input.position.x * 0.4) + time)
Desmos graph of the line equation for the water stream
Where 0.5 is the amplitude of the waves and 0.4 is the frequency. Time is needed as it allows 
the waves to move as it passes, time is elapsed time since start of the program. Amplitude can 
change height of the waves and frequency can increase the number of waves.

Lighting
One of the most important aspects of every graphics program is lighting and its calculation.
The application contains 3 different lights, a directional light, a point light and a spotlight.
All the elements in the scene are correctly lit and cast shadows. The only light that does not 
cast shadows is the point light. The user, while moving into the scene, will be able to change 
attributes – colour, direction etc. The application features also two other ortho meshes. These 
two show the view that the spotlight and directional light have on the scene.
This means that as we change the direction or position of the lights, the view of them on the 
scene will change.
Two different directions cast different 
shadows.
Along with the two ortho meshes, two shadow maps are created, one will hold the calculation 
for the shadows for the directional light and the other one for the spotlight.
The shadow maps’ depth maps are passed in the setShaderParameters() function for each 
element that has a shadow.
The same happens with lights. For each element that is lit, the different lights are passed in the 
setShaderParameters() function.
Before moving on to the shadow_ps and the light calculations, it is important to mention that 
two functions in the App1.cpp hold what the light sees. These two functions allow the shadows 
to be cast onto the scene.
First light shadow function.
The data stored in these functions will be rendered in the two orthomeshes.
The ShadowShader class has two buffers, one will be sent to the shadow_vs (matrices 
calculations) and one will be sent to the shadow_ps.
Shadow_ps connections.
The data sent to the shadow_ps buffer from the shader classes that communicate with it must 
be the same. In the shadow pixel shader, to differentiate the different data sent to it, the 
padding variable is used, if the padding is equal to one, then vertex manipulated textures will 
be calculated, otherwise normal models or meshes.
Along with the texture and the diffuse sampler, the two depth map textures and the shadow sampler 
are present.
Buffer, textures, and samplers
In the main function, we declare the three different starting values of the colour of the lights
and the texture colour, which for now is just equal to the texture.
Before doing the light calculation, we obtain the different vectors and values needed.
The spotlight vector is calculated by subtracting the spotlight position by the world position and 
then normalizing. Immediately after this, the distance from the world to the spotlight is 
calculated by extracting the length from the vector between the spotlight and the terrain.
The same is done for the point light.
The last step before calculating the lighting for the different models and shapes is to obtain the 
projected texture coordinates for the two lights.
Starting with the easiest case, if the shape that we want to calculate the lighting for is not 
manipulated or tessellated, we check for each light if the texture coordinates have depth data.
If there is map data and the shape is not in shadow, then we calculate the light.
To check if the shape is in shadow, we sample the shadow map and then calculate the depth 
from the light. A comparison between the depth of the shadow map and the depth of the light 
is done to determine whether to shadow or to light the pixel in consideration.
For the directional light, the calculateLighting() function is called, it takes in as parameters the 
light direction, the normals and of course the diffuse colour. What the function does is it takes 
the value of the intensity of the light by calculating the dot product between the normal and 
the light Direction (normalized)
Dot product between light direction and normal
Executing the dot product between the two vectors, the value of the intensity of the light is 
obtained. The colour will then be the diffuse colour multiplied by the intensity. This of course
must be saturated to clamp it between 0 and 1.
In the case of the spotlight, the calculateSpotLight() function is called.
What this function does is it obtains the strength of the illumination by calculating the dot 
product of the normal and spotlight vector, this returns a scalar, the intensity.
Another dot product between the spotlight direction and the spotlight vector is calculated to 
find out where the light lies.
Image from (Microsoft, 2021)
As Microsoft explains, the scalar returned by the dot product between the spotlight vector and 
spotlight direction is used to determine whether it is greater than the spotlight angle. If this is
the case, the light applied is at its maximum amount. if it is not, the pixel is beyond the cone 
and no light is cast.
(Microsoft, 2021)
Before the calculation of the spotlight colour value, the different values for the attenuation are 
declared, in this case, a small attenuation is applied.
The colour variable is created, and its value starts as black.
Finally, a check verifies if the scalar previously calculated (dotP) is bigger than the angle of the 
spotlight; if it is, then in the inner cone, there is light. 
The colour is equal to the diffuse colour multiplied by the intensity of the light. The attenuation 
formula is then applied to the colour before returning it.
For the manipulated geometries, the difference is in the calculation of the normals, since as the 
vertexes' position change, the normals change too.
To calculate the new normals, we first declare two floats that will hold the size of the texture.
We then call the texture.GetDimensions() function and pass in as parameters the just created 
floats. 
After getting the texture dimensions, we declare an offset for the texture coordinates, and that 
is going to decide how far the neighbour pixel is going to be.
First part of calculation of 
normals.
worldStep is the step mapped in the world coordinates, I found the values 5 for the uv offset 
and 40 for the worldStep to give the best aesthetic to the scene, but they can be changed.
After these calculations, we calculate the values of the height for each direction from the pixel 
(north, south, east, west) through the getHeight() function, like the one in the water_vs.
This time, we use the uv coordinates and the step in the texture coordinates we just calculated 
to find the height of the neighbouring pixels. We add to the texture coordinate, the uvOffset to 
reach the offset, depending on the direction.
Finally, we measure the difference of elevation of the near pixels in the u axis using the tangent 
and v axis using the bitangent.
To obtain the normal value, we calculate the cross product of the tangent and bitangent, which 
is going to give us a perpendicular vector to these two,
Calculate normals through cross between tangent and bitangent.
Normals output for debugging.
After calculating the normals, we use them to calculate the lights for the directional light and 
spotlight, the same way as the non-manipulated geometry.
After the calculation of the lights, we add them together and multiply them by the texture 
colour to obtain the final colour.
For manipulated 
geometry, calculation of light.

Conclusions
Research has been done to find a texture that fits well the vertex manipulation and dynamic 
tessellation. Where some textures brought more popping effect, I found the one implemented 
the most versatile, also for the implementation of the water stream.
More time could have been spent towards the work and perfection of the post-processing
technique chosen, the motion blur.
Motion blur can be achieved in different ways, where mine uses the difference between the 
mouse position and the centre of the screen, the same result could have been achieved using 
the difference between the view matrices.
Even though all the techniques work effectively, more time and care will be spent researching 
and improving all of them instead of focusing and improving only one.
I started researching more post-processing techniques such as DoF (depth of field) and shadow 
techniques such as Cascaded shadow maps. I started the implementation but understood that 
the deadline was too close to fully implement them.
The feature I would have added first after the ones I already implemented would have been the 
point light shadows, which would have been the same as a spotlight, but with multiple views.
The feature I will certainly consider applying again in the future will be the dynamic tessellation 
and animated vertex manipulation, researching more ways to reduce and minimize popping of 
the different vertexes. Although dynamic tessellation is the technique I researched the most, it 
is the one that caused more problems. In the first implementation, only the first point of each 
patch was calculated from the camera. This caused intense popping problems as the different 
edges of the patch were adjusted based on only one point. After more research, the different 
edges solution was implemented, minimizing the popping effect.

References
Desmos, n.d. [Online] 
Available at: https://www.desmos.com/calculator?lang=it
firoh, 2017. SketchFab:Panda. [Online] 
Available at: https://sketchfab.com/3d-models/panda-388c3230515848cb9c07724f2230af43
[Accessed 2022].
Luna, F. D., 2012. Introduction to 3d game programming with Directx 11. Dulles, VA :: Dulles, VA : 
Mercury Learning and Information.
Microsoft, 2021. Light Types (Direct3D 9). [Online] 
Available at: https://docs.microsoft.com/en-us/windows/win32/direct3d9/light-types
Zink, J., 2011. Practical Rendering and computation with Direct3D 11. Boca Raton, FL: Boca Raton, FL : 
CRC Press
