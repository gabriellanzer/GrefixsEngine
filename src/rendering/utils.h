// StdLib Dependencies
#include <fstream>
#include <iostream>

// Third Party Dependencies
#include <fmt/core.h>
#include <glad/glad.h>
#include <vulkan/vulkan.hpp>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/SPIRV/disassemble.h>

// Using directives
using std::string;
template <typename T>
using vector = std::vector<T>;
using std::istreambuf_iterator;

// TODO: Move this to proper Graphics API abstraction
namespace ShaderUtils
{
	inline void Init() { glslang::InitializeProcess(); }

	inline void Finalize() { glslang::FinalizeProcess(); }

	inline void InitResources(TBuiltInResource& rsc)
	{
		rsc.maxLights = 32;
		rsc.maxClipPlanes = 6;
		rsc.maxTextureUnits = 32;
		rsc.maxTextureCoords = 32;
		rsc.maxVertexAttribs = 64;
		rsc.maxVertexUniformComponents = 4096;
		rsc.maxVaryingFloats = 64;
		rsc.maxVertexTextureImageUnits = 32;
		rsc.maxCombinedTextureImageUnits = 80;
		rsc.maxTextureImageUnits = 32;
		rsc.maxFragmentUniformComponents = 4096;
		rsc.maxDrawBuffers = 32;
		rsc.maxVertexUniformVectors = 128;
		rsc.maxVaryingVectors = 8;
		rsc.maxFragmentUniformVectors = 16;
		rsc.maxVertexOutputVectors = 16;
		rsc.maxFragmentInputVectors = 15;
		rsc.minProgramTexelOffset = -8;
		rsc.maxProgramTexelOffset = 7;
		rsc.maxClipDistances = 8;
		rsc.maxComputeWorkGroupCountX = 65535;
		rsc.maxComputeWorkGroupCountY = 65535;
		rsc.maxComputeWorkGroupCountZ = 65535;
		rsc.maxComputeWorkGroupSizeX = 1024;
		rsc.maxComputeWorkGroupSizeY = 1024;
		rsc.maxComputeWorkGroupSizeZ = 64;
		rsc.maxComputeUniformComponents = 1024;
		rsc.maxComputeTextureImageUnits = 16;
		rsc.maxComputeImageUniforms = 8;
		rsc.maxComputeAtomicCounters = 8;
		rsc.maxComputeAtomicCounterBuffers = 1;
		rsc.maxVaryingComponents = 60;
		rsc.maxVertexOutputComponents = 64;
		rsc.maxGeometryInputComponents = 64;
		rsc.maxGeometryOutputComponents = 128;
		rsc.maxFragmentInputComponents = 128;
		rsc.maxImageUnits = 8;
		rsc.maxCombinedImageUnitsAndFragmentOutputs = 8;
		rsc.maxCombinedShaderOutputResources = 8;
		rsc.maxImageSamples = 0;
		rsc.maxVertexImageUniforms = 0;
		rsc.maxTessControlImageUniforms = 0;
		rsc.maxTessEvaluationImageUniforms = 0;
		rsc.maxGeometryImageUniforms = 0;
		rsc.maxFragmentImageUniforms = 8;
		rsc.maxCombinedImageUniforms = 8;
		rsc.maxGeometryTextureImageUnits = 16;
		rsc.maxGeometryOutputVertices = 256;
		rsc.maxGeometryTotalOutputComponents = 1024;
		rsc.maxGeometryUniformComponents = 1024;
		rsc.maxGeometryVaryingComponents = 64;
		rsc.maxTessControlInputComponents = 128;
		rsc.maxTessControlOutputComponents = 128;
		rsc.maxTessControlTextureImageUnits = 16;
		rsc.maxTessControlUniformComponents = 1024;
		rsc.maxTessControlTotalOutputComponents = 4096;
		rsc.maxTessEvaluationInputComponents = 128;
		rsc.maxTessEvaluationOutputComponents = 128;
		rsc.maxTessEvaluationTextureImageUnits = 16;
		rsc.maxTessEvaluationUniformComponents = 1024;
		rsc.maxTessPatchComponents = 120;
		rsc.maxPatchVertices = 32;
		rsc.maxTessGenLevel = 64;
		rsc.maxViewports = 16;
		rsc.maxVertexAtomicCounters = 0;
		rsc.maxTessControlAtomicCounters = 0;
		rsc.maxTessEvaluationAtomicCounters = 0;
		rsc.maxGeometryAtomicCounters = 0;
		rsc.maxFragmentAtomicCounters = 8;
		rsc.maxCombinedAtomicCounters = 8;
		rsc.maxAtomicCounterBindings = 1;
		rsc.maxVertexAtomicCounterBuffers = 0;
		rsc.maxTessControlAtomicCounterBuffers = 0;
		rsc.maxTessEvaluationAtomicCounterBuffers = 0;
		rsc.maxGeometryAtomicCounterBuffers = 0;
		rsc.maxFragmentAtomicCounterBuffers = 1;
		rsc.maxCombinedAtomicCounterBuffers = 1;
		rsc.maxAtomicCounterBufferSize = 16384;
		rsc.maxTransformFeedbackBuffers = 4;
		rsc.maxTransformFeedbackInterleavedComponents = 64;
		rsc.maxCullDistances = 8;
		rsc.maxCombinedClipAndCullDistances = 8;
		rsc.maxSamples = 4;
		rsc.maxMeshOutputVerticesNV = 256;
		rsc.maxMeshOutputPrimitivesNV = 512;
		rsc.maxMeshWorkGroupSizeX_NV = 32;
		rsc.maxMeshWorkGroupSizeY_NV = 1;
		rsc.maxMeshWorkGroupSizeZ_NV = 1;
		rsc.maxTaskWorkGroupSizeX_NV = 32;
		rsc.maxTaskWorkGroupSizeY_NV = 1;
		rsc.maxTaskWorkGroupSizeZ_NV = 1;
		rsc.maxMeshViewCountNV = 4;
		rsc.limits.nonInductiveForLoops = 1;
		rsc.limits.whileLoops = 1;
		rsc.limits.doWhileLoops = 1;
		rsc.limits.generalUniformIndexing = 1;
		rsc.limits.generalAttributeMatrixVectorIndexing = 1;
		rsc.limits.generalVaryingIndexing = 1;
		rsc.limits.generalSamplerIndexing = 1;
		rsc.limits.generalVariableIndexing = 1;
		rsc.limits.generalConstantMatrixVectorIndexing = 1;
	}

	inline const char* VkShaderTypeToStr(const vk::ShaderStageFlagBits shaderType)
	{
		switch (shaderType)
		{
		case vk::ShaderStageFlagBits::eVertex:
			return "Vertex";
		case vk::ShaderStageFlagBits::eTessellationControl:
			return "TessControl";
		case vk::ShaderStageFlagBits::eTessellationEvaluation:
			return "TessEvaluation";
		case vk::ShaderStageFlagBits::eGeometry:
			return "Geometry";
		case vk::ShaderStageFlagBits::eFragment:
			return "Fragment";
		case vk::ShaderStageFlagBits::eCompute:
			return "Compute";
		case vk::ShaderStageFlagBits::eRaygenNV:
			return "RayGenNV";
		case vk::ShaderStageFlagBits::eAnyHitNV:
			return "AnyHitNV";
		case vk::ShaderStageFlagBits::eClosestHitNV:
			return "ClosestHitNV";
		case vk::ShaderStageFlagBits::eMissNV:
			return "MissNV";
		case vk::ShaderStageFlagBits::eIntersectionNV:
			return "IntersectNV";
		case vk::ShaderStageFlagBits::eCallableNV:
			return "CallableNV";
		case vk::ShaderStageFlagBits::eTaskNV:
			return "TaskNV";
		case vk::ShaderStageFlagBits::eMeshNV:
			return "MeshNV";
		default:
			assert(false && "Unknown shader stage");
			return "Vertex";
		}
	}

	inline EShLanguage FindLanguage(const vk::ShaderStageFlagBits shaderType)
	{
		switch (shaderType)
		{
		case vk::ShaderStageFlagBits::eVertex:
			return EShLangVertex;
		case vk::ShaderStageFlagBits::eTessellationControl:
			return EShLangTessControl;
		case vk::ShaderStageFlagBits::eTessellationEvaluation:
			return EShLangTessEvaluation;
		case vk::ShaderStageFlagBits::eGeometry:
			return EShLangGeometry;
		case vk::ShaderStageFlagBits::eFragment:
			return EShLangFragment;
		case vk::ShaderStageFlagBits::eCompute:
			return EShLangCompute;
		case vk::ShaderStageFlagBits::eRaygenNV:
			return EShLangRayGenNV;
		case vk::ShaderStageFlagBits::eAnyHitNV:
			return EShLangAnyHitNV;
		case vk::ShaderStageFlagBits::eClosestHitNV:
			return EShLangClosestHitNV;
		case vk::ShaderStageFlagBits::eMissNV:
			return EShLangMissNV;
		case vk::ShaderStageFlagBits::eIntersectionNV:
			return EShLangIntersectNV;
		case vk::ShaderStageFlagBits::eCallableNV:
			return EShLangCallableNV;
		case vk::ShaderStageFlagBits::eTaskNV:
			return EShLangTaskNV;
		case vk::ShaderStageFlagBits::eMeshNV:
			return EShLangMeshNV;
		default:
			assert(false && "Unknown shader stage");
			return EShLangVertex;
		}
	}

	inline bool GLSLtoSPV(const vk::ShaderStageFlagBits shaderType, const char* shaderStr,
						  std::vector<unsigned int>& spirv)
	{
		EShLanguage stage = FindLanguage(shaderType);
		glslang::TShader shader(stage);
		glslang::TProgram program;
		TBuiltInResource resources = {};
		InitResources(resources);

		// Enable SPIR-V and Vulkan rules when parsing GLSL
		// TODO: re-add flag for vulkan requirements when compiling GLSL for Vulkan Backend
		EShMessages messages = (EShMessages)(EShMsgSpvRules /*| EShMsgVulkanRules*/);

		const char* c_str;
		shader.setStrings(&(c_str = shaderStr), 1);

		if (!shader.parse(&resources, 100, false, messages))
		{
			fmt::print(shader.getInfoLog());
			fmt::print(shader.getInfoDebugLog());
			fflush(stdout);
			return false; // something didn't work
		}

		program.addShader(&shader);

		//
		// Program-level processing...
		//

		if (!program.link(messages))
		{
			fmt::print(shader.getInfoLog());
			fmt::print(shader.getInfoDebugLog());
			fflush(stdout);
			return false;
		}

		glslang::SpvOptions options = {};
		options.validate = true;
		glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &options);
		fmt::print("GLSL to SPIR-V compilation succeded! Stage: {0}\n", VkShaderTypeToStr(shaderType));

		// Dump Disassemble:
		// spv::Disassemble(std::cout, spirv);

		fflush(stdout);
		return true;
	}

	inline bool TryLoadShaderFile(string shaderPath, string& outShaderData)
	{
		std::ifstream in;
		in.open(shaderPath.c_str(), std::ios::in);
		if (!in)
		{
			return false;
		}
		outShaderData.assign((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
		in.close();
		return true;
	}

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
} // namespace ShaderUtils