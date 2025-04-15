#pragma once

#include "ui/UI.h"
#include "ui/Panel.h"
#include "ui/Button.h"
#include "ui/Label.h"
#include "core/Game.h"
#include "graphics/CardSprite.h"
#include "input/DragDropManager.h"
#include <memory>
#include <vector>
#include <map>

namespace CardGameLib {
namespace UI {

// Base class for game-specific UIs
class GameUI {
public:
    GameUI(UIManager* uiManager, std::shared_ptr<Core::Game> game);
    virtual ~GameUI();
    
    // Initialize the UI
    virtual void Initialize();
    
    // Update the UI
    virtual void Update(float deltaTime);
    
    // Show/hide the UI
    virtual void Show();
    virtual void Hide();
    
    // Handle game moves
    virtual bool HandleMove(const std::string& moveData);
    
    // Get the game
    std::shared_ptr<Core::Game> GetGame() const;
    
protected:
    UIManager* m_uiManager;
    std::shared_ptr<Core::Game> m_game;
    bool m_visible;
    
    // Root panel for all UI elements
    std::shared_ptr<Panel> m_rootPanel;
    
    // Common UI elements
    std::shared_ptr<Button> m_backButton;
    std::shared_ptr<Label> m_statusLabel;
    
    // Card management
    std::vector<std::shared_ptr<Graphics::CardSprite>> m_cardSprites;
    std::map<Graphics::CardSprite*, std::string> m_cardLocations;
    
    // Drag and drop support
    Input::DragDropManager m_dragDropManager;
    
    // Card drag handlers
    virtual void OnCardDragStart(Graphics::CardSprite* cardSprite);
    virtual void OnCardDragMove(Graphics::CardSprite* cardSprite, int x, int y);
    virtual void OnCardDragEnd(Graphics::CardSprite* cardSprite, Input::DragDropTarget* target, bool success);
    
    // Card creation and management
    virtual std::shared_ptr<Graphics::CardSprite> CreateCardSprite(const Core::Card& card);
    virtual void UpdateCardPositions();
    
    // UI event handlers
    virtual void OnBackButtonClicked(const UIEvent& event);
};

// Klondike Solitaire specific UI
class KlondikeUI : public GameUI {
public:
    KlondikeUI(UIManager* uiManager, std::shared_ptr<Core::Game> game);
    virtual ~KlondikeUI();
    
    // Override base class methods
    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void UpdateCardPositions() override;
    
private:
    // UI elements
    std::shared_ptr<Button> m_stockButton;
    
    // Card layout
    float m_cardWidth;
    float m_cardHeight;
    float m_cardSpacing;
    
    // Card stacks
    std::vector<std::shared_ptr<Panel>> m_foundationPanels;
    std::vector<std::shared_ptr<Panel>> m_tableauPanels;
    std::shared_ptr<Panel> m_wastePanel;
    
    // UI event handlers
    void OnStockButtonClicked(const UIEvent& event);
    
    // Implement drag drop targets
    class KlondikeDropTarget : public Input::DragDropTarget {
    public:
        KlondikeDropTarget(KlondikeUI* ui, const std::string& location, int index);
        
        // DragDropTarget interface
        virtual bool CanAcceptDrop(const Graphics::CardSprite* cardSprite) const override;
        virtual bool HandleDrop(Graphics::CardSprite* cardSprite) override;
        virtual void GetBounds(int& x, int& y, int& width, int& height) const override;
        virtual void* GetTargetId() const override;
        
    private:
        KlondikeUI* m_ui;
        std::string m_location;
        int m_index;
        int m_x, m_y, m_width, m_height;
    };
    
    // Drop targets
    std::vector<std::shared_ptr<KlondikeDropTarget>> m_dropTargets;
};

// FreeCell Solitaire specific UI
class FreeCellUI : public GameUI {
public:
    FreeCellUI(UIManager* uiManager, std::shared_ptr<Core::Game> game);
    virtual ~FreeCellUI();
    
    // Override base class methods
    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void UpdateCardPositions() override;
    
private:
    // Card layout
    float m_cardWidth;
    float m_cardHeight;
    float m_cardSpacing;
    
    // Card stacks
    std::vector<std::shared_ptr<Panel>> m_freeCellPanels;
    std::vector<std::shared_ptr<Panel>> m_foundationPanels;
    std::vector<std::shared_ptr<Panel>> m_tableauPanels;
    
    // Implement drag drop targets
    class FreeCellDropTarget : public Input::DragDropTarget {
    public:
        FreeCellDropTarget(FreeCellUI* ui, const std::string& location, int index);
        
        // DragDropTarget interface
        virtual bool CanAcceptDrop(const Graphics::CardSprite* cardSprite) const override;
        virtual bool HandleDrop(Graphics::CardSprite* cardSprite) override;
        virtual void GetBounds(int& x, int& y, int& width, int& height) const override;
        virtual void* GetTargetId() const override;
        
    private:
        FreeCellUI* m_ui;
        std::string m_location;
        int m_index;
        int m_x, m_y, m_width, m_height;
    };
    
    // Drop targets
    std::vector<std::shared_ptr<FreeCellDropTarget>> m_dropTargets;
};

// Spider Solitaire specific UI
class SpiderUI : public GameUI {
public:
    SpiderUI(UIManager* uiManager, std::shared_ptr<Core::Game> game);
    virtual ~SpiderUI();
    
    // Override base class methods
    virtual void Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void UpdateCardPositions() override;
    
private:
    // UI elements
    std::shared_ptr<Button> m_dealButton;
    
    // Card layout
    float m_cardWidth;
    float m_cardHeight;
    float m_cardSpacing;
    
    // Card stacks
    std::vector<std::shared_ptr<Panel>> m_tableauPanels;
    
    // UI event handlers
    void OnDealButtonClicked(const UIEvent& event);
    
    // Implement drag drop targets
    class SpiderDropTarget : public Input::DragDropTarget {
    public:
        SpiderDropTarget(SpiderUI* ui, const std::string& location, int index);
        
        // DragDropTarget interface
        virtual bool CanAcceptDrop(const Graphics::CardSprite* cardSprite) const override;
        virtual bool HandleDrop(Graphics::CardSprite* cardSprite) override;
        virtual void GetBounds(int& x, int& y, int& width, int& height) const override;
        virtual void* GetTargetId() const override;
        
    private:
        SpiderUI* m_ui;
        std::string m_location;
        int m_index;
        int m_x, m_y, m_width, m_height;
    };
    
    // Drop targets
    std::vector<std::shared_ptr<SpiderDropTarget>> m_dropTargets;
};

} // namespace UI
} // namespace CardGameLib
