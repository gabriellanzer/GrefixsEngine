#ifndef __APP__H__
#define __APP__H__

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>

#include <core/iapp.h>

class OpenGL_App : public gefx::IApp
{
  public:
	OpenGL_App() : gefx::IApp("Grefixs OpenGL"){};
	~OpenGL_App() override = default;
	OpenGL_App(OpenGL_App&&) = delete;
	OpenGL_App(const OpenGL_App&) = delete;
	OpenGL_App& operator=(OpenGL_App&&) = delete;
	OpenGL_App& operator=(const OpenGL_App&) = delete;

	void Awake() override;
	void Setup() override;
	void Shutdown() override;
	void Sleep() override;
	void Update(double deltaTime) override;

  private:
	static void OnGlfwErrorCallback(int error, const char* description)
	{
		fprintf(stderr, "Glfw Error %d: %s\n", error, description);
	}

	static void OnGlfwWindowResizeCallback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void DrawAppScreen(double deltaTime);
	GLFWwindow* _window{nullptr};

	GLuint _exampleVAO;
	GLuint _exampleShader;
};

#endif //!__APP__H__