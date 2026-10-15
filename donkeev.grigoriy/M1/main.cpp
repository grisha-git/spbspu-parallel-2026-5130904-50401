#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <string>
#include <exception>

namespace donkeev
{
  struct Point
  {
    double x;
    double y;
  };

  struct Shape
  {
    double a;
    double b;
    Point center;
  };

  struct Bounds
  {
    double min_x, max_x, min_y, max_y;
  };

  struct Result
  {
    long long in_union = 0;
    long long in_intersection = 0;
  };

  using Circles = std::vector< Shape >;

  bool contains(const Shape&, const Point&);
  bool readShapes(std::vector< Shape >&);
  Bounds computeBounds(const std::vector< Shape >& circles);
  Result worker(int id, long long my_tries, const Circles& circles, const Bounds& b, long long seed);
}

int main(int argc, char** argv)
{
  if (argc < 3 || argc > 4)
  {
    std::cerr << "Invalid arguments\n";
    return 1;
  }

  int threads = 0;
  long long tries = 0, seed = 0;
  try
  {
    threads = std::stoi(argv[1]);
    tries = std::stoll(argv[2]);
    seed = (argc > 3) ? std::stoi(argv[3]) : 0;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Arguments error " << e.what() << std::endl;
    return 1;
  }

  if (threads < 0 || tries <= 0 || seed < 0)
  {
    std::cerr << "Threads, tries and seed must be positive\n";
    return 1;
  }
  threads = threads > 0 ? threads : 1;

  std::vector< donkeev::Shape > shapes;
  if (!donkeev::readShapes(shapes))
  {
    return 1;
  }

  donkeev::Bounds bounds = donkeev::computeBounds(shapes);

  std::vector< std::thread > pool;
  std::vector< donkeev::Result > results(threads);

  const long long base = tries / threads;
  const long long rem = tries % threads;

  for (long long i = 0; i < threads; ++i)
  {
    const long long my_tries = base + (i < rem ? 1 : 0);
    pool.emplace_back(
        [&, i, my_tries]
        {
          results[i] = worker(static_cast< int >(i), my_tries, shapes, bounds, seed);
        });
  }

  for (auto& t : pool)
  {
    t.join();
  }

  long long total_union = 0;
  long long total_intersection = 0;
  for (const auto& r : results)
  {
    total_union += r.in_union;
    total_intersection += r.in_intersection;
  }

  const double rect_area = (bounds.max_x - bounds.min_x) * (bounds.max_y - bounds.min_y);

  const double area_union = rect_area * double(total_union) / double(tries);
  const double area_intersection = rect_area * double(total_intersection) / double(tries);

  std::cout << std::setprecision(std::numeric_limits< double >::max_digits10);
  std::cout << area_union << " " << area_intersection << '\n';

  return 0;
}

bool donkeev::contains(const Shape& s, const Point& p)
{
  const double dx = (p.x - s.center.x) / s.a;
  const double dy = (p.y - s.center.y) / s.b;
  return dx * dx + dy * dy <= 1.0;
}

bool donkeev::readShapes(std::vector< Shape >& out)
{
  while (true)
  {
    double x = 0.0;
    double y = 0.0;
    double first = 0.0;
    double second = 0.0;
    if (std::cin >> first >> second >> x >> y)
    {
      if (second == 0.0)
      {
        out.push_back({first, first, {x, y}});
      }
      else
      {
        out.push_back({first, second, {x, y}});
      }
    }
    else if (std::cin.eof())
    {
      break;
    }
    else
    {
      std::cerr << "Invalid circle parametrs\n";
      return false;
    }
  }

  if (out.empty())
  {
    std::cerr << "No circles\n";
    return false;
  }

  return true;
}

donkeev::Bounds donkeev::computeBounds(const std::vector< Shape >& shapes)
{
  double min_x = shapes[0].center.x - shapes[0].a;
  double max_x = shapes[0].center.x + shapes[0].a;
  double min_y = shapes[0].center.y - shapes[0].b;
  double max_y = shapes[0].center.y + shapes[0].b;

  for (const auto& s : shapes)
  {
    const double cx = double(s.center.x);
    const double cy = double(s.center.y);
    min_x = std::min(min_x, cx - s.a);
    max_x = std::max(max_x, cx + s.a);
    min_y = std::min(min_y, cy - s.b);
    max_y = std::max(max_y, cy + s.b);
  }
  return {min_x, max_x, min_y, max_y};
}

donkeev::Result donkeev::worker(int id, long long my_tries, const Circles& circles, const Bounds& b, long long seed)
{
  std::default_random_engine gen(seed + unsigned(id));

  std::uniform_real_distribution< double > dist_x(b.min_x, b.max_x);
  std::uniform_real_distribution< double > dist_y(b.min_y, b.max_y);

  Result res;
  for (long long i = 0; i < my_tries; ++i)
  {
    const double x = dist_x(gen);
    const double y = dist_y(gen);

    bool in_any = false;
    bool in_all = true;
    for (const auto& c : circles)
    {
      const bool inside = contains(c, {x, y});
      in_any = in_any || inside;
      in_all = in_all && inside;
    }

    if (in_any)
    {
      res.in_union++;
    }
    if (in_all)
    {
      res.in_intersection++;
    }
  }
  return res;
}
