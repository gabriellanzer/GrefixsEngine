
// StdLib Includes
#include <vector>
#include <string>
#include <fstream>

// Third Party Includes
#include <volk.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <fmt/core.h>
#include <PerlinNoise.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Application Specific Includes
#include <app/appOpenGL.h>
#include <render/utilsOpenGL.h>

// Using directives
using std::string;
template <typename T>
using vector = std::vector<T>;

void OpenGL_App::Setup()
{
	// Setup Graphics APIs
	glfwSetErrorCallback(OnGlfwErrorCallback);
	if (!glfwInit())
	{
		return;
	}

	_window = glfwCreateWindow(1920, 1080, GetName(), nullptr, nullptr);
	if (!_window)
	{
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(_window);
	glfwSwapInterval(1);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glfwSetInputMode(_window, GLFW_STICKY_KEYS, GLFW_TRUE);

	// Operating System Window Settings
	glfwSetWindowSizeLimits(_window, 640, 480, GLFW_DONT_CARE, GLFW_DONT_CARE);
	glfwSetWindowSizeCallback(_window, OnGlfwWindowResizeCallback);
	glfwMaximizeWindow(_window);

	// Initial viewport parameters
	glClearColor(0.0f, 0.0f, 0.4f, 1.0f);

	glGenVertexArrays(1, &_exampleVAO);

	glBindVertexArray(_exampleVAO);

	GLuint vertexBufId;
	glGenBuffers(1, &vertexBufId);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufId);

	const float quad[] = {
		-0.5f, -0.5f, 0.0f, // v0
		+0.5f, -0.5f, 0.0f, // v1
		-0.5f, +0.5f, 0.0f, // v2
		-0.5f, +0.5f, 0.0f, // v3
		+0.5f, -0.5f, 0.0f, // v4
		+0.5f, +0.5f, 0.0f, // v5
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(float) * 3, 0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Shader Compilation
	RenderUtils::InitGlslang();

	string vertData, fragData;
	vector<unsigned int> vertSpirv, fragSpirv;
	if (RenderUtils::TryLoadShaderFile("../../shaders/vert_col_gl.vs", vertData) &&
		RenderUtils::TryLoadShaderFile("../../shaders/vert_col_gl.fs", fragData))
	{
		RenderUtils::GLSLtoSPV(VK_SHADER_STAGE_VERTEX_BIT, vertData.c_str(), vertSpirv, RenderUtils::ShaderCompileTarget::OpenGL);
		RenderUtils::GLSLtoSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fragData.c_str(), fragSpirv, RenderUtils::ShaderCompileTarget::OpenGL);
		_exampleShader = RenderUtils::OpenGL::CompileShaderProgramSpirV(vertSpirv, fragSpirv);
	}
}

void OpenGL_App::Awake() {}

void OpenGL_App::Sleep() {}

void OpenGL_App::Shutdown()
{
	RenderUtils::FinalizeGlslang();

	// Graphics API shutdown
	glfwDestroyWindow(_window);
	glfwTerminate();
}

void OpenGL_App::Update(double deltaTime)
{
	if (glfwWindowShouldClose(_window))
	{
		_shouldQuit = true;
		return;
	}

	// Poll first so ImGUI has the events.
	// This performs some callbacks as well
	glfwPollEvents();

	// GL Rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawAppScreen(deltaTime);

	glfwSwapBuffers(_window);
}

void OpenGL_App::DrawAppScreen(double deltaTime)
{
	static float accTime = 0.0f;
	accTime += (float)deltaTime;

	const siv::PerlinNoise::seed_type seed = 123456u;

	const siv::PerlinNoise perlin{seed};

	glm::mat4 view = glm::lookAt(glm::vec3{0.0f, 0.0f, 5.0f}, glm::vec3{}, glm::vec3{0.0f, 1.0f, 0.0f});
	glm::mat4 proj = glm::perspective(60.0f, 4 / 3.0f, 0.01f, 1000.0f);
	glm::mat4 viewProj = proj * view;

	for (int x = -50; x < 50; x++)
	{
		for (int y = -50; y < 50; y++)
		{
			const float perlinVal = (float)perlin.octave2D(x + cos(accTime), y + sin(accTime), 2);

			glUseProgram(_exampleShader);
			glUniform1f(2, accTime);
			glUniform1f(3, perlinVal);

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, {x, y, 0.0f});
			model = glm::rotate(model, perlinVal, glm::vec3{0.0f, 0.0f, 1.0f});
			model = glm::scale(model, glm::vec3(0.5));

			glUniformMatrix4fv(0, 1, false, &viewProj[0][0]);
			glUniformMatrix4fv(1, 1, false, &model[0][0]);

			glBindVertexArray(_exampleVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}
}
