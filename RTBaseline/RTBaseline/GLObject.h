#pragma once
#include <glad/glad.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <QDebug>

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


struct Polygon {
    unsigned VAO;
    unsigned VBO;
    unsigned vertNum;
    Polygon(unsigned size, void* data);
};

struct Trajectory {
    unsigned VAO;
    unsigned VBO;
    unsigned vertNum;
    Trajectory(unsigned size, void* data);
};

struct QueryTraj {
    unsigned VAO;
    unsigned VBO;
    unsigned vertNum;
    QueryTraj(unsigned size, void* data);
};

struct Point {
    double x, y;
    Point() { x = 0; y = 0; }
    Point(double xx, double yy) { x = xx; y = yy; }
    Point(const Point& pt) { x = pt.x; y = pt.y; };
    template<class T, class U> Point(T in_x, U in_y) { x = static_cast<double>(in_x); y = static_cast<double>(in_y); }
};

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
    double maxx, maxy, minx, miny;
    CoordBound(double maxu, double maxv, double minu, double minv) {
        maxx = maxu;
        maxy = maxv;
        minx = minu;
        miny = minv;
    }
};


