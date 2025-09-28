#include "tests.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

const int MIN_ASCII = 97;
const int MAX_ASCII = 122;

const int LINE_LENGTH = 100;
const int INIT_N = 0;
const int STEP = 1000; 
const int MAX_N = 100000;

std::string generate_random_text(int chars) {
    std::string text;

    for (int i = 0; i < chars; i++) {
        char rand_char = MIN_ASCII + (rand() % (MAX_ASCII - MIN_ASCII));
        text.push_back(rand_char);
    }

    return text;
}

void save_new_texts() {
    int name = 1;
    std::filesystem::create_directories("texts");
    for (int count = INIT_N; count <= MAX_N; count += STEP) {
        std::ofstream file;
        file.open("texts/test"+std::to_string(name)+".txt", std::ofstream::trunc);
        for (int i = 0; i < count; i++) {
            file << generate_random_text(LINE_LENGTH) << "\n";
        }
        name++;
    }
}
