#include <stdio.h>
#include <raylib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
struct GameRect {
    int id;
    int x;
    int y;
    int w;
    int h;
    Color color;
};
struct CollisionRule {
    int objA;
    int objB;
};
struct LoopCommand {
    int objId;
    int speedX;
    int speedY;
};
struct KeyCommand {
    int key;
    std::vector<LoopCommand> actions;
};
int getRaylibKey(const std::string& keyStr) {
    if (keyStr == "W" || keyStr == "w") return KEY_W;
    if (keyStr == "A" || keyStr == "a") return KEY_A;
    if (keyStr == "S" || keyStr == "s") return KEY_S;
    if (keyStr == "D" || keyStr == "d") return KEY_D;
    if (keyStr == "UP" || keyStr == "up") return KEY_UP;
    if (keyStr == "DOWN" || keyStr == "down") return KEY_DOWN;
    if (keyStr == "LEFT" || keyStr == "left") return KEY_LEFT;
    if (keyStr == "RIGHT" || keyStr == "right") return KEY_RIGHT;
    if (keyStr == "SPACE" || keyStr == "space") return KEY_SPACE;
    if (!keyStr.empty()) return keyStr;
    return 0;
}
int getVarValue(const std::string& name, int sw, int sh) {
    if (name == "screenwidth") return sw;
    if (name == "screenheight") return sh;
    if (!name.empty() && isdigit(name)) return std::stoi(name);
    return 0;
}
int parseValue(const std::string& token, int sw, int sh) {
    std::string ops = "+-*/x";
    size_t op_pos = std::string::npos;
    char found_op = ' ';
    for (char op : ops) {
        size_t pos = token.find(op);
        if (pos != std::string::npos) {
            op_pos = pos;
            found_op = op;
            break;
        }
    }
    if (op_pos != std::string::npos) {
        std::string left = token.substr(0, op_pos);
        std::string right = token.substr(op_pos + 1);
        int valA = getVarValue(left, sw, sh);
        int valB = getVarValue(right, sw, sh);
        if (found_op == '+') return valA + valB;
        if (found_op == '-') return valA - valB;
        if (found_op == '*' || found_op == 'x') return valA * valB;
        if (found_op == '/') return (valB != 0) ? valA / valB : 0;
    }
    return getVarValue(token, sw, sh);
}
int main() {
    int screenwidth = 800;
    int screenheight = 450;
    std::vector<GameRect> rectangles;
    std::vector<CollisionRule> rules;
    std::vector<LoopCommand> loopCommands;
    std::vector<KeyCommand> keyCommands;
    std::ifstream file("code.ez");
    if (!file.is_open()) {
        InitWindow(800, 450, "error");
        SetTargetFPS(60);
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("can't open file: code.ez", 50, 50, 20, MAROON);
            EndDrawing();
        }
        CloseWindow();
        return 1;
    }
    std::string line;
    int rectCounter = 1;
    bool inForeverLoop = false;
    bool inKeyBlock = false;
    KeyCommand currentKeyCmd;
    while (std::getline(file, line)) {
        if (line.find("forever{") != std::string::npos) {
            inForeverLoop = true;
            continue;
        }
        if (inForeverLoop && line.find("}") != std::string::npos) {
            inForeverLoop = false;
            continue;
        }
        size_t if_kd_pos = line.find("if(keydown(");
        if (if_kd_pos == std::string::npos) {
            if_kd_pos = line.find("if (keydown(");
        }
        if (if_kd_pos != std::string::npos) {
            size_t then_pos = line.find(") then");
            if (then_pos != std::string::npos) {
                size_t offset = (line.find("if (") != std::string::npos) ? 12 : 11;
                std::string keyStr = line.substr(if_kd_pos + offset, then_pos - (if_kd_pos + offset));
                currentKeyCmd.key = getRaylibKey(keyStr);
                currentKeyCmd.actions.clear();
                inKeyBlock = true;
                continue;
            }
        }
        if (inKeyBlock && line.find("end") != std::string::npos) {
            keyCommands.push_back(currentKeyCmd);
            inKeyBlock = false;
            continue;
        }
        size_t move_pos = line.find("move(");
        if (move_pos != std::string::npos) {
            size_t close_pos = line.find(")");
            if (close_pos != std::string::npos) {
                std::string args = line.substr(move_pos + 5, close_pos - (move_pos + 5));
                std::stringstream ss(args);
                std::string s_id, s_sx, s_sy;
                if (std::getline(ss, s_id, ',') && std::getline(ss, s_sx, ',') && std::getline(ss, s_sy, ',')) {
                    LoopCommand cmd;
                    cmd.objId = std::stoi(s_id);
                    cmd.speedX = std::stoi(s_sx);
                    cmd.speedY = std::stoi(s_sy);
                    if (inKeyBlock) {
                        currentKeyCmd.actions.push_back(cmd);
                    } else if (inForeverLoop) {
                        loopCommands.push_back(cmd);
                    }
                }
            }
            continue;
        }
        if (inForeverLoop || inKeyBlock) continue;
        size_t delimiter_pos = line.find('=');
        if (delimiter_pos != std::string::npos) {
            std::string var_name = line.substr(0, delimiter_pos);
            std::string var_value = line.substr(delimiter_pos + 1);
            if (var_name == "screenwidth") {
                screenwidth = std::stoi(var_value);
            } else if (var_name == "screenheight") {
                screenheight = std::stoi(var_value);
            } else if (var_name == "rect") {
                std::stringstream ss(var_value);
                std::string s_x, s_y, s_w, s_h, s_r, s_g, s_b;
                if (std::getline(ss, s_x, ',') && std::getline(ss, s_y, ',') &&
                    std::getline(ss, s_w, ',') && std::getline(ss, s_h, ',') &&
                    std::getline(ss, s_r, ',') && std::getline(ss, s_g, ',') &&
                    std::getline(ss, s_b, ',')) {
                    GameRect r;
                    r.id = rectCounter++;
                    r.x = parseValue(s_x, screenwidth, screenheight);
                    r.y = parseValue(s_y, screenwidth, screenheight);
                    r.w = parseValue(s_w, screenwidth, screenheight);
                    r.h = parseValue(s_h, screenwidth, screenheight);
                    r.color = {(unsigned char)std::stoi(s_r), (unsigned char)std::stoi(s_g), (unsigned char)std::stoi(s_b), 255};
                    rectangles.push_back(r);
                }
            }
        }
        size_t if_pos = line.find("if(object(");
        if (if_pos != std::string::npos) {
            size_t touch_pos = line.find(").touchobject(");
            if (touch_pos != std::string::npos) {
                size_t then_pos = line.find(") then");
                if (then_pos != std::string::npos) {
                    std::string idA_str = line.substr(if_pos + 10, touch_pos - (if_pos + 10));
                    std::string idB_str = line.substr(touch_pos + 14, then_pos - (touch_pos + 14));
                    if (!idA_str.empty() && !idB_str.empty()) {
                        CollisionRule rule;
                        rule.objA = std::stoi(idA_str);
                        rule.objB = std::stoi(idB_str);
                        rules.push_back(rule);
                    }
                }
            }
        }
    }
    file.close();
    InitWindow(screenwidth, screenheight, "game");
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        for (const auto& cmd : loopCommands) {
            for (auto& r : rectangles) {
                if (r.id == cmd.objId) {
                    r.x += cmd.speedX;
                    r.y += cmd.speedY;
                }
            }
        }
        for (const auto& kCmd : keyCommands) {
            if (IsKeyDown(kCmd.key)) {
                for (const auto& action : kCmd.actions) {
                    for (auto& r : rectangles) {
                        if (r.id == action.objId) {
                            r.x += action.speedX;
                            r.y += action.speedY;
                        }
                    }
                }
            }
        }
        bool ruleConditionMet = false;
        for (const auto& rule : rules) {
            const GameRect* ptrA = nullptr;
            const GameRect* ptrB = nullptr;
            for (const auto& r : rectangles) {
                if (r.id == rule.objA) ptrA = &r;
                if (r.id == rule.objB) ptrB = &r;
            }
            if (ptrA && ptrB) {
                Rectangle rA = {(float)ptrA->x, (float)ptrA->y, (float)ptrA->w, (float)ptrA->h};
                Rectangle rB = {(float)ptrB->x, (float)ptrB->y, (float)ptrB->w, (float)ptrB->h};
                if (CheckCollisionRecs(rA, rB)) {
                    ruleConditionMet = true;
                }
            }
        }
        BeginDrawing();
        ClearBackground(RAYWHITE);
        for (const auto& r : rectangles) {
            DrawRectangle(r.x, r.y, r.w, r.h, r.color);
        }
        if (ruleConditionMet) {
            DrawText("COLLISION TRIGGERED!", 10, 10, 20, RED);
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
