#ifndef IOSTREAM
#include <iostream>
#endif // !IOSTREAM

#ifndef GLEW
#include "GL\glew.h"
#endif // !GLEW

#ifndef GLUT
#include "GL\glut.h"
#endif // !GLUT
#include "InitGlut.cpp"
#include "./Bitmap.cpp"
using namespace std;
#define VERTEX_COUNT 4
enum VAO_IDs
{
	Triangles,// 0
	NumVAOs// 1
};

enum Buffer_IDs
{
	ArrayBuffer,// 0
	NumBuffers// 1
};

enum Attrib_IDs
{
	vPosition = 0
};

GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];

const GLint NumVertices = 6;

void init(void);
void display(void);
void display1();
void display2();
void display3();
void display4();
void display5();
void display6();
void display7();
void initVertex();
/**
*	纹理贴图
*/
int programHandle = 0;

void loadShaders(const char* vShaderFName, const char* pShaderFName);
GLint compilerShader(const GLchar* fName, GLint shaderNum);
void renderInit();
void initUniform();
void initVertexAttrib();
void createTexture();
void render();
void unInitVertexAttrib();
void destroyShaders();

int main(int argc, char ** argv)
{
	//initVertex();
	initGlut(argc, argv);
	//initVertex();
	loadShaders("vs_es20t1.vert", "ps_es20t1.frag");// 加载着色器代码，编译设置到创建的顶点着色器上，创建Program，绑定着色器到Program上，使用Program
	//startLoop(display7);
	
	if (programHandle != 0)
	{
		renderInit();
		cerr << "start loop"<< endl;
		startLoop(render);
	}
	cerr << "destroyShaders" << endl;
	//destroyShaders();
	//system("pause");
	return 0;
}

/**
*	纹理贴图
*/
int vertexShaderNum;// 顶点着色器地址
int pixelShaderNum;// 片源着色器地址

// Wrraper to load vertex and pixel shader
void loadShaders(const char* vShaderFName, const char* pShaderFName)
{
	vertexShaderNum = glCreateShader(GL_VERTEX_SHADER);// 创建顶点着色器
	pixelShaderNum = glCreateShader(GL_FRAGMENT_SHADER);// 创建片源着色器
	cout << "pixelShaderNum : " << pixelShaderNum << endl;
	if (compilerShader(vShaderFName, vertexShaderNum) == 0)// 顶点着色器编译失败
	{
		return;
	}

	if (compilerShader(pShaderFName, pixelShaderNum) == 0)// 片源着色器编译失败
	{
		return;
	}

	programHandle = glCreateProgram();// 创建Program，用于绑定着色器
	//cout <<"programHandle : " << programHandle << endl;
	glAttachShader(programHandle, vertexShaderNum);// 绑定顶点着色器
	glAttachShader(programHandle, pixelShaderNum);// 绑定片源着色器

	glLinkProgram(programHandle);// 链接Program
	// check if link succeeded
	GLint linked = false;
	glGetProgramiv(programHandle, GL_LINK_STATUS, &linked);// 获取Program链接的状态
	if (!linked)
	{
		GLint errorBufferSize, errorLength;
		glGetShaderiv(programHandle, GL_INFO_LOG_LENGTH, &errorBufferSize);
	
		char* infoLog = new char[errorBufferSize * sizeof(char) + 1];

		if (!infoLog)
		{
			glGetProgramInfoLog(programHandle, errorBufferSize, &errorLength, infoLog);
			infoLog[errorBufferSize + 1] = '\0';
			cerr << ("%s", infoLog) << endl;

			free(infoLog);
		}
	
		cout << "Error linking program!!!!"<< endl;
		return;
	}

	glUseProgram(programHandle);// 使用绑定了着色器的Program
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
		cerr << "Can not open file "<<fName<< endl;
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
		cerr <<"Out of memory" << endl;
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

		cerr << "Error compiling shader "<< fName << endl;
		return 0;
	}

	return 1;
}

void renderInit()
{
	initUniform();
	initVertexAttrib();
	createTexture();
}
GLint locationTransformMatrix;
GLfloat transformMatrix[16] =
{
	1.0f, 0.0f,0.0f,0.0f,
	0.0f,1.0f,0.0f,0.0f,
	0.0f,0.0f,1.0f,0.0f,
	0.0f,0.0f,0.0f,1.0f
};
void initUniform()
{
	locationTransformMatrix = glGetUniformLocation(programHandle, "my_TranformMatrix");// 从Program取出变换矩阵
	glUniformMatrix4fv(locationTransformMatrix, 1, GL_FALSE, transformMatrix);
}
GLint locationVertices;
GLint locationTexcoord;
GLfloat verteices[VERTEX_COUNT][4] =
{
	{-0.8f, 0.8f,		0.0f, 1.0f},
	{0.8f, 0.8f,			1.0f, 1.0f},
	{-0.8f, -0.8f,		0.0f, 0.0f},
	{0.8f, -0.8f,		1.0f, 0.0f}
};
void initVertexAttrib()
{
	
	// Grab location of shader attributes
	locationVertices = glGetAttribLocation(programHandle, "my_Vertex");
	locationTexcoord = glGetAttribLocation(programHandle, "my_Texcoord");

	// enable vertex arrays to push the data
	glEnableVertexAttribArray(locationVertices);
	glEnableVertexAttribArray(locationTexcoord);

	// set data into the arrays
	glVertexAttribPointer(locationVertices, 2, GL_FLOAT, GL_FALSE, 16, &verteices[0][0]);
	glVertexAttribPointer(locationTexcoord, 2, GL_FLOAT, GL_FALSE, 16, &verteices[0][2]);
}

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

GLvoid * pTxtData = NULL;
GLuint  texObj = 0;
GLint locSampler2D = -1;
void createTexture()
{
	GLint bitmapFormat = 0;
	GLuint width = 0;
	GLuint height = 0;
	GLenum format = 0;
	GLenum type = 0;

	pTxtData = ReadBitmap("zero2.bmp", &width, &height, &bitmapFormat);
	//cout << "width : " << width << " , height：" << height << endl;
	cerr << pTxtData<<endl;
	if (pTxtData == NULL)
	{
		return;
	}
	
	getTexFormat(bitmapFormat, &format, &type);

	glGenTextures(1, &texObj);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texObj);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, pTxtData);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// get texture sampler
		locSampler2D = glGetUniformLocation(programHandle, "smapler");
		glUniform1i(locSampler2D, 0);

		free(pTxtData);
}


void render()
{
	static GLfloat angle = 0.0f;
	static GLint count = 0;
	glClear(GL_COLOR_BUFFER_BIT);
	// claear background
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	
	/*count++;
	if (count == 100)
	{
		// setup the rotation matrix rotating by angle around y axis
		transformMatrix[0] = transformMatrix[10] = (GLfloat)cos(angle);
		transformMatrix[2] = (GLfloat)sin(angle);
		transformMatrix[8] = -transformMatrix[2];

		angle += 0.1f;
		glUniformMatrix4fv(locationTransformMatrix, 1, GL_FALSE, transformMatrix);
		count = 0;
	}*/

	glBindTexture(GL_TEXTURE_2D, texObj);
	cerr << "start to draw arrays" << endl;
	glDrawArrays(GL_TRIANGLE_STRIP, 0, VERTEX_COUNT);
	glFinish();
	//unInitVertexAttrib();
}

void unInitVertexAttrib()
{
	// cleanup
	glDisableVertexAttribArray(locationVertices);
	glDisableVertexAttribArray(locationTexcoord);
}

// Cleanup the shaders.
void destroyShaders()
{
	glDeleteShader(vertexShaderNum);
	glDeleteShader(pixelShaderNum);
	glDeleteProgram(programHandle);
	glUseProgram(0);
}



















const GLuint vaoNum = 1;
const GLuint bufferNum = 1;
GLuint mVao[vaoNum];
GLuint mBuffer[bufferNum];

void initVertex() {
	GLfloat vertexs[6][2] = {
		{-0.3f,-0.3f},
		{-0.2f, -0.2f},
		{0.1f,0.1f},
		{0.5f,0.5f},
		{0.3f,0.3f},
		{0.9f,0.9f}
	};
	glGenVertexArrays(vaoNum, mVao);
	glBindVertexArray(mVao[0]);
	glGenBuffers(bufferNum, mBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mBuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexs), vertexs, GL_STATIC_DRAW);
}

void display7() {
	cout << "initGlut -> diaplay7"<< endl;
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.8f, 0.8f, 0.8f, 1);
	glColor3f(1, 0, 0);
	glPointSize(10);
	/*glBegin(GL_POINTS);
	glVertex2f(0.4, 0.4);
	glEnd();
	glFinish();*/
	glBindVertexArray(mVao[0]);
	glDrawArrays(GL_POINTS, 0, 6);
	glFlush();
}

void init(void)
{
	glGenVertexArrays(NumVAOs, VAOs);
	glBindVertexArray(VAOs[Triangles]);
	GLfloat vertices[NumVertices][2] = 
	{
		{-0.9f, -0.9f},
		{0.85f, -0.9f},
		{-0.9f, 0.85f},
		{0.9f, -0.85f},
		{0.9f, 0.9f},
		{-0.85f, 0.9f}
	};

	glGenBuffers(NumBuffers, Buffers);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

}
// 正反面（反转）
void display6()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glFrontFace(GL_CW);
	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_LINE);
	glBegin(GL_POLYGON);

	glVertex2f(-0.5, 0);
	glVertex2f(-0.5, -0.5);
	glVertex2f(0, -0.5);
	glVertex2f(0, 0);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex2f(0.5, 0);
	glVertex2f(0.5, 0.5);
	glVertex2f(0, 0.5);
	glVertex2f(0, 0);


	glEnd();
	glFlush();
}

void display5()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE_LOOP);
	glBegin(GL_POLYGON);

	glVertex3f(0.5, 0.5, 0.5);
	glVertex3f(-0.5, -0.3, 0.5);
	glVertex3f(0.3, 0.3, 0.1);
	glVertex3f(-0.2, 0.2, -0.4);

	glEnd();
	glFlush();
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	//glDrawPixels();
	//glColor3f(0, 0.5, 0.5);
	//glRectf(-0.5, 0.5, 0.5, -0.5);
	glFlush();
}
// 绘制点
void display1()
{
	glBegin(GL_LINE_STRIP);// GL_POINTS和GL_LINES，还有GL_LINE_STRIP，GL_LINE_LOOP，GL_TRIANGLES，GL_TRIANGLE_STRIP，GL_TRIANGLE_FAN
	glVertex2d(-0.8, 0.8);
	glVertex2f(0.4, 0.4);
	glVertex2f(-0.3, -0.3);
	glVertex2f(0.1, 0.3);

	glEnd();
	glFlush();
}
// 绘制圆
const int N = 200;
const GLfloat R = 0.5;
const GLfloat PI = 3.1415926;
void display2()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < N; i++)
	{
		GLfloat x = R * cos(2 * PI / N * i);
		GLfloat y = R * sin(2 * PI / N * i);	
		glVertex2f(x, y);
	}
	glEnd();
	glFlush();
}
// 绘制正弦曲线
const GLfloat factor = 0.1;
void display3()
{
	glClear(GL_COLOR_BUFFER_BIT);
	//glPointSize(0.5f);
	//glLineWidth(10.0f);
	glBegin(GL_LINES);
	glVertex2f(-1, 0);
	glVertex2f(1, 0);
	glVertex2f(0, -1);
	glVertex2f(0, 1);
	glEnd();

	glBegin(GL_POINTS);
	for (GLfloat x = -1.0f / factor; x < 1.0f / factor; x += 0.001f)
	{
		glVertex2f(x * factor, sin(x) * factor);

	}
	glEnd();

	glFlush();
}
// 绘制虚线
void display4()
{
	glClearColor(0, 0, 0, 0);
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(5, 0x0F0F);
	glBegin(GL_LINES);
	
	glVertex2f(-0.5, 0.5);
	glVertex2f(0.5, -0.5);
	
	glEnd();
	glDisable(GL_LINE_STIPPLE);
	glFlush();
}


