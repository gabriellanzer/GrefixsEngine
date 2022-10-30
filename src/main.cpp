
// Hide Console Window
// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

// #define OPENGL_APP
#ifdef OPENGL_APP
#include <app/appOpenGL.h>
#endif

#define VULKAN_APP
#ifdef VULKAN_APP
#include <app/appVulkan.h>
#endif

#include <fstream>

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "Portuguese");

#ifdef OPENGL_APP
	OpenGL_App app;
#endif
#ifdef VULKAN_APP
	Vulkan_App app;
#endif
	app.Run();

	return 0;
}