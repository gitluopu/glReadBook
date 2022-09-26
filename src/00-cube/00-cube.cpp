//////////////////////////////////////////////////////////////////////////////
//
//  test.cpp
//
//////////////////////////////////////////////////////////////////////////////

#include <vermilion.h>

#include "vgl.h"
#include "vapp.h"
#include "LoadShaders.h"
#include <array>
#include<iostream>
#include <cmath>
#include <vector>
#include "vmath.h"
#include "vermilion.h"

#include "vapp.h"
#include "vutils.h"
#include "vbm.h"

#include "vmath.h"
#include<memory>
using namespace vmath;
float aspect;
GLuint gCurShaderId;
GLint gCurWidth;
GLint gCurHeight;
mat4 gShadowMat;
static const vmath::vec3 X(1.0f, 0.0f, 0.0f);
static const vmath::vec3 Y(0.0f, 1.0f, 0.0f);
static const vmath::vec3 Z(0.0f, 0.0f, 1.0f);
class BaseShader{
public:
    void build(){
        m_id = glCreateProgram();
        vglAttachShaderSource(m_id, GL_VERTEX_SHADER, vShader().c_str());
        vglAttachShaderSource(m_id, GL_FRAGMENT_SHADER, fShader().c_str());
        glLinkProgram(m_id);
    }
    void begin(){
        glUseProgram(m_id);
        gCurShaderId = m_id;
    }
    void end(){
        glUseProgram(0);
        gCurShaderId = 0;
    }
    virtual std::string vShader() = 0;
    virtual std::string fShader() = 0;
    GLuint id()const{return m_id;}
private:
    GLuint m_id;
};
class PureColorShader:public BaseShader{
    virtual std::string vShader() override {
        return R"(
            #version 400 core
uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;
layout( location = 0 ) in vec4 vPosition;
void
main()
{
    gl_Position = projMat * viewMat * modelMat * vPosition;
}

)";
    }

    virtual std::string fShader() override {
        return R"(
            #version 400 core
out vec4 fColor;
uniform vec4 uColor;
void
main()
{
		fColor = uColor;
}

)";
    }
};
class BaseModel {
public:
    virtual mat4 modelMat(){
        return translate(0.0f, 0.0f, -5.0f)*scale(1.0f,1.0f,1.0f)*vmath::rotate(30.0f,X);
    }
    virtual mat4 viewMat(){
        return lookat(vec3(0,0,0),vec3(0,0,-1),vec3(0,1,0));
    }
    virtual mat4 projMat(){
        return frustum(-1.0f, 1.0f, -aspect, aspect, 1.0f, 10.0f);
    }
    mat4 vp(){
        const mat4 scale_bias_matrix = mat4(vec4(0.5f, 0.0f, 0.0f, 0.0f),
                                            vec4(0.0f, 0.5f, 0.0f, 0.0f),
                                            vec4(0.0f, 0.0f, 0.5f, 0.0f),
                                            vec4(0.5f, 0.5f, 0.5f, 1.0f));
        return scale_bias_matrix * projMat() * viewMat();
    }
    void setPureColor(const vec4 &color){
        m_pure_color = std::make_shared<vec4>(color);
    }
    void build(){
        _build();
        glGenVertexArrays(1, &m_vertex_id);
        glBindVertexArray(m_vertex_id);

        glGenBuffers( 1, &m_pos_id);
        glBindBuffer( GL_ARRAY_BUFFER, m_pos_id);
        glBufferData( GL_ARRAY_BUFFER, m_vertices.size()*sizeof(float)*3,
                      m_vertices.data(), GL_STATIC_DRAW );
        glVertexAttribPointer( 0, 3, GL_FLOAT,
                               GL_FALSE, 0, BUFFER_OFFSET(0) );
        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, 0);

        if(!m_tex_coords.empty()){
            glGenBuffers( 1, &m_tex_pos_id);
            glBindBuffer( GL_ARRAY_BUFFER, m_tex_pos_id);
            glBufferData( GL_ARRAY_BUFFER, m_tex_coords.size()*sizeof(float)*2,
                          m_tex_coords.data(), GL_STATIC_DRAW );
            glVertexAttribPointer( 1, 2, GL_FLOAT,
                                   GL_FALSE, 0, BUFFER_OFFSET(0) );
            glEnableVertexAttribArray( 1 );
            glBindBuffer( GL_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(0);
    }
    virtual void draw(){
        glBindVertexArray(m_vertex_id);
        glUniformMatrix4fv(glGetUniformLocation(gCurShaderId, "modelMat"), 1, GL_FALSE, modelMat());
        glUniformMatrix4fv(glGetUniformLocation(gCurShaderId, "viewMat"), 1, GL_FALSE, viewMat());
        glUniformMatrix4fv(glGetUniformLocation(gCurShaderId, "projMat"), 1, GL_FALSE, projMat());
        if(m_pure_color)
            glUniform4fv(glGetUniformLocation(gCurShaderId, "uColor"), 1, *m_pure_color);
        glDrawArrays( GL_TRIANGLES, 0, m_vertices.size() );
        glBindVertexArray(0);
    }
protected:
    virtual void _build()=0;
    GLuint m_vertex_id;
    GLuint m_pos_id;
    GLuint m_tex_pos_id;
    std::vector<vec3> m_vertices;
    std::vector<vec2> m_tex_coords;
    std::shared_ptr<vec4> m_pure_color;
};
class CubeModel:public BaseModel{
public:
    virtual void _build() override{
        //left
        m_vertices.push_back(vec3(-1,-1,-1));
        m_vertices.push_back(vec3(-1,-1,1));
        m_vertices.push_back(vec3(-1,1,-1));
        m_vertices.push_back(vec3(-1,-1,1));
        m_vertices.push_back(vec3(-1,1,1));
        m_vertices.push_back(vec3(-1,1,-1));

        //right
        m_vertices.push_back(vec3(1,-1,-1));
        m_vertices.push_back(vec3(1,-1,1));
        m_vertices.push_back(vec3(1,1,-1));
        m_vertices.push_back(vec3(1,-1,1));
        m_vertices.push_back(vec3(1,1,1));
        m_vertices.push_back(vec3(1,1,-1));

        //front
        m_vertices.push_back(vec3(-1,-1,-1));
        m_vertices.push_back(vec3(-1,1,-1));
        m_vertices.push_back(vec3(1,1,-1));
        m_vertices.push_back(vec3(1,1,-1));
        m_vertices.push_back(vec3(1,-1,-1));
        m_vertices.push_back(vec3(-1,-1,-1));

        //back
        m_vertices.push_back(vec3(-1,-1,1));
        m_vertices.push_back(vec3(-1,1,1));
        m_vertices.push_back(vec3(1,1,1));
        m_vertices.push_back(vec3(1,1,1));
        m_vertices.push_back(vec3(1,-1,1));
        m_vertices.push_back(vec3(-1,-1,1));

        //top
        m_vertices.push_back(vec3(-1,1,-1));
        m_vertices.push_back(vec3(-1,1,1));
        m_vertices.push_back(vec3(1,1,1));
        m_vertices.push_back(vec3(1,1,1));
        m_vertices.push_back(vec3(1,1,-1));
        m_vertices.push_back(vec3(-1,1,-1));

        //bottom
        m_vertices.push_back(vec3(-1,-1,-1));
        m_vertices.push_back(vec3(-1,-1,1));
        m_vertices.push_back(vec3(1,-1,1));
        m_vertices.push_back(vec3(1,-1,1));
        m_vertices.push_back(vec3(1,-1,-1));
        m_vertices.push_back(vec3(-1,-1,-1));

        setPureColor({0.2,0.2,0.2,1.0});
    }

    virtual mat4 modelMat()override{
        return translate(0.0f, 0.0f, -5.0f)*scale(1.0f,1.0f,1.0f)*vmath::rotate(0.0f,X);
    }
private:


};

class PlaneModel:public BaseModel{
protected:
    virtual void _build() override{
        //bottom
        m_vertices.push_back(vec3(-1,-1,-1));
        m_vertices.push_back(vec3(-1,-1,1));
        m_vertices.push_back(vec3(1,-1,1));
        m_vertices.push_back(vec3(1,-1,1));
        m_vertices.push_back(vec3(1,-1,-1));
        m_vertices.push_back(vec3(-1,-1,-1));

        setPureColor({0.8,0.8,0.8,1.0});
    }
    virtual mat4 modelMat()override{
        return translate(0.0f, 0.0f, -5.0f)*translate(0.0f, -1.0f, 0.0f)*scale(3.0f,1.0f,3.0f)*translate(0.0f, 1.0f, 0.0f);
    }
private:
};
class DepthTex{
public:
    void build(){
        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, k_width, k_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    int width()const{return k_width;}
    int height()const{return k_height;}
    GLuint id()const{return m_id;}
    void begin(){
        glBindTexture(GL_TEXTURE_2D, m_id);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    void end(){
        glBindTexture(GL_TEXTURE_2D, 0);
    }
private:
    GLuint m_id;
    const int k_width = 2048;
    const int k_height = 2048;
};
class FBO{
public:
    void build(){
        glGenFramebuffers(1, &m_id);
        glBindFramebuffer(GL_FRAMEBUFFER, m_id);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, m_depth_tex->id(), 0);
        glDrawBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void begin(){
        glBindFramebuffer(GL_FRAMEBUFFER, m_id);
        glViewport(0, 0, m_depth_tex->width(), m_depth_tex->height());
        glClearDepth(1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    void end(){
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, gCurWidth, gCurHeight);
    }
    void setDepthTex(DepthTex *tex){
        m_depth_tex = tex;
    }
private:
    GLuint m_id;
    DepthTex *m_depth_tex= nullptr;
};
class ShadowShader:public BaseShader {
    virtual std::string vShader() override {
        return R"(
            #version 400 core
uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;
uniform mat4 shadowMat;
out vec4 shadowCoord;
layout( location = 0 ) in vec4 vPosition;
void
main()
{
    vec4 world_pos = modelMat * vPosition;
    vec4 eye_pos = viewMat * world_pos;
    vec4 clip_pos = projMat * eye_pos;
    shadowCoord = shadowMat * world_pos;
//
    gl_Position = clip_pos;
//    gl_Position = projMat * viewMat * modelMat * vPosition;
}

)";
    }

    virtual std::string fShader() override {
        return R"(
            #version 400 core
uniform sampler2DShadow depth_texture;
in vec4 shadowCoord;
out vec4 fColor;
void
main()
{
        float f = textureProj(depth_texture,shadowCoord);
		fColor = vec4(vec3(f),1.0);
}

)";
    }
};
class CubeModelLight:public CubeModel{
public:
    virtual void _build(){
        CubeModel::_build();
        setPureColor({1.0});
    }
    virtual mat4 viewMat() override{
        return lookat(vec3(-0.8,0.8,0.9),vec3(0,0,-5),vec3(0,1,0));
    }
};
class PlaneModelLight:public PlaneModel{
public:
    virtual void _build(){
        PlaneModel::_build();
        setPureColor({1.0});
    }
    virtual mat4 viewMat() override{
        return lookat(vec3(-0.8,0.8,0.9),vec3(0,0,-5),vec3(0,1,0));
    }
};
class PlaneModelShadow:public PlaneModel{
public:
    virtual void draw() override{
        glUniformMatrix4fv(glGetUniformLocation(gCurShaderId, "shadowMat"), 1, GL_FALSE, gShadowMat);
        PlaneModel::draw();
    }

};
class QuadModel:public BaseModel{
public:
    virtual void _build() override{
        m_vertices.push_back(vec3(-1,-1,0));
        m_vertices.push_back(vec3(-1,1,0));
        m_vertices.push_back(vec3(1,1,0));

        m_vertices.push_back(vec3(-1,-1,0));
        m_vertices.push_back(vec3(1,-1,0));
        m_vertices.push_back(vec3(1,1,0));
        setPureColor(vec4{1,0,0,1});
    }
    virtual mat4 modelMat() override{
        return mat4::identity();
    }
    virtual mat4 viewMat() override{
        return mat4::identity();
    }
    virtual mat4 projMat() override{
        return mat4::identity();
    }
};
class QuadModelTex:public QuadModel{
public:
    virtual void _build() override{
        QuadModel::_build();
        m_tex_coords.push_back(vec2(0,0));
        m_tex_coords.push_back(vec2(0,1));
        m_tex_coords.push_back(vec2(1,1));

        m_tex_coords.push_back(vec2(0,0));
        m_tex_coords.push_back(vec2(1,0));
        m_tex_coords.push_back(vec2(1,1));
    }
private:
};
class QuadTexShader:public BaseShader{
public:
    virtual std::string vShader() override {
        return R"(
            #version 400 core
uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;
layout( location = 0 ) in vec4 vPosition;
layout( location = 1) in vec2 in_tex_coord;
out vec2 vs_tex_coord;
void
main()
{
    gl_Position = projMat * viewMat * modelMat * vPosition;
    vs_tex_coord = in_tex_coord;
}

)";
    }

    virtual std::string fShader() override {
        return R"(
            #version 400 core
out vec4 fColor;
uniform sampler2D tex;
in vec2 vs_tex_coord;
void
main()
{
		fColor = texture(tex,vs_tex_coord);

}

)";
    }
};
class ColorTex{
public:
    void build(){
        const int cellLen = 512;
        const int h = 128;
        std::vector<GLubyte> temp = {255,0,0,255};
        std::vector<GLubyte> data;
        for(int i=0;i<h;i++)
            for(int i=0;i<cellLen;i++)
                data.insert(data.end(),temp.begin(),temp.end());
        std::vector<GLubyte> temp1 = {0,255,0,255};
        for(int i=0;i<h;i++)
            for(int i=0;i<cellLen;i++)
                data.insert(data.end(),temp1.begin(),temp1.end());
        //set the texture
        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D,m_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cellLen, h*2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);// 图象在表面上重复出现
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//边缘截取
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D,0);
    }
    void begin(){
        glBindTexture(GL_TEXTURE_2D,m_id);
    }
    void end(){
        glBindTexture(GL_TEXTURE_2D,0);
    }
private:
    GLuint m_id;
};
BEGIN_APP_DECLARATION(LuopuMvp)
    virtual void Initialize(const char * title);
    virtual void Display(bool auto_redraw);
    virtual void Finalize(void);
    virtual void Resize(int width, int height);
    void OnKey(int key, int scancode, int action, int mods);
    void DrawScene(bool depth_only = false);
    CubeModel m_cube_model;
    PlaneModel m_plane_model;
    DepthTex m_depth_tex;
    FBO m_fbo;
    CubeModelLight m_cube_model_light;
    PlaneModelLight m_plane_model_light;
    ShadowShader m_shadow_shader;
    PlaneModelShadow m_plane_model_shadow;
    QuadModel m_quad_model;
    QuadModelTex m_quad_tex_model;
    QuadTexShader m_quad_tex_shader;
    ColorTex m_color_tex;
    PureColorShader m_pure_color_shader;
END_APP_DECLARATION()

DEFINE_APP(LuopuMvp, "luopu mvp")
//----------------------------------------------------------------------------
//
// init
//
//GLfloat matrix[16];
using namespace vmath;
GLint location;
vmath::mat4 vMat;


void LuopuMvp::Initialize(const char * title)
{
    base::Initialize(title);
    m_pure_color_shader.build();
    m_cube_model.build();
    m_plane_model.build();
    m_depth_tex.build();
    m_fbo.setDepthTex(&m_depth_tex);
    m_fbo.build();
    m_cube_model_light.build();
    m_plane_model_light.build();
    m_shadow_shader.build();
    m_plane_model_shadow.build();
    m_quad_model.build();
    m_quad_tex_model.build();
    m_quad_tex_shader.build();
    m_color_tex.build();
    m_pure_color_shader.build();
}

void LuopuMvp::OnKey(int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
            case GLFW_KEY_M:
            {
                static GLenum  mode = GL_FILL;

                mode = ( mode == GL_FILL ? GL_LINE : GL_FILL );
                glPolygonMode( GL_FRONT_AND_BACK, mode );
            }
                return;
        }
    }

    base::OnKey(key, scancode, action, mods);
}
static void depth1Begin(){
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}
static void depth1End(){
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
}
static void depth2Begin(){
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}
static void depth2End(){
    glDisable(GL_DEPTH_TEST);
}
//----------------------------------------------------------------------------
//
// display
//

void LuopuMvp::Display(bool auto_redraw)
{
    depth1Begin();
    m_pure_color_shader.begin();
    m_fbo.begin();
    gShadowMat = m_cube_model_light.vp();
    m_cube_model_light.draw();
    m_fbo.end();
    m_pure_color_shader.end();
    depth1End();
#if 0
    depth2Begin();
    m_pure_color_shader.begin();
    m_cube_model.draw();
    m_pure_color_shader.end();
    m_shadow_shader.begin();
    m_depth_tex.begin();
    m_plane_model_shadow.draw();
    m_depth_tex.end();
    m_shadow_shader.end();
    depth2End();
#else
    depth2Begin();
    m_quad_tex_shader.begin();
    m_depth_tex.begin();
    m_quad_tex_model.draw();
    m_depth_tex.end();
    m_quad_tex_shader.end();
    depth2End();
#endif
    base::Display(auto_redraw);
}

void LuopuMvp::Resize(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = float(height) / float(width);
    gCurWidth = width;
    gCurHeight = height;
}
void LuopuMvp::DrawScene(bool depth_only)
{
}
void LuopuMvp::Finalize(void)
{

}
