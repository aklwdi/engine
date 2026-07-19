#include <stdio.h>
#include <raylib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream> 

std::string LoadFileToString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "hi";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
// Helper function to save string back to disk safely
void SaveStringToFile(const std::string& filename, const std::string& text) {
    std::ofstream out_file(filename);
    if (out_file.is_open()) {
        out_file << text;
        out_file.close(); 
    }
}
int main() {
    InitWindow(800, 450, "Editor");
    const std::string filename = "code.ez";
    std::string textBuffer = LoadFileToString(filename);
    int backspaceFrameCounter = 0;
    int cursorFrameCounter = 0;
    bool showCursor = true;
    Rectangle buttonBounds = { 630, 40, 140, 40 };
    std::string preset1Code = "test 4 code";
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();
        bool isHovered = CheckCollisionPointRec(mousePos, buttonBounds);
        if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            textBuffer = preset1Code;
            SaveStringToFile(filename, textBuffer);
        }
        int key = GetCharPressed(); 
        while (key > 0) {
            if ((key >= 32) && (key <= 125)) {
                textBuffer.push_back((char)key);
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_ENTER)) {
            textBuffer.append("\n");
        }
        if (IsKeyDown(KEY_BACKSPACE)) {
            backspaceFrameCounter++;
            if (IsKeyPressed(KEY_BACKSPACE) || (backspaceFrameCounter > 25 && backspaceFrameCounter % 3 == 0)) {
                if (!textBuffer.empty()) {
                    textBuffer.pop_back();
                }
            }
        } else {
            backspaceFrameCounter = 0;
        }
        if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_S)) {
            SaveStringToFile(filename, textBuffer);
        }
        cursorFrameCounter++;
        if (cursorFrameCounter >= 30) {
            showCursor = !showCursor;
            cursorFrameCounter = 0;
        }
        BeginDrawing();
        ClearBackground(GetColor(0x1e1e1eff)); 
        int startX = 40;
        int startY = 40;
        int fontSize = 20;
        int textX = startX;
        int textY = startY;
        for (size_t i = 0; i < textBuffer.length(); i++) {
            char c = textBuffer[i];
            if (c == '\n') {
                textX = startX;
                textY += fontSize + 5;
            } else {
                char str[] = { c, '\0' };
                DrawText(str, textX, textY, fontSize, RAYWHITE);
                textX += MeasureText(str, fontSize) + 2;
            }
        }
        if (showCursor) {
            DrawRectangle(textX, textY, 2, fontSize, LIGHTGRAY);
        }
        Color buttonColor = DARKGRAY;
        if (isHovered) {
            buttonColor = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? MAROON : GRAY;
        }
        DrawRectangleRec(buttonBounds, buttonColor);
        DrawRectangleLinesEx(buttonBounds, 2, RAYWHITE);
        int btnTextWidth = MeasureText("platformer", 16);
        int btnTextX = buttonBounds.x + (buttonBounds.width - btnTextWidth) / 2;
        int btnTextY = buttonBounds.y + (buttonBounds.height - 16) / 2;
        DrawText("platformer", btnTextX, btnTextY, 16, RAYWHITE);
        DrawText("press the platformer preset to have a platformer in your hands!", 40, 410, 14, GRAY);
        EndDrawing();
    }
    SaveStringToFile(filename, textBuffer);
    CloseWindow();
    return 0;
}
