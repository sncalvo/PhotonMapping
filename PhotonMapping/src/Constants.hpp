#pragma once

constexpr auto EPSILON = 0.00001f;
constexpr auto MAX_DEPTH = 5;
constexpr auto PHOTONS_PER_SAMPLE = 100;
constexpr auto MAX_PHOTON_SAMPLING_DISTANCE = 0.5f;
constexpr unsigned int PHOTON_LIMIT = 50000;

constexpr auto IMAGE_WIDTH = 512;
constexpr auto IMAGE_HEIGHT = 512;

constexpr auto SHOULD_PRINT_DIFFUSE_PHOTON_MAP = false;
constexpr auto SHOULD_PRINT_DEPTH_PHOTON_MAP = false;
constexpr auto SHOULD_PRINT_HIT_PHOTON_MAP = false;
