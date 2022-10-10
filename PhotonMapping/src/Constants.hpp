#pragma once

#include <unordered_map>
#include <iostream>

extern std::unordered_map<std::string, bool> BOOL_CONSTANTS;
extern std::unordered_map<std::string, float> FLOAT_CONSTANTS;
extern std::unordered_map<std::string, int> INT_CONSTANTS;
extern std::string WIDTH;
extern std::string HEIGHT;

extern std::string MAX_DEPTH;
extern std::string PHOTONS_PER_SAMPLE;
extern std::string PHOTON_LIMIT;
extern std::string EPSILON;
extern std::string MAX_PHOTON_SAMPLING_DISTANCE;

extern std::string SHOULD_PRINT_CAUSTICS_HIT_PHOTON_MAP;
extern std::string SHOULD_PRINT_DEPTH_PHOTON_MAP;
extern std::string SHOULD_PRINT_HIT_PHOTON_MAP;
