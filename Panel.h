#pragma once

#include "ui/UI.h"
#include <memory>
#include "graphics/Texture.h"

namespace CardGameLib {
namespace UI {

class Panel : public UIElement {
public:
    Panel();
    virtual ~Panel();
    
    // UIElement interface
    virtual UIElementType GetType() const override { return UIElementType::PANEL; }
    virtual void Render(Graphics::Renderer* renderer) override;
    
    // Panel-specific methods
    void SetTexture(std::shared_ptr<Graphics::Texture> texture);
    std::shared_ptr<Graphics::Texture> GetTexture() const;
    
    void SetDraggable(bool draggable);
    bool IsDraggable() const;
    
    void SetResizable(bool resizable);
    bool IsResizable() const;
    
    void SetTitleBar(bool hasTitleBar);
    bool HasTitleBar() const;
    
    void SetTitle(const std::string& title);
    const std::string& GetTitle() const;
    
    void SetTitleBarHeight(float height);
    float GetTitleBarHeight() const;
    
    void SetTitleBarColor(float r, float g, float b, float a);
    void GetTitleBarColor(float& r, float& g, float& b, float& a) const;
    
protected:
    // Override mouse event handlers
    virtual void OnMouseDown(int x, int y, Input::MouseButton button) override;
    virtual void OnMouseUp(int x, int y, Input::MouseButton button) override;
    virtual void OnMouseMove(int x, int y) override;
    
private:
    std::shared_ptr<Graphics::Texture> m_texture;
    bool m_draggable;
    bool m_resizable;
    bool m_hasTitleBar;
    std::string m_title;
    float m_titleBarHeight;
    float m_titleBarColor[4];
    
    bool m_isDragging;
    bool m_isResizing;
    int m_dragStartX;
    int m_dragStartY;
    float m_dragStartPanelX;
    float m_dragStartPanelY;
    float m_dragStartPanelWidth;
    float m_dragStartPanelHeight;
    
    bool IsPointInTitleBar(float x, float y) const;
    bool IsPointInResizeArea(float x, float y) const;
};

} // namespace UI
} // namespace CardGameLib
