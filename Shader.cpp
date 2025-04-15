#include "graphics/Shader.h"
#include <GL/glew.h>  // GLEW must come before other GL includes
#include <GL/gl.h>
#include <iostream>
#include <vector>

namespace CardGameLib {
namespace Graphics {

Shader::Shader()
    : m_programId(0)
{
}

Shader::~Shader()
{
    if (m_programId != 0) {
        glDeleteProgram(m_programId);
    }
}

bool Shader::Compile(const std::string& vertexSource, const std::string& fragmentSource)
{
    // Create shader objects
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Set the shader source code
    const char* vShaderCode = vertexSource.c_str();
    const char* fShaderCode = fragmentSource.c_str();
    glShaderSource(vertexShader, 1, &vShaderCode, nullptr);
    glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);
    
    // Compile the shaders
    glCompileShader(vertexShader);
    if (!CheckCompileErrors(vertexShader, "VERTEX")) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    
    glCompileShader(fragmentShader);
    if (!CheckCompileErrors(fragmentShader, "FRAGMENT")) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    
    // Create shader program
    m_programId = glCreateProgram();
    glAttachShader(m_programId, vertexShader);
    glAttachShader(m_programId, fragmentShader);
    glLinkProgram(m_programId);
    
    if (!CheckCompileErrors(m_programId, "PROGRAM")) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(m_programId);
        m_programId = 0;
        return false;
    }
    
    // Delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return true;
}

void Shader::Use() const
{
    if (m_programId != 0) {
        glUseProgram(m_programId);
    }
}

void Shader::SetBool(const std::string& name, bool value) const
{
    glUniform1i(GetUniformLocation(name), static_cast<int>(value));
}

void Shader::SetInt(const std::string& name, int value) const
{
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetFloat(const std::string& name, float value) const
{
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetVec2(const std::string& name, float x, float y) const
{
    glUniform2f(GetUniformLocation(name), x, y);
}

void Shader::SetVec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(GetUniformLocation(name), x, y, z);
}

void Shader::SetVec4(const std::string& name, float x, float y, float z, float w) const
{
    glUniform4f(GetUniformLocation(name), x, y, z, w);
}

void Shader::SetMatrix4(const std::string& name, const float* value) const
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, value);
}

int Shader::GetUniformLocation(const std::string& name) const
{
    // Check if we've already cached this uniform location
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end()) {
        return it->second;
    }
    
    // If not, get the location and cache it
    int location = glGetUniformLocation(m_programId, name.c_str());
    
    // Cache even if it's -1 (not found) to avoid repeated lookups
    const_cast<Shader*>(this)->m_uniformCache[name] = location;
    
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader program." << std::endl;
    }
    
    return location;
}

bool Shader::CheckCompileErrors(unsigned int shader, const std::string& type)
{
    int success;
    char infoLog[1024];
    
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog 
                    << "\n -- --------------------------------------------------- -- " << std::endl;
            return false;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog 
                    << "\n -- --------------------------------------------------- -- " << std::endl;
            return false;
        }
    }
    
    return true;
}

} // namespace Graphics
} // namespace CardGameLib
