#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "graphics/Shader.h"
#include "graphics/Texture.h"

namespace CardGameLib {
namespace Graphics {

// Forward declarations
class CardSprite;

struct Vertex {
    float x, y, z;       // Position
    float r, g, b, a;    // Color
    float s, t;          // Texture coordinates
    
    Vertex() : x(0), y(0), z(0), r(1), g(1), b(1), a(1), s(0), t(0) {}
    Vertex(float x, float y, float z, float r, float g, float b, float a, float s, float t)
        : x(x), y(y), z(z), r(r), g(g), b(b), a(a), s(s), t(t) {}
};

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    // Initialization
    bool Initialize(int width, int height);
    void Shutdown();
    
    // Window management
    void Resize(int width, int height);
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    
    // Rendering
    void BeginFrame();
    void EndFrame();
    
    // Resource management
    std::shared_ptr<Shader> CreateShader(const std::string& name, 
                                       const std::string& vertexSource, 
                                       const std::string& fragmentSource);
    std::shared_ptr<Shader> GetShader(const std::string& name);
    
    std::shared_ptr<Texture> CreateTexture(const std::string& name,
                                        int width, int height, 
                                        const unsigned char* data, 
                                        int channels);
    std::shared_ptr<Texture> GetTexture(const std::string& name);
    
    // Drawing functions
    void DrawQuad(float x, float y, float width, float height, float r, float g, float b, float a);
    void DrawTexturedQuad(float x, float y, float width, float height, 
                         const std::shared_ptr<Texture>& texture);
    void DrawCardSprite(const CardSprite& cardSprite);
    void DrawText(const std::string& text, float x, float y, float scale, float r, float g, float b);
    
    // Immediate mode functions
    void Begin2D();
    void End2D();
    
private:
    // Window and context
    int m_width;
    int m_height;
    
    // OpenGL objects
    unsigned int m_vao;
    unsigned int m_vbo;
    unsigned int m_ebo;
    
    // Shaders and textures
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
    
    // Default resources
    std::shared_ptr<Shader> m_defaultShader;
    std::shared_ptr<Shader> m_textShader;
    std::shared_ptr<Texture> m_fontTexture;
    
    // Create default resources
    void CreateDefaultResources();
    void CreateDefaultShaders();
    void CreateFontTexture();
    
    // Internal utility functions
    void SetupBuffers();
};

} // namespace Graphics
} // namespace CardGameLib
