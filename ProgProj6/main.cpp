#include "random_gen.h"
#include <iostream>

int main() {
    std::cout << "Starting random sequence generation..." << std::endl;

    int previous = 0;

    for (int current : create_random_sequence()) {

        std::cout << "Generated: " << current;

        if (previous != 0) {
            std::cout << " (Difference: " << std::abs(current - previous) << ")" << std::endl;
        }
        else {
            std::cout << " (First value)" << std::endl;
        }


        previous = current;
    }

    std::cout << "Sequence stopped (difference was less than 16)." << std::endl;

    return 0;
}