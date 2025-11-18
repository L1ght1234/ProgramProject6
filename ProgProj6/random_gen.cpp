#include "random_gen.h"
#include <random>

Generator<int> create_random_sequence() {
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<> distributor(1, 256);

    const auto& promise = co_await Generator<int>::PromiseAccessor{};

    while (true) {
        int current_random = distributor(engine);

        co_yield current_random;

        if (promise.should_stop()) {
            co_return;
        }
    }
}