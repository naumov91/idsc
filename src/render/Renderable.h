

#ifndef _RENDERABLE_H
#define _RENDERABLE_H

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <vector>
#include <string>
#include <unordered_map>

#include "Shader.h"

typedef enum {
    FLOAT,
    VEC2_FLOAT,
    VEC3_FLOAT,
    VEC4_FLOAT,
    MAT3_FLOAT,
    MAT4_FLOAT,
    INT,
    VEC2_INT,
    VEC3_INT,
    VEC4_INT
} uniform_data_t;

typedef struct _model_attribute_t {
    GLuint buffer_id;
    GLint attribute_id;
    int num_channels;
} model_attribute_t;

typedef struct _model_texture_t {
    GLenum texture_unit;
    GLuint texture_id;
    GLint texture_uniform_id;
    GLenum texture_type;
} model_texture_t;

typedef struct _model_uniform_t {
    void * data;
    GLint location;
    uniform_data_t type;
    int count;
} model_uniform_t;

class Renderable {
    public:
        Renderable(Shader * shader, bool is_indexed, bool is_instanced, GLenum draw_mode);
        void bind_attribute(float * buffer, int size, int channels, char * attribute_name);
        void bind_indices(int * buffer, int size);
        void bind_uniform(float * buffer, uniform_data_t type, int count, char * uniform_name);
        void bind_2d_texture(const void * data, int width, int height, GLenum type, int channels, char * texture_name);
        void render();
    private:
        Shader * shader;
        bool is_indexed;
        bool is_instanced;
        std::vector<model_attribute_t> attributes;
        std::vector<model_texture_t> textures;
        std::unordered_map<std::string, model_uniform_t> uniforms;
        GLenum draw_mode;
        GLuint indices_buffer_id;
        int num_indices;
};

#endif

