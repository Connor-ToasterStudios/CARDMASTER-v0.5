#include "core/Game.h"
#include "platform/Platform.h"
#include "graphics/Renderer.h"
#include "ui/UI.h"
#include "input/InputManager.h"
#include "input/DragDropManager.h"
#include "games/blackjack/Blackjack.h"

#include <iostream>
#include <memory>

using namespace CardGameLib;

int main(int argc, char** argv)
{
    std::cout << "CardGameLib - Card Game Framework" << std::endl;
    
    // Initialize platform system
    auto platform = std::unique_ptr<Platform::PlatformSystem>(Platform::CreatePlatformSystem());
    if (!platform) {
        std::cerr << "Failed to create platform system" << std::endl;
        return 1;
    }
    
    // Create window
    if (!platform->CreateWindow(800, 600, "CardGameLib Demo")) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }
    
    // Create renderer
    auto renderer = std::make_unique<Graphics::Renderer>();
    renderer->Initialize(800, 600);
    
    // Create UI manager
    auto uiManager = std::make_unique<UI::UIManager>();
    
    // Create input manager
    auto inputManager = std::make_unique<Input::InputManager>();
    inputManager->Initialize();
    
    // Create drag-drop manager
    auto dragDropManager = std::make_unique<Input::DragDropManager>();
    dragDropManager->Initialize(inputManager.get());
    
    // Initialize UI manager with renderer and input manager
    uiManager->Initialize(renderer.get(), inputManager.get());
    
    // Show window
    platform->ShowWindow();
    
    std::cout << "Demo ready. Press any key to exit." << std::endl;
    
    // Create and initialize the Blackjack game
    auto blackjackGame = std::make_unique<Games::Blackjack::BlackjackGame>();
    blackjackGame->Initialize();
    
    std::cout << "Blackjack game initialized." << std::endl;
    
    // Main loop with game logic
    bool running = true;
    
    platform->SetWindowEventCallback([&running](Platform::WindowEventType type, int param1, int param2) {
        if (type == Platform::WindowEventType::CLOSE) {
            running = false;
        }
    });
    
    // Handle user input for the Blackjack game
    inputManager->SetMouseButtonCallback([&blackjackGame](int x, int y, bool isDown) {
        blackjackGame->HandleInput(x, y, isDown);
    });
    
    float deltaTime = 0.016f; // ~60 FPS
    
    // Main game loop
    while (running) {
        // Process events
        platform->PollEvents();
        
        // Update input manager
        inputManager->Update();
        
        // Update drag-drop manager
        dragDropManager->Update();
        
        // Update game logic
        blackjackGame->Update(deltaTime);
        
        // Handle UI input
        uiManager->HandleInput();
        
        // Render frame
        renderer->BeginFrame();
        
        // Render game
        blackjackGame->Render();
        
        // Render UI
        uiManager->Render();
        
        // Finish rendering
        renderer->EndFrame();
        
        // Swap buffers
        platform->SwapBuffers();
        
        // Exit the game after a certain number of frames for this demo
        static int counter = 0;
        if (++counter > 500) { // Run for longer to see more of the game
            running = false;
        }
        
        // Artificial delay to prevent CPU overuse in this simple demo
        platform->Sleep(100);
    }
    
    std::cout << "Demo complete. Exiting." << std::endl;
    
    return 0;
}