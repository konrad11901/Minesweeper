#include "pch.h"
#include "RandomGenerator.h"

RandomGenerator::RandomGenerator() : random_device(), random_engine(random_device()) {}

std::size_t RandomGenerator::GetRandomNumber(std::size_t min, std::size_t max) {
    std::uniform_int_distribution<std::size_t> distribution(min, max);
    return distribution(random_engine);
}