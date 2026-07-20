#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

// If your raylib is installed globally in MSYS2, use <raylib.h>
// Otherwise, keep your absolute path: #include <C:\raylib\raylib\src\raylib.h>
#include <raylib.h> 

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

// Helper function to remove trailing/leading spaces
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

int getRaylibKey(const std::string& keyStr) {
    std::string k = trim(keyStr);
    if (k == "W" || k == "w") return KEY_W;
    if (k == "A" || k == "a") return KEY_A;
    if (k == "S" || k == "s") return KEY_S;
    if (k == "D" || k == "d") return KEY_D;
    if (k == "UP" || k == "up") return KEY_UP;
    if (k == "DOWN" || k == "down") return KEY_DOWN;
    if (k == "LEFT" || k == "left") return KEY_LEFT;
    if (k == "RIGHT" || k == "right") return KEY_RIGHT;
    if (k == "SPACE" || k == "space") return KEY_SPACE;
    return 0; // Fixed: Removed the invalid "return keyStr;"
}

int getVarValue(const std::string& nameStr, int sw, int sh) {
    std::string name = trim(nameStr);
    if (name == "screenwidth") return sw;
    if (name == "screenheight") return sh;
    
    if (!name.empty()) {
        bool isNumber = true;
        size_t start = (name[0] == '-') ? 1 : 0;
        for (size_t i = start; i < name.length(); i++) {
            if (!std::isdigit(static_cast<unsigned char>(name[i]))) {
                isNumber = false;
                break;
            }
        }
        if (isNumber && name.length() > start) return std::stoi(name);
    }
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
        line = trim(line);
        if (line.empty()) continue;

        if (line.find("forever{") != std::string::npos || line.find("forever {") != std::string::npos) {
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
                    cmd.objId = std::stoi(trim(s_id));
                    cmd.speedX = std::stoi(trim(s_sx));
                    cmd.speedY = std::stoi(trim(s_sy));
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
            std::string var_name = trim(line.substr(0, delimiter_pos));
            std::string var_value = trim(line.substr(delimiter_pos + 1));
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
                    r.color = {(unsigned char)std::stoi(trim(s_r)), (unsigned char)std::stoi(trim(s_g)), (unsigned char)std::stoi(trim(s_b)), 255};
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
                        rule.objA = std::stoi(trim(idA_str));
                        rule.objB = std::stoi(trim(idB_str));
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
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        for (const auto& r : rectangles) {
            DrawRectangle(r.x, r.y, r.w, r.h, r.color);
        }
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
