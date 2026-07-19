#include <stdio.h>
#include <raylib.h>
#include <string>
#include <iostream>
#include <fstream>

int main() {
    int screenwidth = 800;
    int screenheight = 450;
    std::ifstream file("code.ez");
    if (!file.is_open()) {
        InitWindow(800,450,"error");
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("can't open file", 50, 50, 20, DARKGRAY);
        EndDrawing();
    }
    std::string line;
    while (std::getline(file, line)) {
        std::string targetHeight = "screenheight=";
        size_t posHeight = line.find(targetHeight);
        if (posHeight != std::string::npos) {
            size_t after_index = posHeight + targetHeight.length();
            std::string remaining_text = line.substr(after_index);
            screenheight = std::stoi(remaining_text);
        }
        std::string targetWidth = "screenwidth=";
        size_t posWidth = line.find(targetWidth);
        if (posWidth != std::string::npos) {
            size_t after_index = posWidth + targetWidth.length();
            std::string remaining_text = line.substr(after_index);
            screenwidth = std::stoi(remaining_text);
        }
    }
    file.close();
    InitWindow(screenwidth, screenheight, "game");
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
