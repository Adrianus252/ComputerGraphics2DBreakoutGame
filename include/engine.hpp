#pragma once
#include <algorithm>
#include <array>
#include <filesystem>
#include <string>
#include <glbinding/gl46core/gl.h>
using namespace gl46core;
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_events.h>
#include <glm/glm.hpp>
#include <fmt/base.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include "time.hpp"
#include "window.hpp"
#include "input.hpp"
#include "pipeline.hpp"
#include "entities/camera.hpp"
#include "entities/mesh.hpp"
#include "entities/material.hpp"
#include "entities/texture.hpp"
#include "game/types.hpp"
#include "game/levels.hpp"
#include "game/breakout_state.hpp"
#include "game/game_state.hpp"
#include "game/config.hpp"
#include "ui/menu.hpp"
#include "game/settings.hpp"

struct Engine {

    // initializes all subsystems, loads resources, sets up the initial game state 
    void init() {
        Time::init();
        _window.init(1280, 720, "Breakout Computer Graphics", 4);
        //_camera.set_perspective(1280, 720, 70);

        _screen = {1280.0f, 720.0f};

        // initialize 2D pipeline
        _pipeline.init("../assets/shaders/flat2d.vert", "../assets/shaders/flat2d.frag");

        // set up shared 2D quad mesh (from mesh.hpp)
        _quad_mesh.init(Mesh::eQuad2D);
        _heart_mesh.init(Mesh::eHeart);
        _rect_material._mode = Material::e2D;
        _rect_material._base_color = glm::vec4(1.0f);
        // load textures
        _paddle_texture.init("../assets/textures/49-Breakout-Tiles.png");
        _paddle_wide_texture.init("../assets/textures/47-Breakout-Tiles.png");
        _paddle_piercing_texture.init("../assets/textures/44-Breakout-Tiles.png");
        _paddle_slow_texture.init("../assets/textures/41-Breakout-Tiles.png");
        _ball_texture.init("../assets/textures/58-Breakout-Tiles.png");
        _powerup_widen_texture.init("../assets/textures/21-Breakout-Tiles.png");
        _powerup_piercing_texture.init("../assets/textures/24-Breakout-Tiles.png");
        _powerup_multiball_texture.init("../assets/textures/26-Breakout-Tiles.png");
        _powerup_slowball_texture.init("../assets/textures/25-Breakout-Tiles.png");
        _heart_texture.init("../assets/textures/60-Breakout-Tiles.png");

        // load brick textures into array        
        const std::array<const char*, 9> brick_texture_files = {
            "../assets/textures/01-Breakout-Tiles.png", // 1 = blue
            "../assets/textures/03-Breakout-Tiles.png", // 2 = light green
            "../assets/textures/05-Breakout-Tiles.png", // 3 = purple
            "../assets/textures/07-Breakout-Tiles.png", // 4 = red
            "../assets/textures/09-Breakout-Tiles.png", // 5 = orange
            "../assets/textures/11-Breakout-Tiles.png", // 6 = light blue
            "../assets/textures/13-Breakout-Tiles.png", // 7 = yellow
            "../assets/textures/15-Breakout-Tiles.png", // 8 = green
            "../assets/textures/19-Breakout-Tiles.png"  // 9 = brown
        };
        for (size_t i = 0; i < brick_texture_files.size(); i++) {
            _brick_textures[i].init(brick_texture_files[i]);
        }

        init_audio();

        // initialize breakout state
        _game.init(_screen);
        _previous_lives = _game.lives;

        // load settings and apply to game state
        _settings.load();
        _game.use_textures = _settings.use_textures;
        _game.use_gradient_brick_colors = _settings.use_gradient_brick_colors;
        _game.paddle_speed_percent = _settings.paddle_speed_percent;
        _show_information_window = _settings.show_information_window;
        _sound_enabled = _settings.sound_enabled;

        // start in menu state
        _state = GameState::Menu;

        // initialize ImGui for UI rendering
        ImGui::CreateContext();

        // init style and tweak a few settings
        ImGui::StyleColorsDark();        
        ImGuiStyle& style = ImGui::GetStyle(); 
        style.FrameRounding = 5.0f;
        style.WindowRounding = 5.0f;
        style.FramePadding = ImVec2(10, 5);
        style.ItemSpacing = ImVec2(10, 8);
        style.WindowPadding = ImVec2(15, 15);
        
        ImGui_ImplSDL3_InitForOpenGL(_window._window_p, _window._context);
        ImGui_ImplOpenGL3_Init();
        
        // scan for custom levels
        scan_custom_levels();
    }
    
    // scans the custom levels folder and stores names and pathes
    void scan_custom_levels() {
        _custom_level_files.clear();
        _custom_level_paths.clear();
        
        std::filesystem::path levels_dir = "../levels";
        
        // create levels directory if it doesn't exist
        if (!std::filesystem::exists(levels_dir)) {
            std::filesystem::create_directories(levels_dir);
            fmt::println("Created custom levels directory: {}", levels_dir.string());
        }
        
        // scan for .txt files
        for (const auto& entry : std::filesystem::directory_iterator(levels_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                _custom_level_files.push_back(entry.path().stem().string()); // filename without extension
                _custom_level_paths.push_back(entry.path().string()); // full path
            }
        }
        
        fmt::println("Found {} custom level(s)", _custom_level_files.size());
    }
    
    
    // clean up all resources, and prepare for application exit
    void destroy() {
        // destroy audio stuff
        if (audio_stream != nullptr) {
            SDL_DestroyAudioStream(audio_stream);
        }
        if (audio_file.buffer != nullptr) {
            SDL_free(audio_file.buffer);
        }
        if (audio_file_loss.buffer != nullptr) {
            SDL_free(audio_file_loss.buffer);
        }
        if (audio_file_win.buffer != nullptr) {
            SDL_free(audio_file_win.buffer);
        }
        if (audio_file_powerup.buffer != nullptr) {
            SDL_free(audio_file_powerup.buffer);
        }

        // free resources 
        // paddle 
        _paddle_texture.destroy();
        _paddle_wide_texture.destroy();
        _paddle_piercing_texture.destroy();
        _paddle_slow_texture.destroy();
        // ball 
        _ball_texture.destroy();
        // power-ups
        _powerup_widen_texture.destroy();
        _powerup_piercing_texture.destroy();
        _powerup_multiball_texture.destroy();
        _powerup_slowball_texture.destroy();
        // heart
        _heart_texture.destroy();
        // bricks
        for (auto& texture : _brick_textures) {
            texture.destroy();
        }
        // mesh 
        _quad_mesh.destroy();
        _heart_mesh.destroy();
        // pipeline and window 
        _pipeline.destroy();
        _window.destroy();

        // shut down imgui backend
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }
    

    // handle input events, pass events to ImGui, and update game state
    auto execute_event(SDL_Event* event_p) -> SDL_AppResult {
        // pass event over to imgui as well
        ImGui_ImplSDL3_ProcessEvent(event_p);
        // let input system process event
        Input::register_event(*event_p);
        switch (event_p->type) {
            case SDL_EventType::SDL_EVENT_QUIT: return SDL_AppResult::SDL_APP_SUCCESS;
            default: break;
        }
        return SDL_AppResult::SDL_APP_CONTINUE;
    }

    // execute frame: update game state, render scene, and present to screen
    void execute_frame() {
        // update timer for accurate Time::get_delta()
        Time::update();
        // start new imgui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // Information Menu (debugging) - only show during gameplay, not in menus
        if (_show_information_window && 
            _state != GameState::Menu && 
            _state != GameState::LevelSelect && 
            _state != GameState::CustomLevelSelect) {
            ImGui::Begin("Information");

            // display fps and frame time
            ImGui::Text("%.1f fps, %.2f ms", ImGui::GetIO().Framerate, Time::get_delta() * 1000.0); //fps and frame time
                        
            // display current level information
            if (_game.using_custom_level) {
                ImGui::Text("Custom: %s", _game.custom_level_name.c_str());
            } else {
                ImGui::Text("Normal: %d - %s", 
                    static_cast<int>(_game.level_index) + 1, 
                    BreakoutLevels::names[_game.level_index]);
            }
                
            // display current lives
            ImGui::Text("Lives: %d", _game.lives); // lives
            
            // display active powerup and powerup timer
            if (_game.paddle_wide_timer > 0.0f) {
                ImGui::Text("Power: Widen - %.1f s", _game.paddle_wide_timer);
            } else if (_game.slow_ball_timer > 0.0f) {
                ImGui::Text("Power: Slow Ball - %.1f s", _game.slow_ball_timer);
            } else if (_game.piercing_timer > 0.0f) {
                ImGui::Text("Power: Pierce - %.1f s", _game.piercing_timer);
            } else {
                ImGui::Text("Power: None");
            }
            
            ImGui::End();
        }

        // draw the main menu if menu state
        // lambda callbacks for starting game, going to menu, next level, custom levels
        _menu.draw(_state, _game, _show_information_window, _sound_enabled,
            _custom_level_files, _custom_level_paths, _selected_custom_level,
            [&]() { start_game(); }, 
            [&]() { go_to_menu(); }, 
            [&]() { next_level(); },
            [&](size_t index, bool is_custom) { select_level(index, is_custom); }
        );

        // Save settings only if they changed and not during gameplay
        if (_state != GameState::Playing && have_settings_changed()) {
            save_settings();
        }

        // handle all the inputs
        execute_input();

        // game state: playing 
        if (_state == GameState::Playing) {
            const int alive_before_update = count_alive_blocks(); // for sound effect logic
            auto result = _game.update_game(static_cast<float>(Time::get_delta())); // update game logic and get result (win/gameover/none)
            const int alive_after_update = count_alive_blocks(); // for sound effect logic
            if (alive_after_update < alive_before_update) {
                play_brick_break_sound(alive_before_update - alive_after_update);
            }
            // check if a life was lost and play sound
            if (_game.lives < _previous_lives) {
                play_loss_sound();
                _previous_lives = _game.lives;
            }
            // check if a powerup was caught and play sound
            if (_game.powerup_caught_this_frame) {
                play_powerup_sound();
            }
            // update lives, if heart power up was caught
            if (_game.lives > _previous_lives) {
                _previous_lives = _game.lives;
            }
            // check win/gameover state (play win sound)
            if (result == BreakoutState::UpdateResult::Win) {
                play_win_sound();
                _state = GameState::Win;
            }
            else if (result == BreakoutState::UpdateResult::GameOver) _state = GameState::GameOver;
        }

        // draw blank world, clear previous frame data 
        glViewport(0, 0, static_cast<GLsizei>(_screen.x), static_cast<GLsizei>(_screen.y)); 
        glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        // function that actually draws the 2D scene (paddle, ball, bricks, powerups, hearts)
        draw_scene_2d();

        // present to the screen
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(_window._window_p);
        Input::flush();
    }


    // draw rectangles for all entities in the game state (paddle, ball, bricks, power-ups, hearts)
    void draw_rect(const Rect& rect, bool textured = false, Texture* texture = nullptr) {
        _pipeline.bind();
        glUniform2f(0, rect.pos.x, rect.pos.y); // sends position to shader as uniform 0 (flat2d.vert)
        glUniform2f(1, rect.size.x, rect.size.y); // sends size to shader as uniform 1 (flat2d.vert)
        glUniform2f(2, _screen.x, _screen.y); // sends screen dimensions to shader as uniform 2 (flat2d.vert)

        // optionally bind texture
        bool can_use_texture = textured && _game.use_textures && texture != nullptr;
        if (can_use_texture) {
            texture->bind();
        }

        // bind material with color (if no texture) or white (if using texture)
        _rect_material.bind(can_use_texture, rect.color);
        _quad_mesh.draw();
    }

    // draw heart shape for lives
    void draw_heart(const glm::vec2& pos, float size, Texture* texture = nullptr) {
        constexpr float kHeartTextureAspect = 128.0f / 116.0f;
        const bool can_use_texture = texture != nullptr && _game.use_textures;

        _pipeline.bind();
        glUniform2f(0, pos.x, pos.y); // sends position to shader as uniform 0 (flat2d.vert)
        glUniform2f(2, _screen.x, _screen.y); // sends screen dimensions to shader as uniform 2 (flat2d.vert)

        if (can_use_texture) {
            const glm::vec2 textured_size{size * kHeartTextureAspect, size};
            glUniform2f(1, textured_size.x, textured_size.y); // sends textured size to shader as uniform 1 (flat2d.vert)   
            texture->bind();
            _rect_material.bind(true, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
            _quad_mesh.draw();
            return;
        }

        glUniform2f(1, size, size); // sends size to shader as uniform 1 (flat2d.vert)
        _rect_material.bind(false, BreakoutConfig::kHeartColor);
        _heart_mesh.draw();
    }

    // get texture for a given brick symbol (symbols '1' to '9' map to brick colors)
    auto brick_texture_for_symbol(char symbol) -> Texture* {
        // check if symbol is valid (must be between '1' and '9')
        if (symbol < '1' || symbol > '9') {
            return nullptr;
        }
        // texture is valid, return pointer to it
        return &_brick_textures[symbol - '1'];
    }

    // counts number of alive blocks in the current game state
    int count_alive_blocks() const {
        int alive_count = 0;

        for (const Block& block : _game.blocks) {
            if (block.alive) {
                alive_count++;
            }
        }

        return alive_count;
    }

    // loads audio files, prepares stream
    void init_audio() {
        if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
            fmt::println("Audio init failed: {}", SDL_GetError());
            return;
        }

        if (!SDL_LoadWAV("../assets/audio/soundbrickbreak.wav", &audio_file.spec, &audio_file.buffer, &audio_file.buffer_size)) {
            fmt::println("Audio file load failed: {}", SDL_GetError());
            return;
        }

        if (!SDL_LoadWAV("../assets/audio/soundlifeloss.wav", &audio_file_loss.spec, &audio_file_loss.buffer, &audio_file_loss.buffer_size)) {
            fmt::println("Loss audio file load failed: {}", SDL_GetError());
        }

        if (!SDL_LoadWAV("../assets/audio/soundgamewin.wav", &audio_file_win.spec, &audio_file_win.buffer, &audio_file_win.buffer_size)) {
            fmt::println("Win audio file load failed: {}", SDL_GetError());
        }

        if (!SDL_LoadWAV("../assets/audio/soundpowerup.wav", &audio_file_powerup.spec, &audio_file_powerup.buffer, &audio_file_powerup.buffer_size)) {
            fmt::println("Powerup audio file load failed: {}", SDL_GetError());
        }

        audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr, nullptr, nullptr);
        if (audio_stream == nullptr) {
            fmt::println("Audio device stream open failed: {}", SDL_GetError());
            return;
        }

        SDL_AudioSpec device_format;
        if (!SDL_GetAudioDeviceFormat(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &device_format, nullptr)) {
            fmt::println("Audio device format query failed: {}", SDL_GetError());
            return;
        }

        if (!SDL_SetAudioStreamFormat(audio_stream, &audio_file.spec, &device_format)) {
            fmt::println("Audio stream format set failed: {}", SDL_GetError());
            return;
        }

        if (!SDL_ResumeAudioStreamDevice(audio_stream)) {
            fmt::println("Audio stream start failed: {}", SDL_GetError());
        }
    }

    // play brick destroyed sound
    // prevent overlapping or queuing of many sounds
    void play_brick_break_sound(int count = 1) {
        if (!_sound_enabled) return;  // sound disabled, do nothing
        if (audio_stream == nullptr || audio_file.buffer == nullptr || audio_file.buffer_size == 0) {
            // audio not initialized, do nothing
            return;
        } else {
            // latest event wins (replace queued sound with newest one)
            (void)count;
            if (!SDL_ClearAudioStream(audio_stream)) {
                fmt::println("Audio stream clear failed: {}", SDL_GetError());
            }
            if (!SDL_PutAudioStreamData(audio_stream, audio_file.buffer, audio_file.buffer_size)) {
                fmt::println("Audio stream queue failed: {}", SDL_GetError());
            }
        }
    }

    // play game over sound, when a life is lost
    void play_loss_sound() {
        if (!_sound_enabled) return;  // Sound disabled, do nothing
        if (audio_stream == nullptr || audio_file_loss.buffer == nullptr || audio_file_loss.buffer_size == 0) {
            // loss audio not initialized, do nothing
            return;
        } else {
            // loss audio is ready, queue it
            if (!SDL_PutAudioStreamData(audio_stream, audio_file_loss.buffer, audio_file_loss.buffer_size)) {
                fmt::println("Loss audio stream queue failed: {}", SDL_GetError());
            }
        }
    }

    // play the sound, when a level is beaten 
    void play_win_sound() {
        if (!_sound_enabled) return;  // sound disabled, do nothing
        if (audio_stream == nullptr || audio_file_win.buffer == nullptr || audio_file_win.buffer_size == 0) {
            // win audio not initialized, do nothing
            return;
        } else {
            // win audio is ready, queue it
            if (!SDL_PutAudioStreamData(audio_stream, audio_file_win.buffer, audio_file_win.buffer_size)) {
                fmt::println("Win audio stream queue failed: {}", SDL_GetError());
            }
        }
    }

    // play sound when a powerup is caught
    // prevent overlapping or queuing of many sounds
    void play_powerup_sound() {
        if (!_sound_enabled) return;  // sound disabled, do nothing
        if (audio_stream == nullptr || audio_file_powerup.buffer == nullptr || audio_file_powerup.buffer_size == 0) {
            // powerup audio not initialized, do nothing
            return;
        } else {
            // latest event wins (replace queued sound with newest one)
            if (!SDL_ClearAudioStream(audio_stream)) {
                fmt::println("Audio stream clear failed: {}", SDL_GetError());
            }
            if (!SDL_PutAudioStreamData(audio_stream, audio_file_powerup.buffer, audio_file_powerup.buffer_size)) {
                fmt::println("Powerup audio stream queue failed: {}", SDL_GetError());
            }
        }
    }

    // check if settings have changed
    bool have_settings_changed() {
        // check if any settings differ from the last saved state
        return _settings.use_textures != _game.use_textures ||
               _settings.use_gradient_brick_colors != _game.use_gradient_brick_colors ||
               _settings.paddle_speed_percent != _game.paddle_speed_percent ||
               _settings.show_information_window != _show_information_window ||
               _settings.sound_enabled != _sound_enabled;
    }

    void save_settings() {
        // sync game state to settings struct
        _settings.use_textures = _game.use_textures;
        _settings.use_gradient_brick_colors = _game.use_gradient_brick_colors;
        _settings.paddle_speed_percent = _game.paddle_speed_percent;
        _settings.show_information_window = _show_information_window;
        _settings.sound_enabled = _sound_enabled;
        // save to file
        _settings.save();
    }

    // handle player input
    void execute_input() {
        // paddle movement and ball release
        if (_state == GameState::Playing) {
            float speed = _game.paddle_speed();
            
            // paddle movement left
            if (Keys::down(SDLK_A) || Keys::down(SDLK_LEFT)) {
                _game.paddle.pos.x -= speed * Time::get_delta();
            }
            
            // paddle movement right
            if (Keys::down(SDLK_D) || Keys::down(SDLK_RIGHT)) {
                _game.paddle.pos.x += speed * Time::get_delta();
            }
            
            // clamp paddle to screen bounds
            _game.paddle.pos.x = glm::clamp(_game.paddle.pos.x, 0.0f, _game.screen.x - _game.paddle.size.x);

            // release ball with space or left mouse click
            if (_game.ball_stuck && (Keys::pressed(SDLK_SPACE) || Mouse::pressed(Mouse::ids::left))) {
                _game.ball_stuck = false;
            }
        }
    }


    // draws all game entities (paddle, ball, bricks, powerups, hearts) 
    void draw_scene_2d() {
        // draw blocks
        for (const auto& block : _game.blocks) {
            if (block.alive) {
                Texture* brick_texture = brick_texture_for_symbol(block.symbol);
                draw_rect(block.rect, brick_texture != nullptr, brick_texture);
            }
        }
        // draw powerups 
        for (const auto& powerup : _game.powerups) {
            if (powerup.active) {
                // check powerup type and use appropriate rendering
                if (powerup.type == PowerUp::Type::ExtraLife) {
                    // draw falling heart for extra life powerup using heart mesh
                    draw_heart(powerup.rect.pos, powerup.rect.size.x, &_heart_texture);
                } else if (powerup.type == PowerUp::Type::PaddleWiden) {
                    draw_rect(powerup.rect, true, &_powerup_widen_texture);
                } else if (powerup.type == PowerUp::Type::Piercing) {
                    draw_rect(powerup.rect, true, &_powerup_piercing_texture);
                } else if (powerup.type == PowerUp::Type::MultiBall) {
                    draw_rect(powerup.rect, true, &_powerup_multiball_texture);
                } else if (powerup.type == PowerUp::Type::SlowBall) {
                    draw_rect(powerup.rect, true, &_powerup_slowball_texture);
                } else {
                    draw_rect(powerup.rect);
                }
            }
        }

        // paddle texture prio (piercing > slow > widen > normal) based on active powerups
        if (_game.piercing_timer > 0.0f) {
            draw_rect(_game.paddle, true, &_paddle_piercing_texture);
        } else if (_game.slow_ball_timer > 0.0f) {
            draw_rect(_game.paddle, true, &_paddle_slow_texture);
        } else if (_game.paddle_wide_timer > 0.0f) {
            draw_rect(_game.paddle, true, &_paddle_wide_texture);
        } else {
            draw_rect(_game.paddle, true, &_paddle_texture);
        }
        // draw balls
        for (const auto& ball : _game.balls) {
            Rect ball_rect;
            ball_rect.pos = ball.pos - glm::vec2(ball.radius);
            ball_rect.size = glm::vec2(ball.radius * 2.0f);
            ball_rect.color = ball.color;
            
            // use ball texture whenever it is available
            draw_rect(ball_rect, true, &_ball_texture);
        }
        draw_lives();
    }

    // draw player lives bottom left 
    void draw_lives() {
        const glm::vec2 heart_size = {BreakoutConfig::kHeartCols * BreakoutConfig::kHeartPixel, BreakoutConfig::kHeartRows * BreakoutConfig::kHeartPixel};
        const float padding = BreakoutConfig::kHeartPadding;
        glm::vec2 start = {BreakoutConfig::kHeartScreenPadding, _screen.y - heart_size.y - BreakoutConfig::kHeartScreenPadding};
        int base_lives = std::min(_game.lives, _game.max_lives);
        int extra_lives = std::max(_game.lives - _game.max_lives, 0);

        int total_to_draw = base_lives + extra_lives;
        for (int i = 0; i < total_to_draw; i++) {
            glm::vec2 pos = {start.x + i * (heart_size.x + padding), start.y};
            draw_heart(pos, heart_size.x, &_heart_texture);
        }
    }

    // starts a new game 
    void start_game() {
        _game.lives = _game.max_lives;
        _previous_lives = _game.lives;
        _game.reset_level();
        _game.reset_ball(true);
        _state = GameState::Playing;
    }

    // back to main menu
    void go_to_menu() {
        _game.lives = _game.max_lives;
        _game.reset_level();
        _game.reset_ball(true);
        _state = GameState::Menu;
    }

    // prepares the next level 
    void next_level() {
        _game.using_custom_level = false;
        _game.custom_level_name.clear();
        _game.level_index = (_game.level_index + 1) % BreakoutLevels::layouts.size();
        _game.reset_level();
        _game.reset_ball(true);
        _state = GameState::Playing;
    }

    // selects a level by index (normal or custom)
    void select_level(size_t index, bool is_custom) {
        if (is_custom) {
            if (index >= _custom_level_paths.size()) {
                fmt::println("Invalid custom level index: {}", index);
                return;
            }

            if (_game.load_custom_level(_custom_level_paths[index])) {
                _game.using_custom_level = true;
                _game.custom_level_name = _custom_level_files[index];
                _game.lives = _game.max_lives;
                _previous_lives = _game.lives;
                fmt::println("Selected custom level: {}", _custom_level_files[index]);
            } else {
                fmt::println("Failed to load custom level: {}", _custom_level_files[index]);
                return;
            }
        } else {
            if (index >= BreakoutLevels::layouts.size()) {
                fmt::println("Invalid level index: {}", index);
                return;
            }
            _game.using_custom_level = false;
            _game.custom_level_name.clear();
            _game.level_index = index;
            fmt::println("Selected level: {} - {}", index + 1, BreakoutLevels::names[index]);
        }

        _game.reset_level();
        _game.reset_ball(true);
    }

    Window _window;
    Camera _camera;
    Pipeline _pipeline;                      
    Mesh _quad_mesh;                         
    Mesh _heart_mesh;                        
    Material _rect_material; 
    
    // textures
    Texture _paddle_texture;
    Texture _paddle_wide_texture;
    Texture _paddle_piercing_texture;
    Texture _paddle_slow_texture;
    Texture _ball_texture;
    Texture _powerup_widen_texture;
    Texture _powerup_piercing_texture;
    Texture _powerup_multiball_texture;
    Texture _powerup_slowball_texture;
    Texture _heart_texture;
    std::array<Texture, 9> _brick_textures;  // brick textures for symbols '1'..'9'
    glm::vec2 _screen;

    BreakoutState _game;
    MenuUI _menu;
    GameState _state = GameState::Menu;
    GameSettings _settings;
    bool _show_information_window = true;
    bool _sound_enabled = true;
    
    // custom levels
    std::vector<std::string> _custom_level_files; // list of custom level filenames
    std::vector<std::string> _custom_level_paths; // full paths of custom level files
    size_t _selected_custom_level = 0;

    // audio
    struct AudioFile {
        void init() {}
        void destroy() {}
        SDL_AudioSpec spec;
        Uint8* buffer = nullptr;
        Uint32 buffer_size = 0;
    };
    AudioFile audio_file;
    AudioFile audio_file_loss;  
    AudioFile audio_file_win;  
    AudioFile audio_file_powerup;  
    SDL_AudioStream* audio_stream = nullptr;
    int _previous_lives = 0;  
};
