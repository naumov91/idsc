
#include "Renderable.h"

#include <string.h>
#include "render_utils.h"

Renderable::Renderable(Shader * shader, bool is_indexed, bool is_instanced, GLenum draw_mode) {
    this->shader = shader;
    this->is_indexed = is_indexed;
    this->is_instanced = is_instanced;

    attributes = std::vector<model_attribute_t>();
    textures = std::vector<model_texture_t>();
    uniforms = std::unordered_map<std::string, model_uniform_t>();
    this->draw_mode = draw_mode;
    indices_buffer_id = -1;
}

void Renderable::bind_attribute(float * buffer, int size, int channels, char * attribute_name) {
    GLint attribute_location = glGetAttribLocation(shader->get_id(), attribute_name);
    if (attribute_location == -1) {
        // fprintf(stderr, "could not find attribute %s\n", attribute_name);
        return;
    }

    GLuint buffer_id;
    glGenBuffers(1, &buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
    glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW);

    model_attribute_t attribute = {
        .buffer_id = buffer_id,
        .attribute_id = attribute_location,
        .num_channels = channels
    };

    attributes.push_back(attribute);
    check_gl_error();
}

void Renderable::bind_indices(int * buffer, int size) {
    glGenBuffers(1 , &indices_buffer_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_buffer_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW);
    num_indices = size / 4;
    check_gl_error();
}

void Renderable::bind_uniform(float * buffer, uniform_data_t type, int count, char * uniform_name) {
    GLint uniform_location = glGetUniformLocation(shader->get_id(), uniform_name);
    if (uniform_location == -1) {
        fprintf(stderr, "could not find uniform %s\n", uniform_name);
        return;
    }

    int channels;
    switch (type) {
        case FLOAT:
            channels = 1;
            break;
        case VEC2_FLOAT:
            channels = 2;
            break;
        case VEC3_FLOAT:
            channels = 3;
            break;
        case VEC4_FLOAT:
            channels = 4;
            break;
        case MAT3_FLOAT:
            channels = 9;
            break;
        case MAT4_FLOAT:
            channels = 16;
            break;
        case INT:
            channels = 1;
            break;
        case VEC2_INT:
            channels = 2;
            break;
        case VEC3_INT:
            channels = 3;
            break;
        case VEC4_INT:
            channels = 4;
            break;
    }

    int size = sizeof(float);
    if (uniforms.count(uniform_name) == 0) {
        void * copied_data = (void *) malloc(size * channels * count);
        memcpy(copied_data, buffer, size * channels * count);
        model_uniform_t uniform = {
            .data = copied_data,
            .location = uniform_location,
            .type = type,
            .count = count
        };
        uniforms.emplace(uniform_name, uniform);
    } else {
        model_uniform_t uniform = uniforms.at(uniform_name);
        if (uniform.type != type || uniform.count != count) {
            fprintf(stderr, "uniform data size does not match previous use %s\n", uniform_name);
            return;
        }
        memcpy(uniform.data, buffer, size * channels * count);
    }
    check_gl_error();
}

void Renderable::bind_2d_texture(const void * data, int width, int height, GLenum type, int channels, char * texture_name) {
    // TODO: return texture id.  allow to bind previous texture ids or free them
    GLint uniform_id = glGetUniformLocation(shader->get_id(), texture_name);
    if (uniform_id == -1) {
        fprintf(stderr, "could not find texture attribute %s\n", texture_name);
        return;
    }

    for (unsigned int i = 0; i < textures.size(); i++) {
        model_texture_t tex = textures.at(i);
        if (tex.texture_uniform_id == uniform_id) {
            glDeleteTextures(1, &tex.texture_id);
            textures.erase(textures.begin() + i);
            break;
        }
    }

    static const GLenum formats [] = {
        GL_RED, GL_RG, GL_RGB, GL_RGBA
    };
    GLenum format = formats[channels - 1];

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    GLuint texture_unit = textures.size();

    model_texture_t texture = {
        .texture_unit = texture_unit,
        .texture_id = texture_id,
        .texture_uniform_id = uniform_id,
        .texture_type = GL_TEXTURE_2D
    };

    textures.push_back(texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderable::render() {

    for (auto kvPair : uniforms) {
        model_uniform_t u = kvPair.second;
        switch (u.type) {
            case FLOAT:
                glUniform1fv(u.location, u.count, (GLfloat *) u.data);
                break;
            case VEC2_FLOAT:
                glUniform2fv(u.location, u.count, (GLfloat *) u.data);
                break;
            case VEC3_FLOAT:
                glUniform3fv(u.location, u.count, (GLfloat *) u.data);
                break;
            case VEC4_FLOAT:
                glUniform4fv(u.location, u.count, (GLfloat *) u.data);
                break;
            case MAT3_FLOAT:
                glUniformMatrix3fv(u.location, u.count, GL_FALSE, (GLfloat *) u.data);
                break;
            case MAT4_FLOAT:
                glUniformMatrix4fv(u.location, u.count, GL_FALSE, (GLfloat *) u.data);
                break;
            case INT:
                glUniform1iv(u.location, u.count, (GLint *) u.data);
                break;
            case VEC2_INT:
                glUniform2iv(u.location, u.count, (GLint *) u.data);
                break;
            case VEC3_INT:
                glUniform3iv(u.location, u.count, (GLint *) u.data);
                break;
            case VEC4_INT:
                glUniform4iv(u.location, u.count, (GLint *) u.data);
                break;
        }
    }
    check_gl_error();

    for (model_texture_t texture : textures) {
        glActiveTexture(texture.texture_unit + GL_TEXTURE0);
        glBindTexture(texture.texture_type, texture.texture_id);
        glUniform1i(texture.texture_uniform_id, texture.texture_unit);
    }
    check_gl_error();

    for (model_attribute_t attribute : attributes) {
        glEnableVertexAttribArray(attribute.attribute_id);
        glBindBuffer(GL_ARRAY_BUFFER, attribute.buffer_id);
        glVertexAttribPointer(attribute.attribute_id, attribute.num_channels, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    }
    check_gl_error();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_buffer_id);
    if (is_instanced) {
        if (is_indexed) {
            // glDrawElementsInstanced(draw_mode, num_indices, GL_UNSIGNED_INT, NULL, num_instances);
        } else {
            // glDrawArraysInstanced(draw_mode, num_indices, GL_UNSIGNED_INT, NULL, num_instances);
        }
    } else {
        if (is_indexed) {
            glDrawElements(draw_mode, num_indices, GL_UNSIGNED_INT, NULL);
        } else {
            // glDrawArrays(draw_mode, num_indices, GL_UNSIGNED_INT, NULL);
        }
    }
    check_gl_error();

    for (model_attribute_t attribute : attributes) {
        glDisableVertexAttribArray(attribute.attribute_id);
    }
    glActiveTexture(GL_TEXTURE0);
    check_gl_error();
}

