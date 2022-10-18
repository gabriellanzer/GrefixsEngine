# Grefixs Engine
This is a learning project for an Graphics API abstraction layer. The pun-intended name will probably reflect the go-horse code style (at least for the pieces not fun to write). The goal of it is to have both OpenGL and Vulkan back-end implementations and that they are fast as a horse (or 100.000 horses at the same time). The project goals are clear:
- Clean API
- High Performance
- Backend Abstraction
- Multiplatform Support

## Setting-up for Development
This project uses CMake as it's buil system generator and Conan as a dependency package manager. Some dependencies are header-only and thus already included in the repo. Both CMake and Conan are required for the code to build:
- CMake: https://cmake.org/
- Conan: https://conan.io/

After installing both, you can open the CMake GUI and configure the CMake project. The configure step will invoke Conan, so make sure it is mapped in the PATH and is accessible from command line. Conan will then download any existing precompiled binaries or build them from source for the specified compiler-target profile.

This setup works great with Ninja as a Generator for either VSCode or CLion. It will also work if you generate a Visual Studio Solution. Compiling with Clang in Windows is a little trickier, you must force CMake to use Clang compiler through environmental variables (at least for the first configure run). If you don't, the program's code will try to compile with Clang but all dependencies downloaded with Conan will use MSVC, thus break the CMake configure step with errors saying the compilers missmatch.

## Dependencies
The project depends on the following libraries so far:
- [GLFW](https://www.glfw.org/)
- [Glad](https://github.com/Dav1dde/glad)
- [FMT](https://github.com/fmtlib/fmt)
- [GLM](https://github.com/g-truc/glm)
- [PerlinNoise](https://github.com/Reputeless/PerlinNoise)
