#ifndef SHAPE_MANAGER_HPP
#define SHAPE_MANAGER_HPP

#include <map>
#include <string>
#include <vector>
#include "circle.hpp"
#include "rectangle.hpp"
#include "shape_set.hpp"

namespace donkeev
{
  class ShapeManager
  {
  public:
    void addCircle(const std::string&, int, int, int);
    void addSet(const std::string&, const std::vector< std::string >&);

    const circle_t& getCircle(const std::string&) const;
    const shape_set_t& getSet(const std::string&) const;

    bool hasCircle(const std::string&) const;
    bool hasSet(const std::string&) const;

    rectangle_t getSetBoundingBox(const std::string&) const;

  private:
    std::map< std::string, circle_t > circles_;
    std::map< std::string, shape_set_t > sets_;
  };
}

#endif
