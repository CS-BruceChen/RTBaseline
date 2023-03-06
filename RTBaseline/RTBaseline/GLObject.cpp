#include "GLObject.h"

//shader implementation
Shader* Shader::newShader(std::string name) {
    std::vector<std::string> shader;
    std::string vs = "./" + name + ".vs.glsl";
    std::string fs = "./" + name + ".fs.glsl";
    shader.push_back(vs);
    shader.push_back(fs);
    return new Shader(shader);
}

Shader::Shader(std::vector<std::string> filePaths) {
    ID = glCreateProgram();
    for (int i = 0; i < (int)filePaths.size(); ++i) {
        int len = filePaths[i].size();
        char shaderType = filePaths[i][len - 7];//[name].[type char]s.glsl,
        std::string shaderString = getShaderSourceCode(filePaths[i]);
        compileAndLinkShader(shaderString, shaderType);

    }
}
Shader::Shader(std::vector<std::string> filePaths, unsigned blocksize) {
    ID = glCreateProgram();
    for (int i = 0; i < (int)filePaths.size(); ++i) {
        int len = filePaths[i].size();
        char shaderType = filePaths[i][len - 7];//[name].[type char]s.glsl,
        std::string shaderString = getShaderSourceCode(filePaths[i], blocksize);
        compileAndLinkShader(shaderString, shaderType);

    }
}

void Shader::use() {
    glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setUint(const std::string& name, unsigned value) const {
    glUniform1ui(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------

void Shader::setVec2(const std::string& name, float x, float y) const {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}
// ------------------------------------------------------------------------

void Shader::setVec3(const std::string& name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}
// ------------------------------------------------------------------------

void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const {
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Shader::setVec4(const std::string& name, int x, int y, int z, int w) const {
    glUniform4i(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

std::string Shader::getShaderSourceCode(const std::string& filePath) {
    std::string computeShaderString;
    std::ifstream computeShaderFile;
    computeShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        computeShaderFile.open(filePath);
        std::stringstream computeShaderStream;
        computeShaderStream << computeShaderFile.rdbuf();
        computeShaderFile.close();
        computeShaderString = computeShaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
         qDebug() << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << "\n";
    }

    return computeShaderString;
}

std::string Shader::getShaderSourceCode(const std::string& filePath, unsigned blocksize) {
    std::string computeShaderString;
    std::ifstream computeShaderFile;
    computeShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        computeShaderFile.open(filePath);
        std::stringstream computeShaderStream;
        computeShaderStream << computeShaderFile.rdbuf();
        computeShaderFile.close();
        computeShaderString = computeShaderStream.str();
        //set blocksize
        std::vector<unsigned> lineBreaksIndex;
        for (unsigned i = 0; i < computeShaderString.size(); ++i) {
            if (computeShaderString[i] == '\n') lineBreaksIndex.push_back(i);
        }
        for (unsigned i = 1; i < lineBreaksIndex.size(); ++i) {
            unsigned pos = lineBreaksIndex[i - 1] + 1;
            unsigned len = lineBreaksIndex[i] - lineBreaksIndex[i - 1] - 1;
            std::string lineString = computeShaderString.substr(pos, len);
            if (lineString == "#define BLOCKSIZE 518") {
                qDebug() << "BLOCKSIZE is successfully changed" << "\n";
                computeShaderString.replace(pos, len, "#define BLOCKSIZE " + std::to_string(blocksize));
            }
        }

    }
    catch (std::ifstream::failure& e)
    {
        qDebug() << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << "\n";
    }

    return computeShaderString;
}


void Shader::compileAndLinkShader(std::string& shaderString, char shaderType) {
    const char* shaderCode = shaderString.c_str();
    GLuint shaderID = 0;
    std::string checkErroType = "";


    if (shaderType == 'v') {//vertex shader
        shaderID = glCreateShader(GL_VERTEX_SHADER);
        checkErroType = "VERTEX";
    }
    else if (shaderType == 'f') {//fragment shader
        shaderID = glCreateShader(GL_FRAGMENT_SHADER);
        checkErroType = "FRAGMENT";
    }
    else if (shaderType == 'c') {//compute shader
        shaderID = glCreateShader(GL_COMPUTE_SHADER);
        checkErroType = "COMPUTE";
    }
    else if (shaderType == 'g') {//geometery shader
        shaderID = glCreateShader(GL_GEOMETRY_SHADER);
        checkErroType = "GEOMETRY";
    }

    //compile shader
    glShaderSource(shaderID, 1, &shaderCode, NULL);
    glCompileShader(shaderID);
    checkErrors(shaderID, checkErroType);

    //link shader
    glAttachShader(ID, shaderID);
    glLinkProgram(ID);
    checkErrors(ID, "PROGRAM");

    //delete shader
    glDeleteShader(shaderID);
}

void Shader::checkErrors(GLuint id, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(id, 1024, NULL, infoLog);
            qDebug() << "ERROR::SHADER_COMPILATION_ERROR of type: " << QString::fromStdString(type) << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << "\n";
        }
    }
    else
    {
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(id, 1024, NULL, infoLog);
            qDebug() << "ERROR::PROGRAM_LINKING_ERROR of type: " << QString::fromStdString(type) << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << "\n";
        }
    }
}

//fbo implementation
FBO::FBO(int width, int height, Attachment attachment, GLenum target, GLenum internal_format) {
    tex = 0; fbo = 0; depth = 0;
    this->wd = width;
    this->ht = height;
    this->target = target;
    if (attachment == CombinedDepthStencil) {
        qDebug() << "attachment not yet implemented" << "\n";
        return;
    }
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &tex);
    glBindTexture(target, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(target, 0, internal_format, width, height, 0, internal_format, GL_FLOAT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    if (attachment == Depth) {
        glGenRenderbuffers(1, &depth);
        glBindRenderbuffer(GL_RENDERBUFFER, depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        qDebug() << "Error in FBO creation!!!!!!!!!!!!!!!!!!!!!!" << "\n";
    }
    bindDefault();
}

FBO::~FBO() {
    if (fbo != 0) {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }
    if (tex != 0) {
        glDeleteTextures(1, &tex);
        tex = 0;
    }
    if (depth != 0) {
        glDeleteRenderbuffers(1, &depth);
        depth = 0;
    }
}

void GLTextureBuffer::create(int size, GLenum format, void* data) {
    this->size = size;
    GLenum err;

    if (bufId > 0)
        glDeleteBuffers(1, &bufId);  //delete previously created tbo

    glGenBuffers(1, &bufId);

    glBindBuffer(GL_TEXTURE_BUFFER, bufId);
    glBufferData(GL_TEXTURE_BUFFER, size, data, GL_DYNAMIC_DRAW);

    err = glGetError();
    if (err > 0) {
        qDebug() << "createTextureBuffer error 1: " << err << "\n";
    }

    if (texId > 0)
        glDeleteTextures(1, &texId); //delete previously created texture

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_BUFFER, texId);
    glTexBuffer(GL_TEXTURE_BUFFER, format, bufId);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    err = glGetError();
    if (err > 0) {
        qDebug() << "createTextureBuffer error 2: " << err << "\n";
    }
}

void GLTextureBuffer::setData(int size, GLenum format, void* data) {
    this->size = size;
    GLenum err;

    if (bufId <= 0) {
        qDebug() << "buffer not created!!";
        return;
    }

    glBindBuffer(GL_TEXTURE_BUFFER, bufId);
    glBufferData(GL_TEXTURE_BUFFER, size, data, GL_DYNAMIC_DRAW);

    err = glGetError();
    if (err > 0) {
        qDebug() << "set data TextureBuffer error 1: " << err << "\n";
    }

    if (texId <= 0) {
        qDebug() << "texture buffer not created!!";
        return;
    }

    glBindTexture(GL_TEXTURE_BUFFER, texId);
    glTexBuffer(GL_TEXTURE_BUFFER, format, bufId);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    err = glGetError();
    if (err > 0) {
        qDebug() << "set data TextureBuffer error 2: " << err << "\n";
    }
}
std::vector<int> GLTextureBuffer::getBuffer() {
    std::vector<int> data(size / sizeof(int));
    glBindBuffer(GL_TEXTURE_BUFFER, bufId);
    GLenum err = glGetError();
    glGetBufferSubData(GL_TEXTURE_BUFFER, 0, size, data.data());
    err = glGetError();
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    err = glGetError();
    if (err > 0) {
        qDebug() << "getBuffer error: " << err << "\n";
    }
    return data;
}
std::vector<float> GLTextureBuffer::getBufferF() {
    std::vector<float> data(size / sizeof(float));
    glBindBuffer(GL_TEXTURE_BUFFER, bufId);
    GLenum err = glGetError();
    glGetBufferSubData(GL_TEXTURE_BUFFER, 0, size, data.data());
    err = glGetError();
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    err = glGetError();
    if (err > 0) {
        qDebug() << "getBuffer error: " << err << "\n";
    }
    return data;
}
void GLTextureBuffer::destroy() {
    if (bufId > 0)
        glDeleteBuffers(1, &bufId);  //delete previously created tbo

    if (texId > 0)
        glDeleteTextures(1, &texId); //delete previously created texture

    bufId = 0;
    texId = 0;
};

DPInfo::DPInfo(std::vector<Point>& tq, std::vector<Point>& t) {
    wd = tq.size();
    ht = t.size();
    isWdBiggerThanHt = wd > ht ? true : false;
    min_wd_ht = wd < ht ? wd : ht;
    d_w_h = distance(wd, ht);
    total_step = wd + ht;

    //init DPBuf
    DP = new int[(wd + 1) * (ht + 1)];
    for (unsigned i = 0; i <= wd; ++i) {
        for (unsigned j = 0; j <= ht; ++j) {
            int val = 0;
            if (i == 0) val = j;
            else if (j == 0) val = i;
            DP[i + j * (wd + 1)] = val;
        }
    }
    DPBuf.create((wd + 1) * (ht + 1) * sizeof(int), GL_R32I, DP);
    glBindImageTexture(0, DPBuf.getTexId(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);

    //init Points Buf
    WX = new float[wd];
    WY = new float[wd];
    HX = new float[ht];
    HY = new float[ht];
    for (unsigned i = 0; i < wd; ++i) {
        WX[i] = tq[i].x;
        WY[i] = tq[i].y;
    }
    for (unsigned i = 0; i < ht; ++i) {
        HX[i] = t[i].x;
        HY[i] = t[i].y;
    }
    WXBuf.create(wd * sizeof(float), GL_R32F, WX);
    WYBuf.create(wd * sizeof(float), GL_R32F, WY);
    HXBuf.create(ht * sizeof(float), GL_R32F, HX);
    HYBuf.create(ht * sizeof(float), GL_R32F, HY);
    glBindImageTexture(1, WXBuf.getTexId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    glBindImageTexture(2, WYBuf.getTexId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    glBindImageTexture(3, HXBuf.getTexId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    glBindImageTexture(4, HYBuf.getTexId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
}

DPInfo::~DPInfo() {
    delete[] DP;
    delete[] WX;
    delete[] WY;
    delete[] HX;
    delete[] HY;
    DPBuf.destroy();
    WXBuf.destroy();
    WYBuf.destroy();
    HXBuf.destroy();
    HYBuf.destroy();
}

void DPInfo::setUniformDPInfo(Shader* s) {
    s->use();
    s->setUint("dpinfo.wd", wd);
    s->setUint("dpinfo.ht", ht);
    s->setBool("dpinfo.isWdBiggerThanHt", isWdBiggerThanHt);
    s->setUint("dpinfo.min_wd_ht", min_wd_ht);
    s->setUint("dpinfo.d_w_h", d_w_h);
    s->setUint("dpinfo.total_step", total_step);
}




