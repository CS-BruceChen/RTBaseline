#pragma once
#include <glad/glad.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <QDebug>
#include <glfw3.h>
#include <clipper/clipper.hpp>
#include <poly2tri/poly2tri.h>
#include <clip2tri/clip2tri.h>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE "RT Terminal"
#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 6

#define initOpenGL(){\
	glfwInit();\
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);\
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);\
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);\
    \
}

#define createWindow(SCR_WIDTH,SCR_HEIGHT,TITLE) glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, TITLE, NULL, NULL);

#define initWindowAndGlad(window){\
    if (window == NULL)\
    {\
        qDebug() << "Failed to create GLFW window"<<"\n";\
        glfwTerminate();\
        \
    }\
    glfwMakeContextCurrent(window);\
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);\
    \
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))\
    {\
        qDebug() << "Failed to initialize GLAD"<<"\n";\
        \
    }\
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);


class Shader
{
public:
    GLuint ID;//shader program ID
    Shader(std::vector<std::string> filePaths);
    Shader(std::vector<std::string> filePaths, unsigned blocksize);//for dynamic local_size
    void use();
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setUint(const std::string& name, unsigned value) const;
    void setVec2(const std::string& name, float x, float y) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec4(const std::string& name, float x, float y, float z, float w) const;
    void setVec4(const std::string& name, int x, int y, int z, int w) const;

private:
    std::string getShaderSourceCode(const std::string& filePath);
    std::string getShaderSourceCode(const std::string& filePath, unsigned blocksize);//for dynamic local_size
    void compileAndLinkShader(std::string& shaderString, char shaderType);
    void checkErrors(GLuint id, std::string type);//id means shader id or program id

public:
    static Shader* newShader(std::string name);
};

class FBO {
public:
    enum Attachment {
        NoAttachment,
        CombinedDepthStencil,
        Depth
    };
    FBO(int width, int height, Attachment attachment, GLenum target, GLenum internal_format);
    ~FBO();
    static void bindDefault() { glBindFramebuffer(GL_FRAMEBUFFER, 0);}

public:
    int getFBO() const { return fbo; }
    int width() const { return wd; }
    int height() const { return ht; }
    GLuint texture() const { return tex; }
    void bind() { glBindFramebuffer(GL_FRAMEBUFFER, this->fbo); }

private:
    int wd, ht;
    GLuint tex;
    GLuint fbo;
    GLuint depth;
    GLenum target;
};

struct Primitive {
    unsigned VAO;
    unsigned VBO;
    unsigned VNUM;
    Primitive(unsigned vertsNum, void* data);
};

//struct Trajectory {
//    unsigned VAO;
//    unsigned VBO;
//    unsigned VNUM;
//    Trajectory(unsigned vertsNum, void* data);
//};
//
//struct Polygon {
//    unsigned VAO;
//    unsigned VBO;
//    unsigned VNUM;
//    Polygon(unsigned vertsNum, void* data);
//};
//
//struct QueryTraj {
//    unsigned VAO;
//    unsigned VBO;
//    unsigned VNUM;
//    QueryTraj(unsigned vertsNum, void* data);
//};

struct Point {
    float x, y;
    Point() { x = 0; y = 0; }
    Point(float xx, float yy) { x = xx; y = yy; }
    Point(const Point& pt) { x = pt.x; y = pt.y; };
    template<class T, class U> Point(T in_x, U in_y) {
        x = static_cast<float>(in_x); 
        y = static_cast<float>(in_y); 
    }
};

typedef std::vector<Point> Sequence;

struct GLTextureBuffer {
    GLTextureBuffer() :texId(0), bufId(0) {}

public:
    void create(int size, GLenum format, void* data);
    void setData(int size, GLenum format, void* data);
    std::vector<int> getBuffer();
    std::vector<float> getBufferF();
    void destroy();
    int getSize() const { return size; }
    GLuint getTexId() const { return texId; }
    GLuint getBufId() const { return bufId; }

private:
    int size;
    GLuint texId, bufId;
};

struct DPInfo {
    DPInfo(std::vector<Point>& tq, std::vector<Point>& t);
    ~DPInfo();
public:
    unsigned get_wd() const { return wd; }
    unsigned get_ht() const { return ht; }
    bool get_isWdBiggerThanHt() const { return isWdBiggerThanHt; }
    unsigned get_min_wd_ht() const { return min_wd_ht; }
    unsigned get_d_w_h() const { return d_w_h; }
    unsigned get_total_step() const { return total_step; }
    std::vector<int> get_DPBuf() { return DPBuf.getBuffer(); }
    void setUniformDPInfo(Shader* s);

private:
    unsigned wd, ht;
    bool isWdBiggerThanHt;
    unsigned min_wd_ht;
    unsigned d_w_h;
    unsigned total_step;
    GLTextureBuffer DPBuf;
    GLTextureBuffer WXBuf, WYBuf, HXBuf, HYBuf;
    int* DP;
    float* WX;
    float* WY;
    float* HX;
    float* HY;
    unsigned distance(unsigned a, unsigned b) {
        if (a > b) return a - b;
        else return b - a;
    }
};

struct CoordBound
{
    float maxx, maxy, minx, miny;
    CoordBound(float maxu, float maxv, float minu, float minv) {
        maxx = maxu;
        maxy = maxv;
        minx = minu;
        miny = minv;
    }
};

void triangulatePolygons(Sequence& polys, std::vector<float>& verts, float ids);

