#pragma once
#include <functional>
#include <string>
#include <SDL3/SDL_events.h>
#include <imgui.h>
#include "input.hpp"
#include "game/config.hpp"
#include "game/game_state.hpp"
#include "game/breakout_state.hpp"
#include "game/levels.hpp"

/*
defines game menu and how the user can interact with it 
user can start/restart a game, select normal/custom level, adjust settings or quit the game
*/

struct MenuUI {

        GameState options_return = GameState::Menu; // start with menu state

        void draw(GameState& state, 
                  BreakoutState& game, 
                  bool& show_information_window, // toggle for info window 
                  bool& sound_enabled, 
                  const std::vector<std::string>& custom_level_files, // custom level file names
                  const std::vector<std::string>& custom_level_paths, // custom level file paths
                  size_t& selected_custom_level, // currently selected custom level index
                  const std::function<void()>& start_game, // callback to start game 
                  const std::function<void()>& go_to_menu, // callback to go to menu
                  const std::function<void()>& next_level, // callback advance to next level
                  const std::function<void(size_t, bool)>& select_level) { // callback to select level (index, is_custom)

            // ESK Key to open/close menu
            if (Input::Keys::pressed(SDLK_ESCAPE)) {
                if (state == GameState::Playing) state = GameState::Paused;
                else if (state == GameState::Paused) state = GameState::Playing;
            }

            if (state == GameState::Playing) return; // no menu during gameplay

            // Menu window setup
            ImGuiIO& io = ImGui::GetIO();
            ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
            ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(BreakoutConfig::kMenuWindowWidth, BreakoutConfig::kMenuWindowHeight), ImGuiCond_Always);

            ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
            ImGui::Begin("Menu", nullptr, flags);

            // Main Menu
            if (state == GameState::Menu) {
                ImGui::Text("Breakout");
                if (game.using_custom_level) {
                    ImGui::Text("Selected Custom: %s", game.custom_level_name.c_str());
                } else {
                    ImGui::Text("Selected Level: %d - %s", 
                        static_cast<int>(game.level_index) + 1,
                        BreakoutLevels::names[game.level_index]);
                }
                if (ImGui::Button("Start", ImVec2(-1, 0))) start_game();
                if (ImGui::Button("Select Level", ImVec2(-1, 0))) state = GameState::LevelSelect;
                if (ImGui::Button("Custom Levels", ImVec2(-1, 0))) state = GameState::CustomLevelSelect;
                if (ImGui::Button("Options", ImVec2(-1, 0))) { options_return = GameState::Menu; state = GameState::Options; }
                if (ImGui::Button("Quit", ImVec2(-1, 0))) { SDL_Event quit_event{}; quit_event.type = SDL_EVENT_QUIT; SDL_PushEvent(&quit_event); }
            // Mainsubmenu: Level Select
            } else if (state == GameState::LevelSelect) {
                ImGui::Text("Select Level");
                for (size_t i = 0; i < BreakoutLevels::layouts.size(); i++) {
                    std::string label = std::string("Level ") + std::to_string(i + 1) + " - " + BreakoutLevels::names[i];
                    if (ImGui::Button(label.c_str(), ImVec2(-1, 0))) {
                        select_level(i, false);
                        state = GameState::Menu;
                    }
                }
                if (ImGui::Button("Back", ImVec2(-1, 0))) state = GameState::Menu;
            // Mainsubmenu: Custom Level Select
            } else if (state == GameState::CustomLevelSelect) {
                ImGui::Text("Custom Levels");
                
                if (custom_level_files.empty()) {
                    ImGui::TextWrapped("No custom levels found. Place .txt files in the 'levels' folder.");
                } else {
                    ImGui::Text("Found %zu custom level(s):", custom_level_files.size());
                    ImGui::Separator();
                    
                    for (size_t i = 0; i < custom_level_files.size(); i++) {
                        if (ImGui::Button(custom_level_files[i].c_str(), ImVec2(-1, 0))) {
                            selected_custom_level = i;
                            select_level(i, true);
                            state = GameState::Menu;
                        }
                    }
                }
                
                if (ImGui::Button("Back", ImVec2(-1, 0))) state = GameState::Menu;
            // Mainsubmenu: Options
            } else if (state == GameState::Options) {
                ImGui::Text("Options");
                ImGui::SliderFloat("Paddle Speed (%)", &game.paddle_speed_percent, BreakoutConfig::kPaddleSpeedMinPercent, BreakoutConfig::kPaddleSpeedMaxPercent, "%.0f");
                ImGui::Checkbox("Use Textures", &game.use_textures);

                // when textures are ON, gradient colors are automatically disabled and cannot be toggled
                if (game.use_textures) {
                    if (game.use_gradient_brick_colors) {
                        game.set_gradient_brick_colors(false);
                    }
                    ImGui::BeginDisabled(true);
                    bool dummy = false;
                    ImGui::Checkbox("Gradient Brick Colors##disabled", &dummy);
                    ImGui::EndDisabled();
                } else {
                    // when textures are OFF, gradient colors can be toggled normally
                    bool use_gradient = game.use_gradient_brick_colors;
                    if (ImGui::Checkbox("Gradient Brick Colors", &use_gradient)) {
                        game.set_gradient_brick_colors(use_gradient);
                    }
                }
                ImGui::Checkbox("Information Window", &show_information_window);
                ImGui::Checkbox("Sound", &sound_enabled);
                if (ImGui::Button("Back", ImVec2(-1, 0))) state = options_return;
            // Gamemenu: Paused
            } else if (state == GameState::Paused) {
                ImGui::Text("Paused");
                if (ImGui::Button("Resume", ImVec2(-1, 0))) state = GameState::Playing;
                if (ImGui::Button("Restart", ImVec2(-1, 0))) start_game();
                if (ImGui::Button("Options", ImVec2(-1, 0))) { options_return = GameState::Paused; state = GameState::Options; }
                if (ImGui::Button("Main Menu", ImVec2(-1, 0))) go_to_menu();
                if (ImGui::Button("Quit", ImVec2(-1, 0))) { SDL_Event quit_event{}; quit_event.type = SDL_EVENT_QUIT; SDL_PushEvent(&quit_event); }
            // Gamemenu: Game Over
            } else if (state == GameState::GameOver) {
                ImGui::Text("Game Over");
                if (ImGui::Button("Restart", ImVec2(-1, 0))) start_game();
                if (ImGui::Button("Main Menu", ImVec2(-1, 0))) go_to_menu();
                if (ImGui::Button("Quit", ImVec2(-1, 0))) { SDL_Event quit_event{}; quit_event.type = SDL_EVENT_QUIT; SDL_PushEvent(&quit_event); }
            // Gamemenu: Win Screen
            } else if (state == GameState::Win) {
                if (game.using_custom_level) {
                    ImGui::Text("Custom level complete");
                } else {
                    ImGui::Text("Level %d complete", game.level_index + 1);
                }
                const bool has_next_normal_level = !game.using_custom_level &&
                    (game.level_index + 1 < BreakoutLevels::layouts.size());
                if (has_next_normal_level) {
                    if (ImGui::Button("Next Level", ImVec2(-1, 0))) next_level();
                }
                if (ImGui::Button("Restart", ImVec2(-1, 0))) start_game();
                if (ImGui::Button("Main Menu", ImVec2(-1, 0))) go_to_menu();
                if (ImGui::Button("Quit", ImVec2(-1, 0))) { SDL_Event quit_event{}; quit_event.type = SDL_EVENT_QUIT; SDL_PushEvent(&quit_event); }
            }

            ImGui::End();
        }
};
