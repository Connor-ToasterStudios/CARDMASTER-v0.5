#pragma once

#include <memory>
#include <functional>
#include <vector>
#include "input/InputManager.h"
#include "graphics/CardSprite.h"

namespace CardGameLib {
namespace Input {

// Forward declarations
class DragDropTarget;

// Define callback types
using DragStartCallback = std::function<void(Graphics::CardSprite*)>;
using DragMoveCallback = std::function<void(Graphics::CardSprite*, int x, int y)>;
using DragEndCallback = std::function<void(Graphics::CardSprite*, DragDropTarget*, bool success)>;

// Interface for drag and drop targets
class DragDropTarget {
public:
    virtual ~DragDropTarget() = default;
    
    // Check if the target can accept the dragged card(s)
    virtual bool CanAcceptDrop(const Graphics::CardSprite* cardSprite) const = 0;
    
    // Handle the drop action
    virtual bool HandleDrop(Graphics::CardSprite* cardSprite) = 0;
    
    // Get the target's bounds
    virtual void GetBounds(int& x, int& y, int& width, int& height) const = 0;
    
    // Get a pointer to the target's identifier (e.g., a pile)
    virtual void* GetTargetId() const = 0;
};

class DragDropManager {
public:
    DragDropManager();
    ~DragDropManager();
    
    // Initialization with input manager
    void Initialize(InputManager* inputManager);
    
    // Update drag and drop state
    void Update();
    
    // Register a draggable card sprite
    void RegisterDraggable(Graphics::CardSprite* cardSprite);
    
    // Unregister a draggable card sprite
    void UnregisterDraggable(Graphics::CardSprite* cardSprite);
    
    // Register a drop target
    void RegisterDropTarget(DragDropTarget* target);
    
    // Unregister a drop target
    void UnregisterDropTarget(DragDropTarget* target);
    
    // Set callbacks
    void SetDragStartCallback(DragStartCallback callback);
    void SetDragMoveCallback(DragMoveCallback callback);
    void SetDragEndCallback(DragEndCallback callback);
    
    // Check if a drag operation is in progress
    bool IsDragging() const;
    
    // Get the currently dragged card sprite
    Graphics::CardSprite* GetDraggedCardSprite() const;
    
private:
    // Input manager reference
    InputManager* m_inputManager;
    
    // Draggable objects
    std::vector<Graphics::CardSprite*> m_draggables;
    
    // Drop targets
    std::vector<DragDropTarget*> m_dropTargets;
    
    // Drag state
    bool m_isDragging;
    Graphics::CardSprite* m_draggedCardSprite;
    int m_dragStartX;
    int m_dragStartY;
    int m_dragOffsetX;
    int m_dragOffsetY;
    
    // Callbacks
    DragStartCallback m_dragStartCallback;
    DragMoveCallback m_dragMoveCallback;
    DragEndCallback m_dragEndCallback;
    
    // Input event handlers
    void OnMouseEvent(const MouseEvent& event);
    
    // Find draggable at position
    Graphics::CardSprite* FindDraggableAtPosition(int x, int y);
    
    // Find drop target at position
    DragDropTarget* FindDropTargetAtPosition(int x, int y);
};

} // namespace Input
} // namespace CardGameLib
