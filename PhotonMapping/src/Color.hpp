#pragma once

#include "FreeImage/FreeImage.h"

struct Color {
    unsigned char r, g, b, a;

    RGBQUAD toRGBQuad() {
        return RGBQUAD {
            r, g, b, a
        };
    };
};
