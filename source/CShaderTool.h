#ifndef CShaderTool_H
#define CShaderTool_H

#include <GL/glew.h>

class CShaderTool
{
    public:

        void init();
        void updateTex(int frame);
        void draw();
        static void checkErrors(std::string desc);

    protected:

        static GLuint genRenderProg(GLuint texHandle);
        static GLuint genComputeProg(GLuint texHandle);
        static GLuint genTexture();

        GLuint renderHandle, computeHandle, texHandle;
};

#endif
