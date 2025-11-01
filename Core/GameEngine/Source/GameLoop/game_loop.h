/**
 * Game Loop - Main game execution loop
 * 
 * Integrates all subsystems:
 * - Input processing
 * - Game logic update
 * - Physics simulation
 * - Rendering
 * - Frame timing
 * 
 * Architecture:
 * - Update Phase: All game state is modified here
 * - Render Phase: All state is read-only during rendering
 * - Frame Timing: Maintains target FPS (usually 60)
 * 
 * Threading: All operations happen on main thread for safety
 */

#pragma once

#include <cstdint>

namespace GeneralsX {

class GameWorld;
class Camera;
class InputManager;
class GraphicsBackend;

/**
 * Frame timing information
 */
struct FrameTiming {
    double total_time;          // Total elapsed time since game start (seconds)
    double frame_time;          // Total time for current frame (seconds)
    double delta_time;          // Time since last frame (seconds)
    double render_time;         // Time spent rendering (seconds)
    double update_time;         // Time spent updating game state (seconds)
    uint32_t frame_number;      // Current frame number
    double target_fps;          // Target frames per second (usually 60)
};

/**
 * Game loop manager
 * 
 * Main execution loop that coordinates all game systems:
 * 1. Process Input
 * 2. Update Game State
 * 3. Render Scene
 * 4. Maintain Frame Rate
 */
class GameLoop {
public:
    /**
     * Constructor
     * @param world GameWorld instance (not owned)
     * @param camera Active camera (not owned)
     * @param input Input manager (not owned)
     * @param graphics Graphics backend (not owned)
     */
    GameLoop(GameWorld* world, Camera* camera, InputManager* input, GraphicsBackend* graphics);
    
    /**
     * Destructor
     */
    ~GameLoop();
    
    /**
     * Set target frames per second
     * @param fps Target FPS (default: 60)
     */
    void SetTargetFPS(double fps);
    
    /**
     * Get current frame timing info
     * @return Frame timing struct
     */
    const FrameTiming& GetFrameTiming() const { return m_timing; }
    
    /**
     * Execute one frame of the game loop
     * 
     * Sequence:
     * 1. Update frame timing
     * 2. Process input
     * 3. Update game state
     * 4. Render scene
     * 5. Maintain frame rate
     * 
     * @return True if game should continue, false if quit requested
     */
    bool Update();
    
    /**
     * Request game quit
     * Sets the quit flag, loop will return false on next Update()
     */
    void RequestQuit();
    
    /**
     * Check if quit is requested
     * @return True if quit was requested
     */
    bool IsQuitRequested() const { return m_quit_requested; }

private:
    // Systems (not owned)
    GameWorld* m_world;
    Camera* m_camera;
    InputManager* m_input;
    GraphicsBackend* m_graphics;
    
    // Frame timing
    FrameTiming m_timing;
    double m_target_frame_time;
    
    // State
    bool m_quit_requested;
    
    /**
     * Update frame timing
     * Calculates delta_time and maintains target FPS
     */
    void UpdateFrameTiming();
    
    /**
     * Process all input
     * Keyboard, mouse, gamepad, etc
     */
    void ProcessInput();
    
    /**
     * Update game state
     * Called once per frame
     * 
     * @param delta_time Time since last frame
     */
    void UpdateGameState(float delta_time);
    
    /**
     * Render the scene
     * Called once per frame
     */
    void RenderScene();
    
    /**
     * Maintain target frame rate
     * Sleeps if frame finished early
     */
    void MaintainFrameRate();
};

} // namespace GeneralsX
