#pragma once

#include <vector>

#include "engine/core/math.h"

namespace spacewar::app::shapes {

inline const std::vector<se::Vec2> kNeedle{
    {1.6f, 0.0f}, {-1.0f, 0.32f}, {-0.7f, 0.0f}, {-1.0f, -0.32f}};

inline const std::vector<se::Vec2> kWedge{
    {1.2f, 0.0f}, {-0.9f, 0.70f}, {-0.5f, 0.0f}, {-0.9f, -0.70f}};

inline const std::vector<se::Vec2> kNeedleFlame{
    {0.0f, 0.24f}, {-1.3f, 0.0f}, {0.0f, -0.24f}};

inline const std::vector<se::Vec2> kWedgeFlame{
    {0.0f, 0.40f}, {-1.5f, 0.0f}, {0.0f, -0.40f}};

}  // namespace spacewar::app::shapes
