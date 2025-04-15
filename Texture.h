#pragma once

namespace CardGameLib {
namespace Graphics {

class Texture {
public:
    Texture();
    ~Texture();
    
    // Create a texture from pixel data
    bool Create(int width, int height, const unsigned char* data, int channels);
    
    // Create an empty texture
    bool CreateEmpty(int width, int height, int channels);
    
    // Update an existing texture
    bool Update(const unsigned char* data);
    
    // Bind the texture to a texture unit
    void Bind(unsigned int unit = 0) const;
    
    // Free resources
    void Delete();
    
    // Getters
    unsigned int GetId() const { return m_id; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetChannels() const { return m_channels; }
    
private:
    unsigned int m_id;
    int m_width;
    int m_height;
    int m_channels;
    
    // Determine the OpenGL format from the number of channels
    unsigned int GetFormat() const;
    unsigned int GetInternalFormat() const;
};

} // namespace Graphics
} // namespace CardGameLib
