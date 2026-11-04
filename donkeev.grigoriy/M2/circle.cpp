#include "circle.hpp"
#include <stdexcept>

donkeev::circle_t::circle_t():
  r_(1),
  center_{0, 0}
{}

donkeev::circle_t::circle_t(int r, int x, int y):
  r_(r),
  center_{x, y}
{
  if (r <= 0)
  {
    throw std::invalid_argument("Radius must be positive");
  }
}

int donkeev::circle_t::getRadius() const
{
  return r_;
}

donkeev::point_t donkeev::circle_t::getCenter() const
{
  return center_;
}

donkeev::rectangle_t donkeev::circle_t::getBoundingBox() const
{
  return rectangle_t{point_t{center_.x - r_, center_.y - r_}, point_t{center_.x + r_, center_.y + r_}};
}
