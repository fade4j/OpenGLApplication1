/*Created by SpaceCowboy 2017年12月11日22:15*/
#include "GL\glew.h"
#include "GL\glut.h"
#include "InitGlut.cpp"
#include "./Bitmap.cpp"
#define VERTEX_COUNT 4
int program = 0;
GLfloat transformMatrix[16]=
{
	1.0f , 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

float vertices[VERTEX_COUNT][4] =
{
	{-0.5f, 0.5f,	0.0f, 1.0f},
	{0.5f, 0.5f,		1.0f, 1.0f},
	{-0.5f, -0.5f,	0.0f, 0.0f},
	{0.5f, -0.5f,   1.0f, 0.0f}
};

GLvoid* bitmap = NULL;
GLuint texture = 0;
void loadShaders(char* vShaderFileName, char* fShaderFileName);
GLint compileShader(char* fileName, GLint shader);
void getTextureFormat(GLint bitmapFormat, GLenum* format, GLenum* type);
void renderInit();
void initUniform();
void initVertexAttrib();
void createTexture();
void render();
int main(int argc, char** argv)
{
	initGlut(argc, argv);
	loadShaders("vs_es20t1.vert", "ps_es20t1.frag");
	if (program != 0)
	{
		renderInit();
		startLoop(render);
	}
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.8f, 0.4f, 0.8f, 1.0f);
	printf("rendering!!!!");
	//glBindTexture(GL_TEXTURE_2D, texture);
	//glBindTexture(GL_TEXTURE_2D, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, VERTEX_COUNT);
	glFinish();
}

void renderInit()
{
	initUniform();
	initVertexAttrib();
	createTexture();
}

void createTexture()
{
	GLuint width = 0;
	GLuint height = 0;
	int bitmapFormat = 0;
	bitmap = ReadBitmap("zero2_R5G6B5.bmp", &width, &height, &bitmapFormat);
	if (bitmap == NULL)
	{
		printf("Texture bitmap is NULL!!!");
		return;
	}

	GLenum format = 0;
	GLenum type = 0;
	getTextureFormat(bitmapFormat, &format, &type);
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, bitmap);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	int sampler2D = glGetUniformLocation(program, "sampler");
	glUniform1i(sampler2D, 0);

	free(bitmap);
}

void initUniform()
{
	GLint uniformLocation = glGetUniformLocation(program, "my_TransformMatrix");
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, transformMatrix);
}

void initVertexAttrib()
{
	GLint verticesLocation = glGetAttribLocation(program, "my_Vertex");
	GLint textureCoordLocation = glGetAttribLocation(program, "my_Texcoord");

	glEnableVertexAttribArray(verticesLocation);
	glEnableVertexAttribArray(textureCoordLocation);

	glVertexAttribPointer(verticesLocation, 2, GL_FLOAT, GL_FALSE, 16, &vertices[0][0]);
	glVertexAttribPointer(textureCoordLocation, 2, GL_FLOAT, GL_FALSE, 16, &vertices[0][2]);
}

void loadShaders(char* vShaderFileName, char* fShaderFileName)
{
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	
	if (compileShader(vShaderFileName, vertexShader) == 0)
	{
		printf("compile '%s' return 0", vShaderFileName);
		return;
	}

	if (compileShader(fShaderFileName, fragmentShader) == 0)
	{
		printf("compile '%s' return 0", fShaderFileName);
		return;
	}

	program = glCreateProgram();

	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program);

	int linked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		printf("program is not linked!!!");
		return;
	}

	glUseProgram(program);
}

/**
* 加载并编译着色器代码
*/
GLint compileShader(char* fName, GLint shaderNum)
{
	FILE* fPtr = NULL;
	//fopen_s()
	fPtr = fopen(fName, "rb");
	if (fPtr == NULL)
	{
		//cerr << "Can not open file " << fName << endl;
		printf("Can not open file '%s'",fName);
		//printf("Can not open file");
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
		cerr << "Out of memory" << endl;
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
			cerr << infoLog << endl;
			free(infoLog);
		}

		//cerr << "Error compiling shader " << fName << endl;
		printf("Error compiling shader '%s'", fName);
		return 0;
	}

	return 1;
}

void getTextureFormat(GLint bitmapFormat, GLenum *Format, GLenum *Type)
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