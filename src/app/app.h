#ifndef __APP__H__
#define __APP__H__

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>

#include <core/iapp.h>

class GrefixsEndine : public gefx::IApp
{
  public:
	GrefixsEndine()
		: gefx::IApp("Grefixs") {};
	~GrefixsEndine() override = default;
	GrefixsEndine(GrefixsEndine&&) = delete;
	GrefixsEndine(const GrefixsEndine&) = delete;
	GrefixsEndine& operator=(GrefixsEndine&&) = delete;
	GrefixsEndine& operator=(const GrefixsEndine&) = delete;

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