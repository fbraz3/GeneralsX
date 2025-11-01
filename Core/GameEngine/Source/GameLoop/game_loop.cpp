/**
 * Game loop implementation
 * 
 * Main game execution loop coordinating update and render
 */

#include "game_loop.h"
#include "../GameWorld/gameworld.h"
#include "../Camera/camera.h"
#include <chrono>
#include <thread>

namespace GeneralsX {

// Forward declarations for systems not included here
// In real implementation, these would be full inclusions
class InputManager;
class GraphicsBackend;

GameLoop::GameLoop(GameWorld* world, Camera* camera, InputManager* input, GraphicsBackend* graphics)
    : m_world(world)
    , m_camera(camera)
    , m_input(input)
    , m_graphics(graphics)
    , m_target_frame_time(1.0 / 60.0)  // 60 FPS default
    , m_quit_requested(false)
{
    m_timing.total_time = 0.0;
    m_timing.frame_time = 0.0;
    m_timing.delta_time = 0.0;
    m_timing.render_time = 0.0;
    m_timing.update_time = 0.0;
    m_timing.frame_number = 0;
    m_timing.target_fps = 60.0;
}

GameLoop::~GameLoop() {
}

void GameLoop::SetTargetFPS(double fps) {
    if (fps > 0.0) {
        m_timing.target_fps = fps;
        m_target_frame_time = 1.0 / fps;
    }
}

bool GameLoop::Update() {
    if (m_quit_requested) {
        return false;
    }
    
    // Update frame timing
    UpdateFrameTiming();
    
    // Process input events
    ProcessInput();
    
    // Update game state
    UpdateGameState(static_cast<float>(m_timing.delta_time));
    
    // Render scene
    RenderScene();
    
    // Maintain target frame rate
    MaintainFrameRate();
    
    // Increment frame counter
    m_timing.frame_number++;
    
    return !m_quit_requested;
}

void GameLoop::RequestQuit() {
    m_quit_requested = true;
}

// ----- Private Methods -----

static auto s_frame_start_time = std::chrono::high_resolution_clock::now();
static auto s_last_frame_time = std::chrono::high_resolution_clock::now();

void GameLoop::UpdateFrameTiming() {
    auto current_time = std::chrono::high_resolution_clock::now();
    
    if (m_timing.frame_number == 0) {
        // First frame
        m_timing.delta_time = m_target_frame_time;
        s_frame_start_time = current_time;
    } else {
        // Subsequent frames
        std::chrono::duration<double> elapsed = current_time - s_last_frame_time;
        m_timing.delta_time = elapsed.count();
    }
    
    std::chrono::duration<double> total_elapsed = current_time - s_frame_start_time;
    m_timing.total_time = total_elapsed.count();
    
    s_last_frame_time = current_time;
}

void GameLoop::ProcessInput() {
    // TODO: Call input system to process keyboard/mouse events
    // This would typically be:
    // if (m_input != nullptr) {
    //     m_input->ProcessInput();
    // }
}

void GameLoop::UpdateGameState(float delta_time) {
    if (m_world == nullptr) {
        return;
    }
    
    auto update_start = std::chrono::high_resolution_clock::now();
    
    // Update camera
    if (m_camera != nullptr) {
        // TODO: Call camera update if needed
        // m_camera->Update(delta_time);
    }
    
    // Update game world
    m_world->Update(delta_time);
    
    // Cleanup (remove dead objects, expired effects, etc)
    m_world->Cleanup();
    
    auto update_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> update_duration = update_end - update_start;
    m_timing.update_time = update_duration.count();
}

void GameLoop::RenderScene() {
    if (m_graphics == nullptr || m_world == nullptr) {
        return;
    }
    
    auto render_start = std::chrono::high_resolution_clock::now();
    
    // TODO: Call graphics backend BeginFrame
    // m_graphics->BeginFrame();
    
    // Render game world
    m_world->Render();
    
    // TODO: Render UI
    // TODO: Call graphics backend EndFrame/Present
    // m_graphics->EndFrame();
    // m_graphics->Present();
    
    auto render_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> render_duration = render_end - render_start;
    m_timing.render_time = render_duration.count();
}

void GameLoop::MaintainFrameRate() {
    // Calculate elapsed time in this frame
    auto frame_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> frame_duration = frame_end - s_frame_start_time;
    m_timing.frame_time = frame_duration.count();
    
    // Sleep if frame finished early
    double sleep_time = m_target_frame_time - m_timing.frame_time;
    if (sleep_time > 0.0) {
        std::chrono::duration<double> sleep_duration(sleep_time);
        std::this_thread::sleep_for(sleep_duration);
    }
}

} // namespace GeneralsX
