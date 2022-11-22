// StdLib Dependencies
#include <fstream>
#include <iostream>

// Third Party Dependencies
#include <fmt/core.h>
#include <stdint.h>
#include <volk.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/SPIRV/disassemble.h>

// Using directives
using std::string;
template <typename T>
using vector = std::vector<T>;
using std::istreambuf_iterator;

namespace RenderUtils
{
	inline void InitGlslang() { glslang::InitializeProcess(); }

	inline void FinalizeGlslang() { glslang::FinalizeProcess(); }

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

	inline const char* VkShaderTypeToStr(const VkShaderStageFlagBits shaderType)
	{
		switch (shaderType)
		{
		case VK_SHADER_STAGE_VERTEX_BIT:
			return "Vertex";
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
			return "TessControl";
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
			return "TessEvaluation";
		case VK_SHADER_STAGE_GEOMETRY_BIT:
			return "Geometry";
		case VK_SHADER_STAGE_FRAGMENT_BIT:
			return "Fragment";
		case VK_SHADER_STAGE_COMPUTE_BIT:
			return "Compute";
		case VK_SHADER_STAGE_RAYGEN_BIT_NV:
			return "RayGenNV";
		case VK_SHADER_STAGE_ANY_HIT_BIT_NV:
			return "AnyHitNV";
		case VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV:
			return "ClosestHitNV";
		case VK_SHADER_STAGE_MISS_BIT_NV:
			return "MissNV";
		case VK_SHADER_STAGE_INTERSECTION_BIT_NV:
			return "IntersectNV";
		case VK_SHADER_STAGE_CALLABLE_BIT_NV:
			return "CallableNV";
		case VK_SHADER_STAGE_TASK_BIT_NV:
			return "TaskNV";
		case VK_SHADER_STAGE_MESH_BIT_NV:
			return "MeshNV";
		default:
			assert(false && "Unknown shader stage");
			return "Vertex";
		}
	}

	inline EShLanguage FindLanguage(const VkShaderStageFlagBits shaderType)
	{
		switch (shaderType)
		{
		case VK_SHADER_STAGE_VERTEX_BIT:
			return EShLangVertex;
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
			return EShLangTessControl;
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
			return EShLangTessEvaluation;
		case VK_SHADER_STAGE_GEOMETRY_BIT:
			return EShLangGeometry;
		case VK_SHADER_STAGE_FRAGMENT_BIT:
			return EShLangFragment;
		case VK_SHADER_STAGE_COMPUTE_BIT:
			return EShLangCompute;
		case VK_SHADER_STAGE_RAYGEN_BIT_NV:
			return EShLangRayGenNV;
		case VK_SHADER_STAGE_ANY_HIT_BIT_NV:
			return EShLangAnyHitNV;
		case VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV:
			return EShLangClosestHitNV;
		case VK_SHADER_STAGE_MISS_BIT_NV:
			return EShLangMissNV;
		case VK_SHADER_STAGE_INTERSECTION_BIT_NV:
			return EShLangIntersectNV;
		case VK_SHADER_STAGE_CALLABLE_BIT_NV:
			return EShLangCallableNV;
		case VK_SHADER_STAGE_TASK_BIT_NV:
			return EShLangTaskNV;
		case VK_SHADER_STAGE_MESH_BIT_NV:
			return EShLangMeshNV;
		default:
			assert(false && "Unknown shader stage");
			return EShLangVertex;
		}
	}

	enum class ShaderCompileTarget : uint8_t
	{
		OpenGL = 0,
		Vulkan = 1
	};

	inline bool GLSLtoSPV(const VkShaderStageFlagBits shaderType, const char* shaderStr,
						  std::vector<unsigned int>& spirv, ShaderCompileTarget compileTarget)
	{
		EShLanguage stage = FindLanguage(shaderType);
		glslang::TShader shader(stage);
		glslang::TProgram program;
		TBuiltInResource resources = {};
		InitResources(resources);

		// Enable SPIR-V and Vulkan rules when parsing GLSL
		EShMessages messages = EShMsgSpvRules;
		if (compileTarget == ShaderCompileTarget::Vulkan) messages = (EShMessages)(messages | EShMsgVulkanRules);

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

} // namespace RenderUtils