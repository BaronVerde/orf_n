
/**
 * Just for fun
 */

#pragma once

#include "omath/vec4.h"
#include <array>

namespace orf_n {

typedef omath::vec4 color_t;

namespace color {

static const color_t cornflowerBlue{ 0.394f, 0.609f, 0.933f, 1.0f };
static const color_t purple{ 0.625f, 0.125f, 0.937f, 1.0f };
static const color_t white{ 1.0f, 1.0f, 1.0f, 1.0f };
static const color_t pink{ 0.93f, 0.51f, 0.93f, 1.0f };
static const color_t black{ 0.0f, 0.0f, 0.0f, 1.0f };
static const color_t lightSlateGray{ 0.464f, 0.531f, 0.597f, 1.0f };
static const color_t gray{ 0.742f, 0.742f, 0.742f, 1.0f };

// Rainbow colors to visualize selected level boxes
static const color_t red{ 1.0f, 0.0f, 0.0f, 1.0f };
static const color_t orange{ 1.0f, 0.5f, 0.0f, 1.0f };
static const color_t yellow{ 1.0f, 1.0f, 0.0f, 1.0f };
static const color_t green{ 0.0f, 1.0f, 0.0f, 1.0f };
static const color_t blue{ 0.0f, 0.0f, 1.0f, 1.0f };
static const color_t indigo{ 0.29f, 0.0f, 0.51f, 1.0f };
static const color_t violet{ 0.58f, 0.0f, 0.83f, 1.0f };

static const std::array<color_t, 7> rainbow{
	violet, indigo, blue, green, yellow, orange, red
};

}

}
