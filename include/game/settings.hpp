#pragma once
#include <exception>
#include <string>
#include <fstream>
#include <glm/common.hpp>
#include <fmt/base.h>


/*
Persistence settings system
saves and loads user preferences to/from a JSON file.when the game starts
*/

struct GameSettings {

    bool use_textures = false; // if textures or normal colors are used
    bool use_gradient_brick_colors = true; // if gradient brick colors are used (only applies when textures are off)
    float paddle_speed_percent = 50.0f; // percentage of base paddle speed (0% to 100%)
    bool show_information_window = true; // disable/enable information window
    bool sound_enabled = true; // disable/enable all game sounds

    // load settings from file, or use defaults if file doesnt exist
    void load(const std::string& filename = "../bin/settings.json") {
        std::ifstream file(filename);
        if (!file.is_open()) {
            fmt::println("Settings file not found, using defaults.");
            return;
        }

        GameSettings parsed_settings;

        try {
            std::string line;
            while (std::getline(file, line)) {
                size_t colon_pos = line.find(':');
                if (colon_pos == std::string::npos) continue;

                // extract key and value
                std::string key = line.substr(0, colon_pos);
                std::string value = line.substr(colon_pos + 1);

                // trim whitespace and quotes
                key.erase(0, key.find_first_not_of(" \t\","));
                key.erase(key.find_last_not_of(" \t\",") + 1);
                value.erase(0, value.find_first_not_of(" \t\","));
                value.erase(value.find_last_not_of(" \t\",") + 1);

                // parse known keys and set settings
                if (key == "use_textures") {
                    parsed_settings.use_textures = (value == "true");
                } else if (key == "use_gradient_brick_colors") {
                    parsed_settings.use_gradient_brick_colors = (value == "true");
                } else if (key == "paddle_speed_percent") {
                    float speed = std::stof(value);
                    parsed_settings.paddle_speed_percent = glm::clamp(speed, 0.0f, 100.0f);
                } else if (key == "show_information_window") {
                    parsed_settings.show_information_window = (value == "true");
                } else if (key == "sound_enabled") {
                    parsed_settings.sound_enabled = (value == "true");
                }
            }

            use_textures = parsed_settings.use_textures;
            use_gradient_brick_colors = parsed_settings.use_gradient_brick_colors;
            paddle_speed_percent = parsed_settings.paddle_speed_percent;
            show_information_window = parsed_settings.show_information_window;
            sound_enabled = parsed_settings.sound_enabled;
            fmt::println("Settings loaded from {}", filename);
        } catch (const std::exception& e) {
            fmt::println("Failed to parse settings from {}: {}", filename, e.what());
            fmt::println("Using default settings.");
        }
    }

    // save settings to file.
    void save(const std::string& filename = "../bin/settings.json") const {
        
        std::ofstream file(filename); // open file for writing (will create or overwrite)
        
        if (!file.is_open()) {
            fmt::println("Failed to open settings file for writing.");
            return;
        }

        file << "{\n";
        file << "  \"use_textures\": " << (use_textures ? "true" : "false") << ",\n";
        file << "  \"use_gradient_brick_colors\": " << (use_gradient_brick_colors ? "true" : "false") << ",\n";
        file << "  \"paddle_speed_percent\": " << paddle_speed_percent << ",\n";
        file << "  \"show_information_window\": " << (show_information_window ? "true" : "false") << ",\n";
        file << "  \"sound_enabled\": " << (sound_enabled ? "true" : "false") << "\n";
        file << "}\n";

        file.close();
        //fmt::println("Settings saved to {}", filename);
    }
};
