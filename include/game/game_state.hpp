#pragma once

// game state for managing UI screens and gameplay flow.
// tracks which screen the player is currently viewing

enum class GameState {
    Menu, // Main menu screen (start game, level select, options, quit)
    LevelSelect, // Level selection screen (choose which level to play)
    CustomLevelSelect, // Custom level selection screen (choose user created levels)
    Options, // Options/settings screen (adjust paddle speed, render mode)
    Playing, // Active gameplay (ball in motion, player controls paddle)
    Paused,  // Gameplay paused (ESC pressed, can resume or return to menu)
    GameOver, // Game over screen (ran out of lives, option to retry or return to menu)
    Win // Level complete/victory screen (all blocks destroyed, advance to next level)
};
