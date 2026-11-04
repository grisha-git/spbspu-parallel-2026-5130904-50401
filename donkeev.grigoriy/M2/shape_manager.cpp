#include "shape_manager.hpp"
#include <stdexcept>

void donkeev::ShapeManager::addCircle(const std::string& name, int r, int x, int y)
{
  if (circles_.count(name) > 0)
  {
    throw std::invalid_argument("Circle with this name already exists");
  }
  circles_.emplace(name, circle_t(r, x, y));
}

void donkeev::ShapeManager::addSet(const std::string& name, const std::vector< std::string >& shape_names)
{
  if (sets_.count(name) > 0)
  {
    throw std::invalid_argument("Set with this name already exists");
  }

  for (const auto& shape_name : shape_names)
  {
    if (circles_.count(shape_name) == 0)
    {
      throw std::invalid_argument("Referenced shape does not exist");
    }
  }

  sets_.emplace(name, shape_set_t{shape_names});
}

const donkeev::circle_t& donkeev::ShapeManager::getCircle(const std::string& name) const
{
  auto it = circles_.find(name);
  if (it == circles_.end())
  {
    throw std::out_of_range("Circle does not exist");
  }
  return it->second;
}

const donkeev::shape_set_t& donkeev::ShapeManager::getSet(const std::string& name) const
{
  auto it = sets_.find(name);
  if (it == sets_.end())
  {
    throw std::out_of_range("Set does not exist");
  }
  return it->second;
}

bool donkeev::ShapeManager::hasCircle(const std::string& name) const
{
  return circles_.count(name) > 0;
}

bool donkeev::ShapeManager::hasSet(const std::string& name) const
{
  return sets_.count(name) > 0;
}

donkeev::rectangle_t donkeev::ShapeManager::getSetBoundingBox(const std::string& name) const
{
  auto it = sets_.find(name);
  if (it == sets_.end())
  {
    throw std::out_of_range("Set does not exist");
  }

  const auto& shape_names = it->second.shape_names;
  if (shape_names.empty())
  {
    throw std::logic_error("Set is empty");
  }

  const auto& first_circle = getCircle(shape_names[0]);
  rectangle_t bbox = first_circle.getBoundingBox();

  for (size_t i = 1; i < shape_names.size(); ++i)
  {
    const auto& circle = getCircle(shape_names[i]);
    const rectangle_t curr_bbox = circle.getBoundingBox();

    if (curr_bbox.bottom_left.x < bbox.bottom_left.x)
    {
      bbox.bottom_left.x = curr_bbox.bottom_left.x;
    }
    if (curr_bbox.bottom_left.y < bbox.bottom_left.y)
    {
      bbox.bottom_left.y = curr_bbox.bottom_left.y;
    }
    if (curr_bbox.top_right.x > bbox.top_right.x)
    {
      bbox.top_right.x = curr_bbox.top_right.x;
    }
    if (curr_bbox.top_right.y > bbox.top_right.y)
    {
      bbox.top_right.y = curr_bbox.top_right.y;
    }
  }

  return bbox;
}
