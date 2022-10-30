
// Third Party Dependencies
#include <glad/glad.h>

// Internal Dependencies
#include <render/utilsCore.h>

// Using directives
using std::string;
template <typename T>
using vector = std::vector<T>;

namespace RenderUtils::OpenGL
{
	inline GLuint CompileShaderProgramTxt(string vertTxtData, string fragTxtData)
	{
		char logStr[1024];
		int resultCode = 0;
		bool valid = !vertTxtData.empty() && !fragTxtData.empty();

		// compile
		const char* c_str;
		uint32_t vid;
		if (valid)
		{
			vid = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vid, 1, &(c_str = vertTxtData.c_str()), NULL);
			glCompileShader(vid);

			glGetShaderiv(vid, GL_COMPILE_STATUS, &resultCode);
			if (resultCode == GL_FALSE)
			{
				glGetShaderInfoLog(vid, 1024, NULL, logStr);
				fmt::print("[OpenGL] Vertex Shader - Compile Error:\n{0}\n", logStr);
				glDeleteShader(vid);
				fflush(stdout);
				return 0;
			}
			else
			{
				fmt::print("[OpenGL] Vertex Shader - Compiled Successfully!\n");
			}
			fflush(stdout);
		}

		uint32_t fid;
		if (valid)
		{
			fid = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fid, 1, &(c_str = fragTxtData.c_str()), NULL);
			glCompileShader(fid);

			glGetShaderiv(fid, GL_COMPILE_STATUS, &resultCode);
			if (resultCode == GL_FALSE)
			{
				glGetShaderInfoLog(fid, 1024, NULL, logStr);
				fmt::print("[OpenGL] Fragment Shader - Compile Error:\n{0}\n", logStr);
				glDeleteShader(vid);
				glDeleteShader(fid);
				fflush(stdout);
				return 0;
			}
			else
			{
				fmt::print("[OpenGL] Fragment Shader - Compiled Successfully!\n");
			}
		}

		// link
		uint32_t pid = glCreateProgram();
		if (valid) glAttachShader(pid, vid);
		if (valid) glAttachShader(pid, fid);
		glLinkProgram(pid);

		// log
		glGetProgramInfoLog(pid, 1024, NULL, logStr);

		glGetProgramiv(pid, GL_LINK_STATUS, &resultCode);
		if (resultCode == GL_FALSE)
		{
			glGetProgramInfoLog(pid, 1024, NULL, logStr);
			fmt::print("[OpenGL] Shader Program - Link Error:\n{0}\n", logStr);
			glDeleteShader(vid);
			glDeleteShader(fid);
			glDeleteProgram(pid);
			fflush(stdout);
			return 0;
		}
		else
		{
			fmt::print("[OpenGL] Shader Program - Linked Successfully!\n");
		}

		// clean
		if (valid) glDetachShader(pid, vid);
		if (valid) glDetachShader(pid, fid);

		if (valid) glDeleteShader(vid);
		if (valid) glDeleteShader(fid);

		fflush(stdout);
		return pid;
	}

	inline GLuint CompileShaderProgramSpirV(const vector<unsigned int>& vertSpirVData,
											const vector<unsigned int>& fragSpirVData)
	{
		char logStr[1024];
		int resultCode = 0;
		bool valid = !vertSpirVData.empty() && !fragSpirVData.empty();

		// compile
		uint32_t vid = 0;
		if (valid)
		{
			vid = glCreateShader(GL_VERTEX_SHADER);
			const int vertDataBytesSize = static_cast<int>(vertSpirVData.size() * sizeof(unsigned int));
			glShaderBinary(1, &vid, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, vertSpirVData.data(), vertDataBytesSize);
			glSpecializeShader(vid, "main", 0, nullptr, nullptr);

			glGetShaderiv(vid, GL_COMPILE_STATUS, &resultCode);
			if (resultCode == GL_FALSE)
			{
				glGetShaderInfoLog(vid, 1024, NULL, logStr);
				fmt::print("[Spir-V] Vertex Shader - Compile Error:\n{0}\n", logStr);
				glDeleteShader(vid);
				fflush(stdout);
				return 0;
			}
			else
			{
				fmt::print("[Spir-V] Vertex Shader - Compiled Successfully!\n");
			}
		}

		uint32_t fid = 0;
		if (valid)
		{
			fid = glCreateShader(GL_FRAGMENT_SHADER);
			const int fragDataBytesSize = static_cast<int>(fragSpirVData.size() * sizeof(unsigned int));
			glShaderBinary(1, &fid, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, fragSpirVData.data(), fragDataBytesSize);
			glSpecializeShader(fid, "main", 0, nullptr, nullptr);

			glGetShaderiv(fid, GL_COMPILE_STATUS, &resultCode);
			if (resultCode == GL_FALSE)
			{
				glGetShaderInfoLog(fid, 1024, NULL, logStr);
				fmt::print("[Spir-V] Fragment Shader - Compile Error:\n{0}\n", logStr);
				glDeleteShader(vid);
				glDeleteShader(fid);
				fflush(stdout);
				return 0;
			}
			else
			{
				fmt::print("[Spir-V] Fragment Shader - Compiled Successfully!\n");
			}
		}

		// link
		uint32_t pid = glCreateProgram();
		if (valid) glAttachShader(pid, vid);
		if (valid) glAttachShader(pid, fid);
		glLinkProgram(pid);

		glGetProgramiv(pid, GL_LINK_STATUS, &resultCode);
		if (resultCode == GL_FALSE)
		{
			glGetProgramInfoLog(pid, 1024, NULL, logStr);
			fmt::print("[Spir-V] Shader Program - Link Error:\n{0}\n", logStr);
			glDeleteShader(vid);
			glDeleteShader(fid);
			glDeleteProgram(pid);
			fflush(stdout);
			return 0;
		}
		else
		{
			fmt::print("[Spir-V] Shader Program - Linked Successfully!\n");
		}

		// clean
		if (valid) glDetachShader(pid, vid);
		if (valid) glDetachShader(pid, fid);

		if (valid) glDeleteShader(vid);
		if (valid) glDeleteShader(fid);

		fflush(stdout);
		return pid;
	}

} // namespace RenderUtils::OpenGL