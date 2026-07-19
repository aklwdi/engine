#include <stdio.h>
#include <raylib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream> 
#include <vector>
struct CharPos {
    size_t index;
    Rectangle bounds;
};
std::string LoadFileToString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return "hi";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
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
    size_t cursorIndex = textBuffer.length();
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
            cursorIndex = textBuffer.length();
            SaveStringToFile(filename, textBuffer);
        }
        int key = GetCharPressed(); 
        while (key > 0) {
            if ((key >= 32) && (key <= 125)) {
                textBuffer.insert(cursorIndex, 1, (char)key);
                cursorIndex++;
                showCursor = true;
                cursorFrameCounter = 0;
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_ENTER)) {
            textBuffer.insert(cursorIndex, 1, '\n');
            cursorIndex++;
            showCursor = true;
            cursorFrameCounter = 0;
        }
        if (IsKeyDown(KEY_BACKSPACE)) {
            backspaceFrameCounter++;
            if (IsKeyPressed(KEY_BACKSPACE) || (backspaceFrameCounter > 25 && backspaceFrameCounter % 3 == 0)) {
                if (cursorIndex > 0 && !textBuffer.empty()) {
                    textBuffer.erase(cursorIndex - 1, 1);
                    cursorIndex--;
                    showCursor = true;
                    cursorFrameCounter = 0;
                }
            }
        } else {
            backspaceFrameCounter = 0;
        }
        if (IsKeyPressed(KEY_LEFT) && cursorIndex > 0) {
            cursorIndex--;
            showCursor = true;
            cursorFrameCounter = 0;
        }
        if (IsKeyPressed(KEY_RIGHT) && cursorIndex < textBuffer.length()) {
            cursorIndex++;
            showCursor = true;
            cursorFrameCounter = 0;
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
        int cursorRenderX = startX;
        int cursorRenderY = startY;
        std::vector<CharPos> clickMap;
        if (cursorIndex == 0) {
            cursorRenderX = textX;
            cursorRenderY = textY;
        }
        for (size_t i = 0; i < textBuffer.length(); i++) {
            char c = textBuffer[i];
            int glyphWidth = 0;
            if (c != '\n') {
                char str[] = { c, '\0' };
                glyphWidth = MeasureText(str, fontSize) + 2;
                Rectangle charBox = { (float)textX, (float)textY, (float)glyphWidth, (float)(fontSize + 5) };
                clickMap.push_back({ i, charBox });
                DrawText(str, textX, textY, fontSize, RAYWHITE);
            } else {
                Rectangle nlBox = { (float)textX, (float)textY, 15.0f, (float)(fontSize + 5) };
                clickMap.push_back({ i, nlBox });
            }
            if (c == '\n') {
                textX = startX;
                textY += fontSize + 5;
            } else {
                textX += glyphWidth;
            }
            if (i + 1 == cursorIndex) {
                cursorRenderX = textX;
                cursorRenderY = textY;
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !isHovered) {
            bool foundClick = false;
            for (const auto& cp : clickMap) {
                if (CheckCollisionPointRec(mousePos, cp.bounds)) {
                    if (mousePos.x > cp.bounds.x + cp.bounds.width / 2 && textBuffer[cp.index] != '\n') {
                        cursorIndex = cp.index + 1;
                    } else {
                        cursorIndex = cp.index;
                    }
                    foundClick = true;
                    showCursor = true;
                    cursorFrameCounter = 0;
                    break;
                }
            }
            if (!foundClick && mousePos.x >= startX && mousePos.y >= startY && mousePos.y <= textY + fontSize + 5) {
                cursorIndex = textBuffer.length();
                showCursor = true;
                cursorFrameCounter = 0;
            }
        }
        if (showCursor) {
            DrawRectangle(cursorRenderX, cursorRenderY, 2, fontSize, LIGHTGRAY);
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
