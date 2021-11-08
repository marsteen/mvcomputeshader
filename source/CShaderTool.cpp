
#include <iostream>
#include <string>
#include <GL/glew.h>
#include <CShaderTool.h>

using namespace std;

//---------------------------------------------------------------------------
//
// Klasse:    CShaderTool
// Methode:   init
//
//---------------------------------------------------------------------------

void CShaderTool::init()
{
    cout << "CShaderTool::Init START" << endl;
    texHandle = genTexture();
    cout << "genTexture ok" << endl;
    renderHandle = genRenderProg(texHandle);
    cout << "genRenderProg ok" << endl;
    computeHandle = genComputeProg(texHandle);
    cout << "genComputeProg ok" << endl;
    cout << "CShaderTool::Init OK" << endl;
}


//---------------------------------------------------------------------------
//
// Klasse:    CShaderTool
// Methode:   checkErrors
//
//---------------------------------------------------------------------------

void CShaderTool::checkErrors(std::string desc)
{
    GLenum e = glGetError();

    if (e != GL_NO_ERROR)
    {
        fprintf(stderr, "OpenGL error in \"%s\": %s (%d)\n", desc.c_str(), gluErrorString(e), e);
        exit(20);
    }
}


//---------------------------------------------------------------------------
//
// Klasse:    CShaderTool
// Methode:   genRenderProg
//
//---------------------------------------------------------------------------

GLuint CShaderTool::genRenderProg(GLuint texHandle)
{
    GLuint progHandle = glCreateProgram();
    GLuint vp = glCreateShader(GL_VERTEX_SHADER);
    GLuint fp = glCreateShader(GL_FRAGMENT_SHADER);

    const char* vpSrc[] = {
        "#version 430\n",
        "in vec2 pos;\
		 out vec2 texCoord;\
		 void main() {\
			 texCoord = pos*0.5f + 0.5f;\
			 gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);\
		 }"
    };

    const char* fpSrc[] = {
        "#version 430\n",
        "uniform sampler2D srcTex;\
		 in vec2 texCoord;\
		 out vec4 color;\
		 void main() {\
			 float c = texture(srcTex, texCoord).x;\
			 color = vec4(c, 1.0, 1.0, 1.0);\
		 }"
    };

    glShaderSource(vp, 2, vpSrc, NULL);
    glShaderSource(fp, 2, fpSrc, NULL);

    glCompileShader(vp);
    int rvalue;

    glGetShaderiv(vp, GL_COMPILE_STATUS, &rvalue);
    if (!rvalue)
    {
        fprintf(stderr, "Error in compiling vp\n");
        exit(30);
    }
    glAttachShader(progHandle, vp);

    glCompileShader(fp);
    glGetShaderiv(fp, GL_COMPILE_STATUS, &rvalue);
    if (!rvalue)
    {
        fprintf(stderr, "Error in compiling fp\n");
        exit(31);
    }
    glAttachShader(progHandle, fp);

    glBindFragDataLocation(progHandle, 0, "color");
    glLinkProgram(progHandle);

    glGetProgramiv(progHandle, GL_LINK_STATUS, &rvalue);
    if (!rvalue)
    {
        fprintf(stderr, "Error in linking sp\n");
        exit(32);
    }

    glUseProgram(progHandle);
    glUniform1i(glGetUniformLocation(progHandle, "srcTex"), 0);

    GLuint vertArray;

    glGenVertexArrays(1, &vertArray);
    glBindVertexArray(vertArray);

    GLuint posBuf;

    glGenBuffers(1, &posBuf);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf);
    float data[] = {
        -1.0f, -1.0f,
        -1.0f,	1.0f,
        1.0f,  -1.0f,
        1.0f, 1.0f
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*8, data, GL_STREAM_DRAW);
    GLint posPtr = glGetAttribLocation(progHandle, "pos");

    glVertexAttribPointer(posPtr, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posPtr);

    checkErrors("Render shaders");
    return progHandle;
}


//---------------------------------------------------------------------------
//
// Klasse:    CShaderTool
// Methode:   genComputeProg
//
//---------------------------------------------------------------------------

GLuint CShaderTool::genComputeProg(GLuint texHandle)
{
    // Creating the compute shader, and the program object containing the shader
    GLuint progHandle = glCreateProgram();
    GLuint cs = glCreateShader(GL_COMPUTE_SHADER);

    // In order to write to a texture, we have to introduce it as image2D.
    // local_size_x/y/z layout variables define the work group size.
    // gl_GlobalInvocationID is a uvec3 variable giving the global ID of the thread,
    // gl_LocalInvocationID is the local index within the work group, and
    // gl_WorkGroupID is the work group's index
    const char* csSrc[] = {
        "#version 430\n",
        "uniform float roll;\
		 layout (r32f) uniform image2D destTex;\
		 layout (local_size_x = 16, local_size_y = 16) in;\
		 void main() {\
			 ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);\
			 float localCoef = length(vec2(ivec2(gl_LocalInvocationID.xy)-8)/8.0);\
			 float globalCoef = sin(float(gl_WorkGroupID.x+gl_WorkGroupID.y)*0.1 + roll)*0.5;\
			 imageStore(destTex, storePos, vec4(1.0-globalCoef*localCoef, 1.0, 1.0, 1.0));\
		 }"
    };

    glShaderSource(cs, 2, csSrc, NULL);
    glCompileShader(cs);
    int rvalue;

    glGetShaderiv(cs, GL_COMPILE_STATUS, &rvalue);
    if (!rvalue)
    {
        fprintf(stderr, "Error in compiling the compute shader\n");
        GLchar log[10240];
        GLsizei length;
        glGetShaderInfoLog(cs, 10239, &length, log);
        fprintf(stderr, "Compiler log:\n%s\n", log);
        exit(40);
    }
    glAttachShader(progHandle, cs);

    glLinkProgram(progHandle);
    glGetProgramiv(progHandle, GL_LINK_STATUS, &rvalue);
    if (!rvalue)
    {
        fprintf(stderr, "Error in linking compute shader program\n");
        GLchar log[10240];
        GLsizei length;
        glGetProgramInfoLog(progHandle, 10239, &length, log);
        fprintf(stderr, "Linker log:\n%s\n", log);
        exit(41);
    }
    glUseProgram(progHandle);

    glUniform1i(glGetUniformLocation(progHandle, "destTex"), 0);

    checkErrors("Compute shader");
    return progHandle;
}


//---------------------------------------------------------------------------
//
// Klasse:    CShaderTool
// Methode:   genTexture
//
//---------------------------------------------------------------------------

GLuint CShaderTool::genTexture()
{
    // We create a single float channel 512^2 texture
    GLuint texHandle;

    glGenTextures(1, &texHandle);

    cout << "glGenTextures ok:" << texHandle << endl;
    glActiveTexture(GL_TEXTURE0);
    cout << "glActiveTexture ok" << endl;
    glBindTexture(GL_TEXTURE_2D, texHandle);
    cout << "glBindTexture ok" << endl;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 512, 512, 0, GL_RED, GL_FLOAT, NULL);

    // Because we're also using this tex as an image (in order to write to it),
    // we bind it to an image unit as well
    glBindImageTexture(0, texHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
    cout << "glBindImageTexture ok" << endl;
    checkErrors("Gen texture");
    return texHandle;
}


//---------------------------------------------------------------------------
//
// Klasse:    CShaderTool
// Methode:   updateTex
//
//---------------------------------------------------------------------------

void CShaderTool::updateTex(int frame)
{
    glUseProgram(computeHandle);
    glUniform1f(glGetUniformLocation(computeHandle, "roll"), (float)frame*0.01f);
    glDispatchCompute(512/16, 512/16, 1); // 512^2 threads in blocks of 16^2
    checkErrors("Dispatch compute shader");
}


//---------------------------------------------------------------------------
//
// Klasse:    CShaderTool
// Methode:   draw
//
//---------------------------------------------------------------------------

void CShaderTool::draw()
{
    glUseProgram(renderHandle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    checkErrors("Draw screen");
}
=======



#include <iostream>
#include <string>
#include <GL/glew.h>
#include <CShaderTool.h>

using namespace std;

//---------------------------------------------------------------------------
//
// Klasse:    CShaderTool
// Methode:   init
//
//---------------------------------------------------------------------------

void CShaderTool::init()
{
	cout << "CShaderTool::Init START" << endl;
    texHandle = genTexture();
	cout << "genTexture ok" << endl;
	renderHandle  = genRenderProg(texHandle);
	cout << "genRenderProg ok" << endl;
	computeHandle = genComputeProg(texHandle);
	cout << "genComputeProg ok" << endl;
	cout << "CShaderTool::Init OK" << endl;
}

//---------------------------------------------------------------------------
//
// Klasse:    CShaderTool
// Methode:   checkErrors
//
//---------------------------------------------------------------------------

void CShaderTool::checkErrors(std::string desc) 
{
	GLenum e = glGetError();
	if (e != GL_NO_ERROR) 
	{
		fprintf(stderr, "OpenGL error in \"%s\": %s (%d)\n", desc.c_str(), gluErrorString(e), e);
		exit(20);
	}
}


static void showShaderCompileError(GLuint shader)
{
	int infoLen;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
	if (infoLen)
	{
		char* buf = (char*) malloc(infoLen);
		if (buf)
		{
			glGetShaderInfoLog(shader, infoLen, NULL, buf);
			cout << "Could not compile shader. Error: " << buf << endl;
			free(buf);
		}
	}
}

//---------------------------------------------------------------------------
//
// Klasse:    CShaderTool
// Methode:   genRenderProg
//
//---------------------------------------------------------------------------

GLuint CShaderTool::genRenderProg(GLuint texHandle) 
{
    GLuint progHandle = glCreateProgram();
    GLuint vp = glCreateShader(GL_VERTEX_SHADER);
    GLuint fp = glCreateShader(GL_FRAGMENT_SHADER);

	const char *vpSrc[] = {
		"#version 430\n",
		"in vec2 pos;\
		 out vec2 texCoord;\
		 void main() {\
			 texCoord = pos*0.5f + 0.5f;\
			 gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);\
		 }"
	};

	const char *fpSrc[] = {
		"#version 430\n",
		"uniform sampler2D srcTex;\
		 in vec2 texCoord;\
		 out vec4 color;\
		 void main() {\
			 float c = texture(srcTex, texCoord).x;\
			 color = vec4(c, 1.0, 1.0, 1.0);\
		 }"
	};

    glShaderSource(vp, 2, vpSrc, NULL);
    glShaderSource(fp, 2, fpSrc, NULL);

    glCompileShader(vp);
    int rvalue;
    glGetShaderiv(vp, GL_COMPILE_STATUS, &rvalue);
    if (!rvalue)
    {
    	showShaderCompileError(vp);
        fprintf(stderr, "Error in compiling vp\n");
        exit(30);
    }
    glAttachShader(progHandle, vp);

    glCompileShader(fp);
    glGetShaderiv(fp, GL_COMPILE_STATUS, &rvalue);
    if (!rvalue)
    {
		showShaderCompileError(fp);
        fprintf(stderr, "Error in compiling fp\n");
        exit(31);
    }
    glAttachShader(progHandle, fp);

	glBindFragDataLocation(progHandle, 0, "color");
    glLinkProgram(progHandle);

    glGetProgramiv(progHandle, GL_LINK_STATUS, &rvalue);
    if (!rvalue) {
        fprintf(stderr, "Error in linking sp\n");
        exit(32);
    }   
    
	glUseProgram(progHandle);
	glUniform1i(glGetUniformLocation(progHandle, "srcTex"), 0);

	GLuint vertArray;
    glGenVertexArrays(1, &vertArray);
	glBindVertexArray(vertArray);

	GLuint posBuf;
	glGenBuffers(1, &posBuf);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf);
	float data[] = {
		-1.0f, -1.0f,
		-1.0f, 1.0f,
		1.0f, -1.0f,
		1.0f, 1.0f
	};
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*8, data, GL_STREAM_DRAW);
	GLint posPtr = glGetAttribLocation(progHandle, "pos");
    glVertexAttribPointer(posPtr, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posPtr);

	checkErrors("Render shaders");
	return progHandle;
}

//---------------------------------------------------------------------------
//
// Klasse:    CShaderTool
// Methode:   genComputeProg
//
//---------------------------------------------------------------------------

GLuint CShaderTool::genComputeProg(GLuint texHandle)
{
	// Creating the compute shader, and the program object containing the shader
    GLuint progHandle = glCreateProgram();
    GLuint cs = glCreateShader(GL_COMPUTE_SHADER);

	// In order to write to a texture, we have to introduce it as image2D.
	// local_size_x/y/z layout variables define the work group size.
	// gl_GlobalInvocationID is a uvec3 variable giving the global ID of the thread,
	// gl_LocalInvocationID is the local index within the work group, and
	// gl_WorkGroupID is the work group's index
	const char *csSrc[] = {
		"#version 430\n",
		"uniform float roll;\
		 layout (r32f) uniform image2D destTex;\
		 layout (local_size_x = 32, local_size_y = 32) in;\
		 void main() {\
			 ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);\
			 float localCoef = length(vec2(ivec2(gl_LocalInvocationID.xy)-8)/8.0);\
			 float globalCoef = sin(float(gl_WorkGroupID.x+gl_WorkGroupID.y)*0.1 + roll)*0.5;\
			 imageStore(destTex, storePos, vec4(1.0-globalCoef*localCoef, 1.0, 1.0, 1.0));\
		 }"
	};

    glShaderSource(cs, 2, csSrc, NULL);
	glCompileShader(cs);
    int rvalue;
    glGetShaderiv(cs, GL_COMPILE_STATUS, &rvalue);
    if (!rvalue) {
        fprintf(stderr, "Error in compiling the compute shader\n");
        GLchar log[10240];
        GLsizei length;
        glGetShaderInfoLog(cs, 10239, &length, log);
        fprintf(stderr, "Compiler log:\n%s\n", log);
        exit(40);
    }
    glAttachShader(progHandle, cs);

    glLinkProgram(progHandle);
    glGetProgramiv(progHandle, GL_LINK_STATUS, &rvalue);
    if (!rvalue) {
        fprintf(stderr, "Error in linking compute shader program\n");
        GLchar log[10240];
        GLsizei length;
        glGetProgramInfoLog(progHandle, 10239, &length, log);
        fprintf(stderr, "Linker log:\n%s\n", log);
        exit(41);
    }   
	glUseProgram(progHandle);
    
	glUniform1i(glGetUniformLocation(progHandle, "destTex"), 0);

	checkErrors("Compute shader");
	return progHandle;
}

//---------------------------------------------------------------------------
//
// Klasse:    CShaderTool
// Methode:   genTexture
//
//---------------------------------------------------------------------------

GLuint CShaderTool::genTexture() 
{
	// We create a single float channel 512^2 texture
	glewInit();
	GLuint texHandle;
	glGenTextures(1, &texHandle);
	
	cout << "glGenTextures ok texture handle=" << texHandle << endl;
	glActiveTexture(GL_TEXTURE0);
	cout << "glActiveTexture ok" << endl;
	glBindTexture(GL_TEXTURE_2D, texHandle);
	cout << "glBindTexture ok" << endl;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 512, 512 , 0, GL_RED, GL_FLOAT, NULL);
	cout << "glTexImage2D ok" << endl;

	// Because we're also using this tex as an image (in order to write to it),
	// we bind it to an image unit as well
	glBindImageTexture(0, texHandle, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);
	cout << "glBindImageTexture ok" << endl;
	checkErrors("Gen texture");	
	return texHandle;
}

//---------------------------------------------------------------------------
//
// Klasse:    CShaderTool
// Methode:   updateTex
//
//---------------------------------------------------------------------------

void CShaderTool::updateTex(int frame)
{
	
	glUseProgram(computeHandle);
	glUniform1f(glGetUniformLocation(computeHandle, "roll"), (float)frame*0.01f);
	glDispatchCompute(512/16, 512/16, 1); // 512^2 threads in blocks of 16^2
	checkErrors("Dispatch compute shader");
	
}

//---------------------------------------------------------------------------
//
// Klasse:    CShaderTool
// Methode:   draw
//
//---------------------------------------------------------------------------

void CShaderTool::draw()
{
	glUseProgram(renderHandle);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	checkErrors("Draw screen");
}
>>>>>>> 0b0ff403ecb2616ddda7691694791e043907856f
