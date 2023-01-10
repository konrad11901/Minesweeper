#pragma once

class RandomGenerator {
public:
    RandomGenerator();

    std::size_t GetRandomNumber(std::size_t min, std::size_t max);
private:
    std::random_device random_device;
    std::default_random_engine random_engine;
};