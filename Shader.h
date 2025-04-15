#pragma once

#include <string>
#include <unordered_map>

namespace CardGameLib {
namespace Graphics {

class Shader {
public:
    Shader();
    ~Shader();
    
    // Compile the shader from source code
    bool Compile(const std::string& vertexSource, const std::string& fragmentSource);
    
    // Activate the shader
    void Use() const;
    
    // Utility functions for setting uniforms
    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec2(const std::string& name, float x, float y) const;
    void SetVec3(const std::string& name, float x, float y, float z) const;
    void SetVec4(const std::string& name, float x, float y, float z, float w) const;
    void SetMatrix4(const std::string& name, const float* value) const;
    
    // Get shader program ID
    unsigned int GetProgramId() const { return m_programId; }
    
private:
    unsigned int m_programId;
    std::unordered_map<std::string, int> m_uniformCache;
    
    // Utility function to get uniform location with caching
    int GetUniformLocation(const std::string& name) const;
    
    // Check for compilation or linking errors
    bool CheckCompileErrors(unsigned int shader, const std::string& type);
};

} // namespace Graphics
} // namespace CardGameLib
