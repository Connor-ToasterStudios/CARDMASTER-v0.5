#include "graphics/Renderer.h"
#include "graphics/CardSprite.h"
#include <GL/glew.h>  // GLEW must come before other GL includes
#include <GL/gl.h>
#include <stdexcept>
#include <iostream>

namespace CardGameLib {
namespace Graphics {

// Default vertex shader
const char* DEFAULT_VERTEX_SHADER = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec4 aColor;
    layout (location = 2) in vec2 aTexCoord;
    
    out vec4 vertexColor;
    out vec2 texCoord;
    
    uniform mat4 projection;
    uniform mat4 model;
    
    void main()
    {
        gl_Position = projection * model * vec4(aPos, 1.0);
        vertexColor = aColor;
        texCoord = aTexCoord;
    }
)";

// Default fragment shader
const char* DEFAULT_FRAGMENT_SHADER = R"(
    #version 330 core
    in vec4 vertexColor;
    in vec2 texCoord;
    
    out vec4 FragColor;
    
    uniform sampler2D texture1;
    uniform bool useTexture;
    
    void main()
    {
        if (useTexture)
            FragColor = texture(texture1, texCoord) * vertexColor;
        else
            FragColor = vertexColor;
    }
)";

// Text vertex shader
const char* TEXT_VERTEX_SHADER = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec4 aColor;
    layout (location = 2) in vec2 aTexCoord;
    
    out vec4 vertexColor;
    out vec2 texCoord;
    
    uniform mat4 projection;
    
    void main()
    {
        gl_Position = projection * vec4(aPos, 1.0);
        vertexColor = aColor;
        texCoord = aTexCoord;
    }
)";

// Text fragment shader
const char* TEXT_FRAGMENT_SHADER = R"(
    #version 330 core
    in vec4 vertexColor;
    in vec2 texCoord;
    
    out vec4 FragColor;
    
    uniform sampler2D fontTexture;
    
    void main()
    {
        vec4 sampled = vec4(1.0, 1.0, 1.0, texture(fontTexture, texCoord).r);
        FragColor = vertexColor * sampled;
    }
)";

Renderer::Renderer()
    : m_width(800)
    , m_height(600)
    , m_vao(0)
    , m_vbo(0)
    , m_ebo(0)
{
}

Renderer::~Renderer()
{
    Shutdown();
}

bool Renderer::Initialize(int width, int height)
{
    m_width = width;
    m_height = height;
    
    // Initialize basic OpenGL state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Set up buffers
    SetupBuffers();
    
    // Create default resources
    CreateDefaultResources();
    
    return true;
}

void Renderer::Shutdown()
{
    // Clear resources
    m_shaders.clear();
    m_textures.clear();
    m_defaultShader = nullptr;
    m_textShader = nullptr;
    m_fontTexture = nullptr;
    
    // Delete OpenGL objects
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    
    if (m_ebo != 0) {
        glDeleteBuffers(1, &m_ebo);
        m_ebo = 0;
    }
}

void Renderer::Resize(int width, int height)
{
    m_width = width;
    m_height = height;
    glViewport(0, 0, width, height);
}

void Renderer::BeginFrame()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndFrame()
{
    // Nothing special to do here - the application will swap buffers
}

std::shared_ptr<Shader> Renderer::CreateShader(const std::string& name, 
                                             const std::string& vertexSource, 
                                             const std::string& fragmentSource)
{
    auto shader = std::make_shared<Shader>();
    if (!shader->Compile(vertexSource, fragmentSource)) {
        std::cerr << "Failed to compile shader: " << name << std::endl;
        return nullptr;
    }
    
    m_shaders[name] = shader;
    return shader;
}

std::shared_ptr<Shader> Renderer::GetShader(const std::string& name)
{
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second;
    }
    
    return nullptr;
}

std::shared_ptr<Texture> Renderer::CreateTexture(const std::string& name,
                                              int width, int height, 
                                              const unsigned char* data, 
                                              int channels)
{
    auto texture = std::make_shared<Texture>();
    if (!texture->Create(width, height, data, channels)) {
        std::cerr << "Failed to create texture: " << name << std::endl;
        return nullptr;
    }
    
    m_textures[name] = texture;
    return texture;
}

std::shared_ptr<Texture> Renderer::GetTexture(const std::string& name)
{
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second;
    }
    
    return nullptr;
}

void Renderer::DrawQuad(float x, float y, float width, float height, float r, float g, float b, float a)
{
    if (!m_defaultShader) {
        return;
    }
    
    // Set up vertices
    Vertex vertices[4] = {
        Vertex(x, y, 0, r, g, b, a, 0, 0),
        Vertex(x + width, y, 0, r, g, b, a, 1, 0),
        Vertex(x + width, y + height, 0, r, g, b, a, 1, 1),
        Vertex(x, y + height, 0, r, g, b, a, 0, 1)
    };
    
    // Set up indices
    unsigned int indices[6] = {
        0, 1, 2,
        2, 3, 0
    };
    
    // Bind shader and set uniforms
    m_defaultShader->Use();
    m_defaultShader->SetBool("useTexture", false);
    
    // Update VBO and EBO
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);
    
    // Draw
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::DrawTexturedQuad(float x, float y, float width, float height, 
                             const std::shared_ptr<Texture>& texture)
{
    if (!m_defaultShader || !texture) {
        return;
    }
    
    // Set up vertices
    Vertex vertices[4] = {
        Vertex(x, y, 0, 1, 1, 1, 1, 0, 0),
        Vertex(x + width, y, 0, 1, 1, 1, 1, 1, 0),
        Vertex(x + width, y + height, 0, 1, 1, 1, 1, 1, 1),
        Vertex(x, y + height, 0, 1, 1, 1, 1, 0, 1)
    };
    
    // Set up indices
    unsigned int indices[6] = {
        0, 1, 2,
        2, 3, 0
    };
    
    // Bind shader and set uniforms
    m_defaultShader->Use();
    m_defaultShader->SetBool("useTexture", true);
    
    // Bind texture
    texture->Bind(0);
    m_defaultShader->SetInt("texture1", 0);
    
    // Update VBO and EBO
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);
    
    // Draw
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::DrawCardSprite(const CardSprite& cardSprite)
{
    // Get card position and size
    float x = cardSprite.GetX();
    float y = cardSprite.GetY();
    float width = cardSprite.GetWidth();
    float height = cardSprite.GetHeight();
    
    // Get card texture
    std::shared_ptr<Texture> texture = cardSprite.GetTexture();
    
    // Draw the card
    DrawTexturedQuad(x, y, width, height, texture);
}

void Renderer::DrawText(const std::string& text, float x, float y, float scale, float r, float g, float b)
{
    // This is a placeholder for text rendering
    // In a real implementation, we would need a font texture atlas and glyph metrics
    // For simplicity, we'll draw colored rectangles
    
    float charWidth = 10.0f * scale;
    float charHeight = 20.0f * scale;
    
    for (size_t i = 0; i < text.length(); i++) {
        DrawQuad(x + i * charWidth, y, charWidth - 1, charHeight, r, g, b, 1.0f);
    }
}

void Renderer::Begin2D()
{
    // Set up 2D orthographic projection
    if (m_defaultShader) {
        m_defaultShader->Use();
        
        // Set up projection matrix (orthographic)
        float projection[16] = {
            2.0f / m_width, 0.0f, 0.0f, 0.0f,
            0.0f, -2.0f / m_height, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f, 1.0f
        };
        
        m_defaultShader->SetMatrix4("projection", projection);
        
        // Set up model matrix (identity)
        float model[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        
        m_defaultShader->SetMatrix4("model", model);
    }
}

void Renderer::End2D()
{
    // Nothing special to do here
}

void Renderer::SetupBuffers()
{
    // Create VAO
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    
    // Create VBO
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, nullptr, GL_DYNAMIC_DRAW);
    
    // Create EBO
    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, nullptr, GL_DYNAMIC_DRAW);
    
    // Set up vertex attributes
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    // Unbind
    glBindVertexArray(0);
}

void Renderer::CreateDefaultResources()
{
    CreateDefaultShaders();
    CreateFontTexture();
}

void Renderer::CreateDefaultShaders()
{
    // Create default shader
    m_defaultShader = CreateShader("default", DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER);
    
    // Create text shader
    m_textShader = CreateShader("text", TEXT_VERTEX_SHADER, TEXT_FRAGMENT_SHADER);
}

void Renderer::CreateFontTexture()
{
    // Create a simple white texture for now
    // In a real implementation, we would load a proper font texture atlas
    const int width = 128;
    const int height = 128;
    unsigned char* data = new unsigned char[width * height];
    
    // Fill with white
    for (int i = 0; i < width * height; i++) {
        data[i] = 255;
    }
    
    m_fontTexture = CreateTexture("font", width, height, data, 1);
    
    delete[] data;
}

} // namespace Graphics
} // namespace CardGameLib
