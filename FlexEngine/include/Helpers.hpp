#pragma once

#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <assimp/vector2.h>
#include <assimp/vector3.h>
#include <assimp/color4.h>

#include "imgui.h"

#include "Types.hpp"

namespace flex
{
	GLFWimage LoadGLFWimage(const std::string& filePath, bool alpha = false, bool flipVertically = false);
	void DestroyGLFWimage(GLFWimage& image);

	struct HDRImage
	{
		bool Load(const std::string& hdrFilePath, bool flipVertically);
		void Free();

		i32 width;
		i32 height;
		i32 channelCount;
		std::string filePath;
		real* pixels;
	};

	struct RollingAverage
	{
		RollingAverage(i32 valueCount);

		void AddValue(real newValue);
		void Reset();

		real currentAverage;
		std::vector<real> prevValues;

		i32 currentIndex = 0;
	};

	bool ReadFile(const std::string& filePath, std::vector<char>& vec);

	/*
	* Reads in a .wav file and fills in given values according to file contents
	* Returns true if reading and parsing succeeded
	*/
	bool ParseWAVFile(const std::string& filePath, i32* format, void** data, i32* size, i32* freq);

	/* Interpret 4 bytes starting at ptr as an unsigned 32-bit int */
	u32 Parse32u(char* ptr);
	/* Interpret 2 bytes starting at ptr as an unsigned 16-bit int */
	u16 Parse16u(char* ptr);


	// Removes all content before the final '/' or '\' 
	void StripLeadingDirectories(std::string& filePath);

	std::vector<std::string> Split(const std::string& str, char delim);

	/*
	 * Returns the index of the first character which isn't a number
	 * of letter (or -1 if none exist) starting from offset
	 */
	i32 NextNonAlphaNumeric(const std::string& str, i32 offset);

	real Lerp(real a, real b, real t);
	glm::vec2 Lerp(const glm::vec2& a, const glm::vec2& b, real t);
	glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, real t);

	/* Parses a single float, returns -1.0f if incorrectly formatted */
	real ParseFloat(const std::string& floatStr);

	/* Parses a comma separated list of 2 floats */
	glm::vec2 ParseVec2(const std::string& vecStr);

	/* Parses a comma separated list of 3 floats */
	glm::vec3 ParseVec3(const std::string& vecStr);

	/*
	* Parses a comma separated list of 3 or 4 floats
	* If defaultW is non-negative and no w component exists in the string defaultW will be used
	* If defaultW is negative then an error will be generated if the string doesn't
	* contain 4 components
	*/
	glm::vec4 ParseVec4(const std::string& vecStr, real defaultW = 1.0f);

	template<class T>
	inline typename std::vector<T>::const_iterator Contains(const std::vector<T>& vec, const T& t)
	{
		for (auto iter = vec.begin(); iter != vec.end(); ++iter)
		{
			if (*iter == t)
			{
				return iter;
			}
		}

		return vec.end();
	}

	template<class TReal>
	glm::vec2 ToVec2(const aiVector2t<TReal>& vec)
	{
		return glm::vec2(vec.x, vec.y);
	}

	template<class TReal>
	glm::vec3 ToVec3(const aiVector3t<TReal>& vec)
	{
		return glm::vec3(vec.x, vec.y, vec.z);
	}

	template<class TReal>
	glm::vec4 ToVec4(const aiColor4t<TReal>& color)
	{
		return glm::vec4(color.r, color.g, color.b, color.a);
	}

	std::string FloatToString(real f, i32 precision);

	// TODO: Remove: unused
	//void Vec2ToString(const glm::vec2& vec, std::ostream& stream);
	//void Vec3ToString(const glm::vec3& vec, std::ostream& stream);
	//void Vec4ToString(const glm::vec4& vec, std::ostream& stream);

	std::string Vec2ToString(const glm::vec2& vec);
	std::string Vec3ToString(const glm::vec3& vec);
	std::string Vec4ToString(const glm::vec4& vec);

	void CopyColorToClipboard(const glm::vec4& col);
	void CopyColorToClipboard(const glm::vec3& col);

	glm::vec3 PasteColor3FromClipboard();
	glm::vec4 PasteColor4FromClipboard();

	CullFace StringToCullFace(const std::string& str);
	std::string CullFaceToString(CullFace cullFace);

	void ToLower(std::string& str);
	void ToUpper(std::string& str);

	bool StartsWith(const std::string& str, const std::string& start);

	std::string SerializableTypeToString(SerializableType type);
	SerializableType StringToSerializableType(const std::string& serializableTypeStr);

} // namespace flex
