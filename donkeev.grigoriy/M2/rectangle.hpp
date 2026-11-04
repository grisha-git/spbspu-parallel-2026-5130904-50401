#ifndef RECTANGLE_HPP
#define RECTANGLE_HPP

#include "point.hpp"

namespace donkeev
{
  struct rectangle_t
  {
    point_t bottom_left;
    point_t top_right;
  };
}

#endif
