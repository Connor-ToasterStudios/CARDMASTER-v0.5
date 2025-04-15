#include "input/DragDropManager.h"

namespace CardGameLib {
namespace Input {

DragDropManager::DragDropManager()
    : m_inputManager(nullptr)
    , m_isDragging(false)
    , m_draggedCardSprite(nullptr)
    , m_dragStartX(0)
    , m_dragStartY(0)
    , m_dragOffsetX(0)
    , m_dragOffsetY(0)
{
}

DragDropManager::~DragDropManager()
{
}

void DragDropManager::Initialize(InputManager* inputManager)
{
    m_inputManager = inputManager;
    
    // Register for mouse events
    if (m_inputManager) {
        m_inputManager->RegisterMouseCallback([this](const MouseEvent& event) {
            OnMouseEvent(event);
        });
    }
}

void DragDropManager::Update()
{
    // Update drag position if dragging
    if (m_isDragging && m_draggedCardSprite) {
        int mouseX, mouseY;
        m_inputManager->GetMousePosition(mouseX, mouseY);
        
        // Update card position
        float newX = static_cast<float>(mouseX - m_dragOffsetX);
        float newY = static_cast<float>(mouseY - m_dragOffsetY);
        m_draggedCardSprite->SetPosition(newX, newY);
        
        // Call the drag move callback if provided
        if (m_dragMoveCallback) {
            m_dragMoveCallback(m_draggedCardSprite, mouseX, mouseY);
        }
    }
}

void DragDropManager::RegisterDraggable(Graphics::CardSprite* cardSprite)
{
    if (cardSprite) {
        m_draggables.push_back(cardSprite);
    }
}

void DragDropManager::UnregisterDraggable(Graphics::CardSprite* cardSprite)
{
    auto it = std::find(m_draggables.begin(), m_draggables.end(), cardSprite);
    if (it != m_draggables.end()) {
        m_draggables.erase(it);
    }
}

void DragDropManager::RegisterDropTarget(DragDropTarget* target)
{
    if (target) {
        m_dropTargets.push_back(target);
    }
}

void DragDropManager::UnregisterDropTarget(DragDropTarget* target)
{
    auto it = std::find(m_dropTargets.begin(), m_dropTargets.end(), target);
    if (it != m_dropTargets.end()) {
        m_dropTargets.erase(it);
    }
}

void DragDropManager::SetDragStartCallback(DragStartCallback callback)
{
    m_dragStartCallback = callback;
}

void DragDropManager::SetDragMoveCallback(DragMoveCallback callback)
{
    m_dragMoveCallback = callback;
}

void DragDropManager::SetDragEndCallback(DragEndCallback callback)
{
    m_dragEndCallback = callback;
}

bool DragDropManager::IsDragging() const
{
    return m_isDragging;
}

Graphics::CardSprite* DragDropManager::GetDraggedCardSprite() const
{
    return m_draggedCardSprite;
}

void DragDropManager::OnMouseEvent(const MouseEvent& event)
{
    if (event.type == MouseEventType::PRESS && event.button == MouseButton::LEFT) {
        // Start drag
        if (!m_isDragging) {
            m_draggedCardSprite = FindDraggableAtPosition(event.x, event.y);
            
            if (m_draggedCardSprite) {
                m_isDragging = true;
                m_dragStartX = event.x;
                m_dragStartY = event.y;
                
                // Calculate offset from the top-left corner of the card
                m_dragOffsetX = static_cast<int>(event.x - m_draggedCardSprite->GetX());
                m_dragOffsetY = static_cast<int>(event.y - m_draggedCardSprite->GetY());
                
                // Set the dragging flag on the card sprite
                m_draggedCardSprite->SetDragging(true);
                
                // Call the drag start callback if provided
                if (m_dragStartCallback) {
                    m_dragStartCallback(m_draggedCardSprite);
                }
            }
        }
    }
    else if (event.type == MouseEventType::RELEASE && event.button == MouseButton::LEFT) {
        // End drag
        if (m_isDragging && m_draggedCardSprite) {
            // Find drop target at release position
            DragDropTarget* target = FindDropTargetAtPosition(event.x, event.y);
            bool success = false;
            
            if (target && target->CanAcceptDrop(m_draggedCardSprite)) {
                // Handle the drop
                success = target->HandleDrop(m_draggedCardSprite);
            }
            
            // Call the drag end callback if provided
            if (m_dragEndCallback) {
                m_dragEndCallback(m_draggedCardSprite, target, success);
            }
            
            // Reset dragging state
            m_draggedCardSprite->SetDragging(false);
            m_draggedCardSprite = nullptr;
            m_isDragging = false;
        }
    }
}

Graphics::CardSprite* DragDropManager::FindDraggableAtPosition(int x, int y)
{
    // Check draggable card sprites in reverse order (top to bottom)
    for (auto it = m_draggables.rbegin(); it != m_draggables.rend(); ++it) {
        Graphics::CardSprite* cardSprite = *it;
        
        // Skip invisible cards
        if (!cardSprite->IsVisible()) {
            continue;
        }
        
        // Check if the point is within the card bounds
        if (cardSprite->ContainsPoint(static_cast<float>(x), static_cast<float>(y))) {
            return cardSprite;
        }
    }
    
    return nullptr;
}

DragDropTarget* DragDropManager::FindDropTargetAtPosition(int x, int y)
{
    for (auto target : m_dropTargets) {
        int targetX, targetY, targetWidth, targetHeight;
        target->GetBounds(targetX, targetY, targetWidth, targetHeight);
        
        // Check if the point is within the target bounds
        if (x >= targetX && x < targetX + targetWidth && y >= targetY && y < targetY + targetHeight) {
            return target;
        }
    }
    
    return nullptr;
}

} // namespace Input
} // namespace CardGameLib
