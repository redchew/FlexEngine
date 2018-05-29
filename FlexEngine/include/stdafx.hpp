#pragma once

#define NOMINMAX

#define COMPILE_OPEN_GL 1
#define COMPILE_VULKAN 0

#pragma warning(disable : 4201) // nonstandard extension used: nanmeless struct/union
//#pragma warning(disable : 4820) // bytes' bytes padding added after construct 'member_name'
//#pragma warning(disable : 4868) // compiler may not enforce left-to-right evaluation order in braced initializer list
//#pragma warning(disable : 4710) // function not inlined

#include "Types.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#if COMPILE_VULKAN
#pragma warning(push, 0) // Don't generate warnings for 3rd party code    
	#include <glad/glad.h>
	#include <vulkan/vulkan.hpp>
	#include <GLFW/glfw3.h>
	#include <GLFW/glfw3native.h>
#pragma warning(pop)

	#include "Graphics/Vulkan/VulkanRenderer.hpp"
	#include "Window/Vulkan/VulkanWindowWrapper.hpp"
#endif // COMPILE_VULKAN

#if COMPILE_OPEN_GL
#pragma warning(push, 0) // Don't generate warnings for 3rd party code    
#define GLFW_EXPOSE_NATIVE_WIN32
	#include <glad/glad.h>
	#include <GLFW/glfw3.h>
	#include <GLFW/glfw3native.h>
#pragma warning(pop)

#if _DEBUG
#ifndef CheckGLErrorMessages
void _CheckGLErrorMessages(const char *file, flex::i32 line);
#define CheckGLErrorMessages() _CheckGLErrorMessages(__FILE__,__LINE__)
#endif // CheckGLErrorMessages
#else
#define CheckGLErrorMessages() 
#endif // COMPILE_OPEN_GL

	#include "Graphics/GL/GLRenderer.hpp"
	#include "Window/GL/GLWindowWrapper.hpp"

#endif // COMPILE_OPEN_GL

#include "Physics/PhysicsTypeConversions.hpp"

template<class T>
inline void SafeDelete(T &pObjectToDelete)
{
	if (pObjectToDelete != nullptr)
	{
		delete(pObjectToDelete);
		pObjectToDelete = nullptr;
	}
}

#ifndef btAssert
#define btAssert(e) assert(e)
#endif

#pragma warning(push, 0) // Don't generate warnings for 3rd party code    
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

// For matrix_decompose
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#pragma warning(pop)

#define PI (glm::pi<real>())
#define TWO_PI (glm::two_pi<real>())
#define PI_DIV_TWO (glm::half_pi<real>())
#define PI_DIV_FOUR (glm::quarter_pi<real>())
#define THREE_OVER_TWO_PI (glm::three_over_two_pi<real>())

namespace flex
{
	static const std::string RESOURCE_LOCATION = "../../../FlexEngine/resources/";
}
