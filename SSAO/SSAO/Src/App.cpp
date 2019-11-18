#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include<iostream>
#include <vector>
#include <map>
#include <random>

#include "Renderer.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Shader.h"
#include "My_Texture.h"
#include "camera.h"
#include "Cubemap.h"
#include "Model.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

//---------------------------------
/* Settings and Variables: */

//Functions definition
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// Window settings
const unsigned int SCR_WIDTH = 1280;		// Before: 800
const unsigned int SCR_HEIGHT = 720;	// Before: 600

//Paths:
std::string ShaderPath_Model = "Res/shaders/model_shader.shader";
std::string ShaderPath_Geometry = "Res/shaders/shader_geometry.shader";
std::string ShaderPath_SSAO = "Res/shaders/shader_ssao.shader";
std::string ShaderPath_Lighting = "Res/shaders/shader_lighting.shader";
std::string ShaderPath_SSAOBlur = "Res/shaders/shader_ssaoblur.shader";
std::string ShaderPath_SSAOTexture = "Res/shaders/shader_ssaoTexture.shader";

std::string ShaderPath_SAAO_X = "Res/shaders/shader_saao_x.shader";
std::string ShaderPath_SAAO_Y = "Res/shaders/shader_saao_y.shader";
std::string ShaderPath_SAAOTexture = "Res/shaders/shader_saaoTexture.shader";
std::string ShaderPath_SAAOBlur_X = "Res/shaders/shader_saaoblur_x.shader";
std::string ShaderPath_SAAOBlur_Y = "Res/shaders/shader_saaoblur_y.shader";
std::string ShaderPath_SAAOBlurTexture = "Res/shaders/shader_saaoBlurTexture.shader";
std::string ShaderPath_saaoLighting = "Res/shaders/shader_saao_lighting.shader";

std::string ShaderPath_quad = "Res/shaders/shader_quad.shader";

//Timing:
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
float t_geometry, t_ssao, t_ssaoblur, t_lighting,t_time;
int frameNumber = 0;

//Camera
Camera camera(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
bool movecamera = true;
bool setcursorpos = false;

//Mouse:
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//GBuffer
unsigned int gBuffer;							// G-Buffer variable
unsigned int gPosition, gNormal, gAlbedo;		// Position colorbuffer, Normal colorbuffer, Albedo colorbuffer(color+ specular) ->All collected inside the G-Buffer
unsigned int depthRBO; 							// Depth render buffer object

//Kernel Texture
unsigned int noiseText; 						// Texture that holds the noise

//SSAO Framebuffer
unsigned int ssaoFBO, ssaoCB;
unsigned int ssaoBFBO, ssaoBCB;

//SAAO Framebuffer
unsigned int saaoFBO[2], saaoBFBO[2];
unsigned int saaoCB[2], saaoBCB[2];

int samplesX = 8;
int samplesY = 8;

//Kernel variables
int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); //Generates random Floats between 0.0 and 1.0
std::default_random_engine generator; //Random engine class that generates pseudo-random numbers
std::vector<glm::vec3> ssaoKernel; //SSAO kernel vector

float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

//Noise texture
std::vector<glm::vec3> ssaoNoise;

//Light Properties:
glm::vec3 lightPos = glm::vec3(3.0, 4.0, -2.0);
glm::vec3 lightColor = glm::vec3(1.0, 1.0, 1.0);
const float constant = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
const float linear = 0.09;
const float quadratic = 0.032;

ImVec4 testingcolor = ImVec4(lightColor.x, lightColor.y, lightColor.z, 1.0);


//ImGui Var
bool SSAObool = false;
bool SSAOBlurbool = false;
bool SSAOLightingbool = true;
bool SAAObool = false;

//---------------------------------

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "FRR Lab 2: SSAO", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	/*Mouse Capture*/
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	if (glewInit() != GLEW_OK)
		std::cout << "Error!" << std::endl;

	//Enable Depth test and Blend
	glEnable(GL_DEPTH_TEST);   
	glDisable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);




	//---------------------------------
	float cube_vertices[] = {
		// back face
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
		// front face
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		// left face
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		// right face
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
		// bottom face
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		// top face
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
		 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
	};

	float quad_vertices[] = { 
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f, 0.0f,
	 1.0f,  1.0f,  0.0f, 1.0f, 1.0f,

	 1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
	 1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f, 0.0f
	};
	//---------------------------------

	//Cube VAO and VBO
	VertexArray cube_va;
	VertexBuffer cube_vb(cube_vertices, sizeof(cube_vertices));
	VertexBufferLayout cube_layout;

	cube_layout.Push<float>(3); //Position
	cube_layout.Push<float>(3); //Normal
	cube_layout.Push<float>(2); //Texture
	cube_va.AddBuffer(cube_vb, cube_layout);

	//Quad VAO and VBO
	VertexArray quad_va;
	VertexBuffer quad_vb(quad_vertices, sizeof(quad_vertices));
	VertexBufferLayout quad_layout;

	quad_layout.Push<float>(3); //Position
	quad_layout.Push<float>(2); //Texture
	quad_va.AddBuffer(quad_vb, quad_layout);

	//GBuffer
	{
		//G-Buffer Generation:
		GLCall(glGenFramebuffers(1, &gBuffer));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, gBuffer));

		//Position colorbuffer textture configuration
		GLCall(glGenTextures(1, &gPosition));
		GLCall(glBindTexture(GL_TEXTURE_2D, gPosition));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));	//Prevents the oversample position/depth values in screen-space
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

		//Attach the Position buffer into the currently bound Framebuffer Object.
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0));

		//Normal colorbuffer textture configuration
		GLCall(glGenTextures(1, &gNormal));
		GLCall(glBindTexture(GL_TEXTURE_2D, gNormal));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

		//Attach the Normal buffer into the currently bound Framebuffer Object.
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0));

		//Albedo colorbuffer texture configuration
		GLCall(glGenTextures(1, &gAlbedo));
		GLCall(glBindTexture(GL_TEXTURE_2D, gAlbedo));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

		//Attach the Albedo buffer into the currently bound Framebuffer Object.
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0));

		//Specify the list of color buffers to be drawn into:
		unsigned int colorbuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		GLCall(glDrawBuffers(3, colorbuffers));

		//Create a depth renderbuffer object 
		GLCall(glGenRenderbuffers(1, &depthRBO));
		GLCall(glBindRenderbuffer(GL_RENDERBUFFER, depthRBO));
		GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT));

		//Attach the Depth renderbuffer 
		GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO));

		//Finally we check if the framebuffer created is completed or not
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			cout << "The Framebuffer is not complete!" << endl;
			cout << "Status: " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << endl;
		}

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0)); 	//Unbind the framebuffer created
	}

	//SSAO Buffer
	{
		GLCall(glGenFramebuffers(1, &ssaoFBO));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO));

		//Color buffer configuration
		GLCall(glGenTextures(1, &ssaoCB));
		GLCall(glBindTexture(GL_TEXTURE_2D, ssaoCB));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

		//Attach the Position buffer into the currently bound Framebuffer Object.
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoCB, 0));

		//Finally we check if the framebuffer created is completed or not
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			cout << "The SAAO Framebuffer is not complete!" << endl;
			cout << "Status: " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << endl;
		}

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0)); 	//Unbind the framebuffer created
	}

	//Separable Aproximation AO Framebuffers and Color buffers
	GLCall(glGenFramebuffers(2, saaoFBO));
	GLCall(glGenTextures(2, saaoCB));

	for (unsigned int i = 0; i < 2; i++)
	{
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, saaoFBO[i]));
		GLCall(glBindTexture(GL_TEXTURE_2D, saaoCB[i]));

		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, saaoCB[i], 0));

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			cout << "The saao Framebuffer is not complete!" << endl;
			cout << "Status: " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << endl;
		}
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	//Sample Kernel
	{
		for (unsigned int i = 0; i < kernelSize; ++i)
		{
			glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0,   // x between -1.0 and 1.0
				randomFloats(generator) * 2.0 - 1.0,				// y between -1.0 and 1.0
				randomFloats(generator));						    // z between  0.0 and 1.0 -> Semisphere!

			sample = glm::normalize(sample);
			sample *= randomFloats(generator);
			float scale = float(i) / (float)kernelSize;

			//Scales the samples to be more aligned to the center of the kernel
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			ssaoKernel.push_back(sample);
		}
	}

	//Noise Texture
	{
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0,  // x between - 1.0 and 1.0
				randomFloats(generator) * 2.0 - 1.0,  // y between - 1.0 and 1.0
				0.0f);								  // z = 0.0
			ssaoNoise.push_back(noise);
		}

		//Creation of the texture that holds the random rotation vectors
		GLCall(glGenTextures(1, &noiseText));
		GLCall(glBindTexture(GL_TEXTURE_2D, noiseText));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	}

	//SSAO Blur Framebuffer
	{
		GLCall(glGenFramebuffers(1, &ssaoBFBO));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, ssaoBFBO));

		//Color buffer texture (Blur)
		GLCall(glGenTextures(1, &ssaoBCB));
		GLCall(glBindTexture(GL_TEXTURE_2D, ssaoBCB));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

		//Attach the Position buffer into the currently bound Framebuffer Object.
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBCB, 0));

		//Finally we check if the framebuffer created is completed or not
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	//Gaussian Blur
	glGenFramebuffers(2, saaoBFBO);
	glGenTextures(2, saaoBCB);
	for (unsigned int i = 0; i < 2; i++)
	{
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, saaoBFBO[i]));
		GLCall(glBindTexture(GL_TEXTURE_2D, saaoBCB[i]));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, saaoBCB[i], 0));

		//Finally we check if the framebuffer created is completed or not
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;

	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	//Load Model
	Model ourModel("Res/cat/12223_Cat_v1_l3.obj");

	//Shader init
	Shader shader_model(ShaderPath_Model); 
	shader_model.Bind();

	Shader shader_quad(ShaderPath_quad);
	shader_quad.Bind();

	Shader shader_geometry(ShaderPath_Geometry); //Geometry shader
	shader_geometry.Bind();

	Shader shader_ssao(ShaderPath_SSAO); //SSAO shader
	shader_ssao.Bind();
	shader_ssao.SetUniform1i("gPosition", 0);
	shader_ssao.SetUniform1i("gNormal", 1);
	shader_ssao.SetUniform1i("noiseText", 2);

	Shader shader_lighting(ShaderPath_Lighting); //Lighting shader
	shader_lighting.Bind();
	shader_lighting.SetUniform1i("gPosition", 0);
	shader_lighting.SetUniform1i("gNormal", 1);
	shader_lighting.SetUniform1i("gAlbedo", 2);
	shader_lighting.SetUniform1i("ssao", 3);

	Shader shader_ssaoblur(ShaderPath_SSAOBlur); //SSAO Blur shader
	shader_ssaoblur.Bind();
	shader_ssaoblur.SetUniform1i("ssao", 0);

	Shader shader_ssaoTexture(ShaderPath_SSAOTexture); //SSAO Texture shader
	shader_ssaoTexture.Bind();
	shader_ssaoTexture.SetUniform1i("ssao", 0);

	Shader shader_saao_x(ShaderPath_SAAO_X); //SAAO X component shader
	shader_saao_x.Bind();
	shader_saao_x.SetUniform1i("gPosition", 0);
	shader_saao_x.SetUniform1i("gNormal", 1);

	Shader shader_saao_y(ShaderPath_SAAO_Y); //SAAO Y component shader
	shader_saao_y.Bind();
	shader_saao_y.SetUniform1i("gPosition", 0);
	shader_saao_y.SetUniform1i("gNormal", 1);
	shader_saao_y.SetUniform1i("saao_x", 2);

	Shader shader_saaoTexture(ShaderPath_SAAOTexture); //SAAO Texture shader
	shader_saaoTexture.Bind();
	shader_saaoTexture.SetUniform1i("saao_y", 0);

	Shader shader_saaoblur_x(ShaderPath_SAAOBlur_X); //SAAO Blur X
	shader_saaoblur_x.Bind();
	shader_saaoblur_x.SetUniform1i("saao", 0);

	Shader shader_saaoblur_y(ShaderPath_SAAOBlur_Y); //SAAO Blur Y
	shader_saaoblur_y.Bind();
	shader_saaoblur_y.SetUniform1i("saao", 0);

	Shader shader_saaoblur(ShaderPath_SAAOBlurTexture); //SAAO Blur Texture
	shader_saaoblur.Bind();
	shader_saaoblur.SetUniform1i("saaoblur_x", 0);
	shader_saaoblur.SetUniform1i("saaoblur_y", 1);

	Shader shader_saaolighting(ShaderPath_saaoLighting); //Lighting shader
	shader_saaolighting.Bind();
	shader_saaolighting.SetUniform1i("gPosition", 0);
	shader_saaolighting.SetUniform1i("gNormal", 1);
	shader_saaolighting.SetUniform1i("gAlbedo", 2);
	shader_saaolighting.SetUniform1i("saaoblur_x", 3);
	shader_saaolighting.SetUniform1i("saaoblur_y", 4);

	//Renderer
	Renderer renderer;

	ImGui::CreateContext();
	ImGui_ImplOpenGL3_Init();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui::StyleColorsDark();

	//---------------------------------
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		//Per-frame time logic
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//Input
		processInput(window);

		glEnable(GL_DEPTH_TEST);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			//Geometry Draw---------------------------------------------------------------
			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, gBuffer));
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
				glm::mat4 view = camera.GetViewMatrix();
				glm::mat4 model = glm::mat4(1.0f);
				glm::mat4 mvp = proj * view * model;

				//Cube
				model = glm::rotate(model, glm::radians(-50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::translate(model, glm::vec3(0.0f, 4.5f, -0.5f));
				model = glm::scale(model, glm::vec3(5.0f, 5.0f, 5.0f));
				mvp = proj * view * model;

				shader_geometry.Bind();
				shader_geometry.SetUniformMat4f("view", view);
				shader_geometry.SetUniformMat4f("model", model);
				shader_geometry.SetUniformMat4f("MVP", mvp);
				shader_geometry.SetUniform1i("iNormals", 1);

				renderer.Draw_NoIb(cube_va, shader_geometry);

				//Model
				model = glm::mat4(1.0f);
				model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.5f));
				model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
				mvp = proj * view * model;

				shader_geometry.Bind();
				shader_geometry.SetUniformMat4f("view", view);
				shader_geometry.SetUniformMat4f("model", model);
				shader_geometry.SetUniformMat4f("MVP", mvp);
				shader_geometry.SetUniform1i("iNormals", 0);

				ourModel.Draw(shader_geometry);
			GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

			//SSAO Texture Draw ----------------------------------------------------------
			if (SSAObool == true && SSAOBlurbool == false && SAAObool == false)
			{
				//SSAO Draw-------------------------------------------------------------------
				GLCall(glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO));
				glClear(GL_COLOR_BUFFER_BIT);

				shader_ssao.Bind();
				shader_ssao.SetUniformMat4f("projection", proj);
				shader_ssao.SetUniform1i("kernelSize", kernelSize);
				shader_ssao.SetUniform1f("radius", radius);
				shader_ssao.SetUniform1f("bias", bias);
				for (unsigned int i = 0; i < 64; ++i)
				{
					shader_ssao.SetUniform3fv("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
				}

				GLCall(glActiveTexture(GL_TEXTURE0));
				GLCall(glBindTexture(GL_TEXTURE_2D, gPosition));
				GLCall(glActiveTexture(GL_TEXTURE1));
				GLCall(glBindTexture(GL_TEXTURE_2D, gNormal));
				GLCall(glActiveTexture(GL_TEXTURE2));
				GLCall(glBindTexture(GL_TEXTURE_2D, noiseText));

				renderer.Draw_NoIb(quad_va, shader_ssao);

				GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
				//SSAO Texture Draw----------------------------------------------------------
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				shader_ssaoTexture.Bind();

				GLCall(glActiveTexture(GL_TEXTURE0));
				GLCall(glBindTexture(GL_TEXTURE_2D, ssaoCB));

				renderer.Draw_NoIb(quad_va, shader_ssaoTexture);

			}
			//SSAO Blur Texture Draw -----------------------------------------------------
			else if (SSAOBlurbool == true && SSAObool == false && SAAObool == false)
			{
				//SSAO Draw-------------------------------------------------------------------
				GLCall(glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO));
				glClear(GL_COLOR_BUFFER_BIT);

				shader_ssao.Bind();
				shader_ssao.SetUniformMat4f("projection", proj);
				shader_ssao.SetUniform1i("kernelSize", kernelSize);
				shader_ssao.SetUniform1f("radius", radius);
				shader_ssao.SetUniform1f("bias", bias);
				for (unsigned int i = 0; i < 64; ++i)
				{
					shader_ssao.SetUniform3fv("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
				}

				GLCall(glActiveTexture(GL_TEXTURE0));
				GLCall(glBindTexture(GL_TEXTURE_2D, gPosition));
				GLCall(glActiveTexture(GL_TEXTURE1));
				GLCall(glBindTexture(GL_TEXTURE_2D, gNormal));
				GLCall(glActiveTexture(GL_TEXTURE2));
				GLCall(glBindTexture(GL_TEXTURE_2D, noiseText));

				renderer.Draw_NoIb(quad_va, shader_ssao);

				//SSAO Blur Draw -------------------------------------------------------------
				GLCall(glBindFramebuffer(GL_FRAMEBUFFER, ssaoBFBO));
				glClear(GL_COLOR_BUFFER_BIT);

				shader_ssaoblur.Bind();
				GLCall(glActiveTexture(GL_TEXTURE0));
				GLCall(glBindTexture(GL_TEXTURE_2D, ssaoCB));

				renderer.Draw_NoIb(quad_va, shader_ssaoblur);

				GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

				t_ssaoblur = t_lighting = (glfwGetTime() - currentFrame - (t_geometry / 1000.0) - (t_ssao / 1000.0))* 1000.0;

				//Lighting--------------------------------------------------------------------
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				shader_ssaoTexture.Bind();

				GLCall(glActiveTexture(GL_TEXTURE0));
				GLCall(glBindTexture(GL_TEXTURE_2D, ssaoBCB));

				renderer.Draw_NoIb(quad_va, shader_ssaoTexture);
			}
			//Separable Aproximation Ambient Occlusion
			else if (SAAObool == true && SSAObool == false && SSAOBlurbool == false)
			{

				//SAAO Draw-------------------------------------------------------------------
				for (int i = 0; i < 2; i++)
				{
					GLCall(glBindFramebuffer(GL_FRAMEBUFFER, saaoFBO[i]));
					glClear(GL_COLOR_BUFFER_BIT);

					if (i == 0)
					{
						shader_saao_x.Bind();
						shader_saao_x.SetUniform1i("samplesX", samplesX);
						shader_saao_x.SetUniform1f("radius", radius);
						shader_saao_x.SetUniform1f("bias", bias);
						shader_saao_x.SetUniformMat4f("projection", proj);
						for (unsigned int i = 0; i < 64; ++i)
						{
							shader_saao_x.SetUniform3fv("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
						}
					}
					else
					{
						shader_saao_y.Bind();
						shader_saao_y.SetUniform1i("samplesY", samplesY);
						shader_saao_y.SetUniform1f("radius", radius);
						shader_saao_y.SetUniform1f("bias", bias);
						shader_saao_y.SetUniformMat4f("projection", proj);
						for (unsigned int i = 0; i < 64; ++i)
						{
							shader_saao_y.SetUniform3fv("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
						}
					}

					GLCall(glActiveTexture(GL_TEXTURE0));
					GLCall(glBindTexture(GL_TEXTURE_2D, gPosition));
					GLCall(glActiveTexture(GL_TEXTURE1));
					GLCall(glBindTexture(GL_TEXTURE_2D, gNormal));
					if (i == 1)
					{
						GLCall(glActiveTexture(GL_TEXTURE2));
						GLCall(glBindTexture(GL_TEXTURE_2D, saaoCB[0]));
					}

					if (i == 0)
						renderer.Draw_NoIb(quad_va, shader_saao_x);
					else
						renderer.Draw_NoIb(quad_va, shader_saao_y);
				}

				GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
				//----------------------------------------------------------
				//Code to show the texture -> Comment all the SAAO code below and discomment this part.

				//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				//shader_saaoTexture.Bind();

				//GLCall(glActiveTexture(GL_TEXTURE0));
				//GLCall(glBindTexture(GL_TEXTURE_2D, saaoCB[1]));


				//renderer.Draw_NoIb(quad_va, shader_saaoTexture);

				//Gaussian Blur Draw----------------------------------------
				for (int i = 0; i < 2; i++)
				{
					if (i % 2 == 1)
					{
						GLCall(glBindFramebuffer(GL_FRAMEBUFFER, saaoBFBO[0]));
						glClear(GL_COLOR_BUFFER_BIT);
						shader_saaoblur_x.Bind();
						GLCall(glActiveTexture(GL_TEXTURE0));
						GLCall(glBindTexture(GL_TEXTURE_2D, saaoCB[1]));

						renderer.Draw_NoIb(quad_va, shader_saaoblur_x);
					}
					else
					{
						GLCall(glBindFramebuffer(GL_FRAMEBUFFER, saaoBFBO[1]));
						glClear(GL_COLOR_BUFFER_BIT);
						shader_saaoblur_y.Bind();
						GLCall(glActiveTexture(GL_TEXTURE0));
						GLCall(glBindTexture(GL_TEXTURE_2D, saaoCB[1]));

						renderer.Draw_NoIb(quad_va, shader_saaoblur_y);
					}

					GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
				}

				//----------------------
				//Code to show the saao blur texture -> Comment all the SAAO code below and discomment this part.

				//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				//shader_saaoblur.Bind();
				//
				//GLCall(glActiveTexture(GL_TEXTURE0));
				//GLCall(glBindTexture(GL_TEXTURE_2D, saaoBCB[0]));
				//GLCall(glActiveTexture(GL_TEXTURE1));
				//GLCall(glBindTexture(GL_TEXTURE_2D, saaoBCB[1]));

				//renderer.Draw_NoIb(quad_va, shader_saaoblur);

				//Lighting Draw----------------------------------------
				glm::vec3 lightPosView = glm::vec3(camera.GetViewMatrix() * glm::vec4(lightPos, 1.0));

				shader_saaolighting.Bind();
				shader_saaolighting.SetUniform3fv("light.Position", lightPosView);
				shader_saaolighting.SetUniform3fv("light.Color", lightColor);
				shader_saaolighting.SetUniform1f("light.linear", linear);
				shader_saaolighting.SetUniform1f("light.quadratic", quadratic);

				GLCall(glActiveTexture(GL_TEXTURE0));
				GLCall(glBindTexture(GL_TEXTURE_2D, gPosition));
				GLCall(glActiveTexture(GL_TEXTURE1));
				GLCall(glBindTexture(GL_TEXTURE_2D, gNormal));
				GLCall(glActiveTexture(GL_TEXTURE2));
				GLCall(glBindTexture(GL_TEXTURE_2D, gAlbedo));
				GLCall(glActiveTexture(GL_TEXTURE3));
				GLCall(glBindTexture(GL_TEXTURE_2D, saaoBCB[0]));
				GLCall(glActiveTexture(GL_TEXTURE4));
				GLCall(glBindTexture(GL_TEXTURE_2D, saaoBCB[1]));

				renderer.Draw_NoIb(quad_va, shader_saaolighting);

			}
			//SSAO Blur Lighting Draw-----------------------------------------------------
			else
			{	
				GLCall(glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO));
				glClear(GL_COLOR_BUFFER_BIT);

				shader_ssao.Bind();
				shader_ssao.SetUniformMat4f("projection", proj);
				shader_ssao.SetUniform1i("kernelSize", kernelSize);
				shader_ssao.SetUniform1f("radius", radius);
				shader_ssao.SetUniform1f("bias", bias);
				for (unsigned int i = 0; i < 64; ++i)
				{
					shader_ssao.SetUniform3fv("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
				}

				GLCall(glActiveTexture(GL_TEXTURE0));
				GLCall(glBindTexture(GL_TEXTURE_2D, gPosition));
				GLCall(glActiveTexture(GL_TEXTURE1));
				GLCall(glBindTexture(GL_TEXTURE_2D, gNormal));
				GLCall(glActiveTexture(GL_TEXTURE2));
				GLCall(glBindTexture(GL_TEXTURE_2D, noiseText));

				renderer.Draw_NoIb(quad_va, shader_ssao);

				GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
				//SSAO Blur Draw -------------------------------------------------------------
				GLCall(glBindFramebuffer(GL_FRAMEBUFFER, ssaoBFBO));
				glClear(GL_COLOR_BUFFER_BIT);

				shader_ssaoblur.Bind();
				GLCall(glActiveTexture(GL_TEXTURE0));
				GLCall(glBindTexture(GL_TEXTURE_2D, ssaoCB));

				renderer.Draw_NoIb(quad_va, shader_ssaoblur);

				GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

				//Lighting--------------------------------------------------------------------
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				glm::vec3 lightPosView = glm::vec3(camera.GetViewMatrix() * glm::vec4(lightPos, 1.0));

				shader_lighting.Bind();
				shader_lighting.SetUniform3fv("light.Position", lightPosView);
				shader_lighting.SetUniform3fv("light.Color", lightColor);
				shader_lighting.SetUniform1f("light.linear", linear);
				shader_lighting.SetUniform1f("light.quadratic", quadratic);

				GLCall(glActiveTexture(GL_TEXTURE0));
				GLCall(glBindTexture(GL_TEXTURE_2D, gPosition));
				GLCall(glActiveTexture(GL_TEXTURE1));
				GLCall(glBindTexture(GL_TEXTURE_2D, gNormal));
				GLCall(glActiveTexture(GL_TEXTURE2));
				GLCall(glBindTexture(GL_TEXTURE_2D, gAlbedo));
				GLCall(glActiveTexture(GL_TEXTURE3));
				GLCall(glBindTexture(GL_TEXTURE_2D, ssaoBCB));

				renderer.Draw_NoIb(quad_va, shader_lighting);
			}

			//ImGui-----------------------------------------------------------------------
			ImGui::Begin("SSAO");
				ImGui::SetWindowPos(ImVec2(10.0, 10.0));
				ImGui::SetWindowSize(ImVec2(350.0, 390.0));

				ImGui::CollapsingHeader("Application Info");

					if (SSAObool == true && SSAOBlurbool == false && SAAObool == false)
						ImGui::Text("Mode: SSAO Texture");
					else if (SSAObool == false && SSAOBlurbool == true && SAAObool == false )
						ImGui::Text("Mode: SSAO Blur Texture");
					else if (SSAObool == false && SSAOBlurbool == false && SAAObool == true)
						ImGui::Text("Mode: SAAO ");
					else
						ImGui::Text("Mode: SSAO Blur Lighting");

					ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

				ImGui::CollapsingHeader("Modes");

					ImGui::Checkbox("SSAO", &SSAObool);
					ImGui::Checkbox("SSAO Blur", &SSAOBlurbool);
					ImGui::Checkbox("SAAO", &SAAObool);

				ImGui::CollapsingHeader("General Variables");

					ImGui::ColorEdit3("Light Color", (float*)& testingcolor);
					lightColor = glm::vec3((float)testingcolor.x, (float)testingcolor.y, (float)testingcolor.z);

					ImGui::SliderFloat("Bias", &bias, 0.01, 0.5);
					ImGui::SliderFloat("Radius", &radius, 0.1, 1.0);

				ImGui::CollapsingHeader("SSAO Variables");
			
					ImGui::SliderInt("Samples", &kernelSize, 16, 64);

				ImGui::CollapsingHeader("SAAO Variables");

					ImGui::SliderInt("samples X", &samplesX, 4, 32);
					ImGui::SliderInt("samples Y", &samplesY, 4, 32);

			ImGui::End();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		}

		frameNumber += 1;

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

//Key Orders:
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	//Enable disable Cursor
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		movecamera = false;
		setcursorpos = true;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		movecamera = true;
		if (setcursorpos == true)
		{
			glfwSetCursorPos(window, lastX, lastY); //When the curso dissapear returns it to the position it was before
			setcursorpos = false;
		}
	}

}

//Changing the window size:
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

//Mouse call:
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if(movecamera)
	{
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

		lastX = xpos;
		lastY = ypos;

		camera.ProcessMouseMovement(xoffset, yoffset, true);
	}
}

//Scroll call:
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
