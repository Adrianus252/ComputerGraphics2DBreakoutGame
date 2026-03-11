#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include <glbinding/gl46core/gl.h>
using namespace gl46core;


// surface of an object 

struct Material {
    enum MaterialMode { e2D, e3D };
    
    // for 2D: use_texture and solid_color parameters
    void bind(bool use_texture = false, const glm::vec4& solid_color = glm::vec4(1.0f)) {
        if (_mode == e2D) {      
            // when texture is enabled, texture * white, otherwise: solid color from object
            if (use_texture) {
                glUniform4f(3, 1.0f, 1.0f, 1.0f, 1.0f); // white color to show texture as-is
            } else {
                // Solid color from the object
                glUniform4f(3, solid_color.r, solid_color.g, solid_color.b, solid_color.a);
            }
            glUniform1i(4, use_texture ? 1 : 0);
        } else {
            // 3D lit: texture contribution, specular, shininess
            glUniform1f(17, _texture_contribution);
            glUniform1f(18, _specular);
            glUniform1f(19, _specular_shininess);
        }
    }
    
    MaterialMode _mode = e3D; // default
    glm::vec4 _base_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // white by default
    // 3D properties
    float _texture_contribution = 0.0f;
    float _specular = 1.0f;
    float _specular_shininess = 32.0f;
};