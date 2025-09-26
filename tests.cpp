#include "tests.h"

#include <cstdlib>
#include <fstream>
#include <string>

const int MIN_ASCII = 97;
const int MAX_ASCII = 122;
const int INIT = 100000;
const int STEP = INIT/2; 
const int MAX = 1000000;

std::string generate_random_text(int chars) {
    std::string text;

    for (int i = 0; i < chars; i++) {
        char rand_char = MIN_ASCII + (rand() % (MAX_ASCII - MIN_ASCII));
        text.push_back(rand_char);
    }

    return text;
}

void save_new_texts() {
    for (int count = INIT; count <= MAX; count += STEP) {
        std::ofstream file;
        file.open("texts/test"+std::to_string(count)+".txt");
        file << generate_random_text(count) << "\n";
    }
}
