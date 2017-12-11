#include "GL\glew.h"
#include "./Bitmap.cpp"
void getTexFormat(GLint bitmapFormat, GLenum *Format, GLenum *Type)
{
	GLenum format = 0;
	GLenum type = 0;

	switch (bitmapFormat)
	{
	case RGBA_8888:
		format = GL_RGBA;
		type = GL_UNSIGNED_BYTE;
		break;

	case BGRA_8888:
		format = GL_BGRA_EXT;
		type = GL_UNSIGNED_BYTE;
		break;

	case RGBX_8888:
		format = GL_RGBA;
		type = GL_UNSIGNED_BYTE;
		break;

	case RGB_888:
		format = GL_RGB;
		type = GL_UNSIGNED_BYTE;
		break;

	case RGB_565:
		format = GL_RGB;
		type = GL_UNSIGNED_SHORT_5_6_5;
		break;

	case RGBA_4444:
		format = GL_RGBA;
		type = GL_UNSIGNED_SHORT_4_4_4_4;
		break;

	case ARGB_1555:
		format = GL_RGBA;
		type = GL_UNSIGNED_SHORT_5_5_5_1;
		break;

	case L8:
		format = GL_RGBA;
		type = GL_UNSIGNED_SHORT_5_5_5_1;
		break;

	default:
		break;
	}

	if (Format)
		*Format = format;

	if (Type)
		*Type = type;
}

/**
* 加载并编译着色器代码
*/
GLint compilerShader(const GLchar* fName, GLint shaderNum)
{
	FILE* fPtr = NULL;
	//fopen_s()
	fPtr = fopen(fName, "rb");
	if (fPtr == NULL)
	{
		//cerr << "Can not open file " << fName << endl;
		printf("Can not open file '%s'" ,fName);
		return 0;
	}

	int length;
	fseek(fPtr, 0, SEEK_END);
	length = ftell(fPtr);
	fseek(fPtr, 0, SEEK_SET);

	length++;
	char* shaderSource = new char[sizeof(char) * length];
	if (shaderSource == NULL)
	{
		//cerr << "Out of memory" << endl;
		printf("Out of memory");
		return 0;
	}

	memset(shaderSource, 0, length);
	fread(shaderSource, length - 1, 1, fPtr);

	length--;
	glShaderSource(shaderNum, 1, (const char**)&shaderSource, &length);
	glCompileShader(shaderNum);

	free(shaderSource);
	fclose(fPtr);

	GLint compiled = 0;
	glGetShaderiv(shaderNum, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		GLint errorBufferSize, errorLength;
		glGetShaderiv(shaderNum, GL_INFO_LOG_LENGTH, &errorBufferSize);

		char* infoLog = new char[errorBufferSize * sizeof(char) + 1];
		if (infoLog)
		{
			glGetShaderInfoLog(shaderNum, errorBufferSize, &errorLength, infoLog);
			infoLog[errorBufferSize + 1] = '\0';
			//cerr << infoLog << endl;
			printf("infoLog '%s'", infoLog);
			free(infoLog);
		}

		//cerr << "Error compiling shader " << fName << endl;
		printf("Error compiling shader '%s'" ,fName);
		return 0;
	}

	return 1;
}