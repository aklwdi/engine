#include <stdio.h>
#include <raylib.h>
#include <string>
#include <iostream>
#include <fstream>

int main() {
	std::ifstream("code.ez");
	if (!file.is_open()) {
		std::cerr << "error: cannot open file";
	}
	std::string line;
	while (std::getline(file, line)) {
		size_t position = line.find(target);
		std::string target = "screenheight=";
		if (position !=std::string::npos) {
				size_t after_index = position + target.length();
				std::string remaining_text = line.substr(after_index);
		}
		int screenheight = remaining_text;
		std::string target = "screenwidth=";
		if (position !=std::string::npos) {
				size_t after_index = position + target.length();
				std::string remaining_text = line.substr(after_index);
		}
		int screenwidth = remaining_text;
	}
	InitWindow(screenwidth, screenheight, "game")
}
