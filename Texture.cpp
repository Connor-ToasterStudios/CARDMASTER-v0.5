#include "graphics/Texture.h"
#include <GL/glew.h>  // GLEW must come before other GL includes
#include <GL/gl.h>
#include <iostream>

namespace CardGameLib {
namespace Graphics {

Texture::Texture()
    : m_id(0)
    , m_width(0)
    , m_height(0)
    , m_channels(0)
{
}

Texture::~Texture()
{
    Delete();
}

bool Texture::Create(int width, int height, const unsigned char* data, int channels)
{
    if (m_id != 0) {
        Delete();
    }
    
    m_width = width;
    m_height = height;
    m_channels = channels;
    
    // Create OpenGL texture
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Upload the texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GetInternalFormat(), width, height, 0, GetFormat(), GL_UNSIGNED_BYTE, data);
    
    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);
    
    return true;
}

bool Texture::CreateEmpty(int width, int height, int channels)
{
    return Create(width, height, nullptr, channels);
}

bool Texture::Update(const unsigned char* data)
{
    if (m_id == 0) {
        return false;
    }
    
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GetFormat(), GL_UNSIGNED_BYTE, data);
    
    return true;
}

void Texture::Bind(unsigned int unit) const
{
    if (m_id == 0) {
        return;
    }
    
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture::Delete()
{
    if (m_id != 0) {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
}

unsigned int Texture::GetFormat() const
{
    switch (m_channels) {
        case 1: return GL_RED;
        case 2: return GL_RG;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default: return GL_RGB;
    }
}

unsigned int Texture::GetInternalFormat() const
{
    switch (m_channels) {
        case 1: return GL_RED;
        case 2: return GL_RG8;
        case 3: return GL_RGB8;
        case 4: return GL_RGBA8;
        default: return GL_RGB8;
    }
}

} // namespace Graphics
} // namespace CardGameLib
