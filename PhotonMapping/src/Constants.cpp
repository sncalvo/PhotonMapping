#include "Constants.hpp"

#include <unordered_map>
#include <iostream>

std::unordered_map<std::string, bool> BOOL_CONSTANTS;
std::unordered_map<std::string, float> FLOAT_CONSTANTS;
std::unordered_map<std::string, int> INT_CONSTANTS;

std::string WIDTH = "WIDTH";
std::string HEIGHT = "HEIGHT";

std::string MAX_DEPTH = "MAX_DEPTH";
std::string PHOTONS_PER_SAMPLE = "PHOTONS_PER_SAMPLE";
std::string PHOTON_LIMIT = "PHOTON_LIMIT";
std::string EPSILON = "EPSILON";
std::string MAX_PHOTON_SAMPLING_DISTANCE = "MAX_PHOTON_SAMPLING_DISTANCE";

std::string SHOULD_PRINT_CAUSTICS_HIT_PHOTON_MAP = "SHOULD_PRINT_CAUSTICS_HIT_PHOTON_MAP";
std::string SHOULD_PRINT_DEPTH_PHOTON_MAP = "SHOULD_PRINT_DEPTH_PHOTON_MAP";
std::string SHOULD_PRINT_HIT_PHOTON_MAP = "SHOULD_PRINT_HIT_PHOTON_MAP";

std::string GAMMA_CORRECTION = "GAMMA_CORRECTION";
