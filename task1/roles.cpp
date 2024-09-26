#include <vector>
#include <random>
#include <algorithm>
#include <iostream>

template <typename T>
T choose_random(const std::vector<T>& players) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    if (players.empty()) {
        throw std::invalid_argument("players vector is empty");
    }
    std::vector<T> sample(1);
    std::sample(players.begin(), players.end(), sample.begin(), 1, gen);
    return sample[0];
}


