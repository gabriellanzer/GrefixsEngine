
// TODOs:
// - Spir-V
//  - Compile and Run
//  - Reflection
// - Shader Manager (look reference code - twitter)
//  - Uniform Buffer API
//  - Textures API
//   - Descriptor
// -
// - Mesh Descriptor API

// StdLib Includes
#include <vector>
#include <string>
#include <fstream>

// Third Party Includes
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <fmt/core.h>
#include <PerlinNoise.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Application Specific Includes
#include <app/app.h>
#include <utils/spirv.h>

// Using directives
using std::string;
template <typename T>
using vector = std::vector<T>;
using std::ios;
using std::istreambuf_iterator;
using vk::ShaderStageFlagBits;

static bool TryLoadShaderFile(string shaderPath, string& outShaderData)
{
	std::ifstream in;
	in.open(shaderPath.c_str(), ios::in);
	if (!in)
	{
		return false;
	}
	outShaderData.assign((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
	in.close();
	return true;
}

static GLuint CompileShaderProgram(string vertData, string fragData)
{
	bool valid = !vertData.empty() && !fragData.empty();

	// compile
	const char* c_str;
	uint32_t vid;
	if (valid)
	{
		vid = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vid, 1, &(c_str = vertData.c_str()), NULL);
		glCompileShader(vid);
	}

	uint32_t fid;
	if (valid)
	{
		fid = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fid, 1, &(c_str = fragData.c_str()), NULL);
		glCompileShader(fid);
	}

	// link
	uint32_t pid = glCreateProgram();
	if (valid) glAttachShader(pid, vid);
	if (valid) glAttachShader(pid, fid);
	glLinkProgram(pid);

	// log
	char logStr[1024];
	glGetProgramInfoLog(pid, 1024, NULL, logStr);
	fmt::print("Compile Shader Status: {0}\n", logStr);

	// clean
	if (valid) glDetachShader(pid, vid);
	if (valid) glDetachShader(pid, fid);

	if (valid) glDeleteShader(vid);
	if (valid) glDeleteShader(fid);

	return pid;
}

void GrefixsEndine::Setup()
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

	float triangle[] = {
		-0.5f, -0.5f, 0.0f, +0.5f, -0.5f, 0.0f, -0.5f, +0.5f, 0.0f,
		-0.5f, +0.5f, 0.0f, +0.5f, -0.5f, 0.0f, +0.5f, +0.5f, 0.0f,
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(float) * 3, 0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Shader Compilation
	SpirvUtils::Init();

	string vertData, fragData;
	vector<unsigned int> vertSpirv, fragSpirv;
	if (TryLoadShaderFile("../../shaders/vert_col.vs", vertData) &&
		TryLoadShaderFile("../../shaders/vert_col.fs", fragData))
	{
		_exampleShader = CompileShaderProgram(vertData, fragData);
		SpirvUtils::GLSLtoSPV(ShaderStageFlagBits::eVertex, vertData.c_str(), vertSpirv);
		SpirvUtils::GLSLtoSPV(ShaderStageFlagBits::eFragment, fragData.c_str(), fragSpirv);
	}
}

void GrefixsEndine::Awake() {}

void GrefixsEndine::Sleep() {}

void GrefixsEndine::Shutdown()
{
	SpirvUtils::Finalize();

	// Graphics API shutdown
	glfwDestroyWindow(_window);
	glfwTerminate();
}

void GrefixsEndine::Update(double deltaTime)
{
	shouldQuit = glfwWindowShouldClose(_window);

	// Poll first so ImGUI has the events.
	// This performs some callbacks as well
	glfwPollEvents();

	// GL Rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawAppScreen(deltaTime);

	glfwSwapBuffers(_window);
}

void GrefixsEndine::DrawAppScreen(double deltaTime)
{
	static double accTime = 0.0;
	accTime += deltaTime;

	const siv::PerlinNoise::seed_type seed = 123456u;

	const siv::PerlinNoise perlin{seed};

	glm::mat4 view = glm::lookAt(glm::vec3{0.0f, 0.0f, 5.0f}, glm::vec3{}, glm::vec3{0.0f, 1.0f, 0.0f});
	glm::mat4 proj = glm::perspective(60.0f, 4 / 3.0f, 0.01f, 1000.0f);
	glm::mat4 viewProj = proj * view;

	for (int x = -50; x < 50; x++)
	{
		for (int y = -50; y < 50; y++)
		{
			const float perlinVal = perlin.octave2D(x + cos(accTime), y + sin(accTime), 2);

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
