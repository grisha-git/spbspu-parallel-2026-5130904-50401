#include "process_manager.hpp"
#include <stdexcept>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <cerrno>
#include <random>
#include <chrono>
#include <thread>
#include <iostream>
#include <iomanip>

namespace
{
  constexpr int kRngShift = 11;
  constexpr double kInv = 1.0 / 9007199254740992.0;
}

bool donkeev::writeAll(int fd, const void* buffer, size_t count)
{
  const char* const ptr = static_cast< const char* >(buffer);
  size_t total_written = 0;
  while (total_written < count)
  {
    const ssize_t written = write(fd, ptr + total_written, count - total_written);
    if (written < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }
      return false;
    }
    total_written += static_cast< size_t >(written);
  }
  return true;
}

bool donkeev::readAll(int fd, void* buffer, size_t count)
{
  char* const ptr = static_cast< char* >(buffer);
  size_t total_read = 0;
  while (total_read < count)
  {
    const ssize_t bytes_read = read(fd, ptr + total_read, count - total_read);
    if (bytes_read < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }
      return false;
    }
    if (bytes_read == 0)
    {
      return false;
    }
    total_read += static_cast< size_t >(bytes_read);
  }
  return true;
}

void donkeev::monteCarloWorker(const std::vector< donkeev::circle_t >& circles, const donkeev::rectangle_t& bbox,
    size_t thread_tries, size_t thread_seed, size_t& out_hits)
{
  std::vector< donkeev::FlatCircle > flat_circles;
  flat_circles.reserve(circles.size());
  for (const auto& c : circles)
  {
    auto center = c.getCenter();
    const double r = c.getRadius();

    donkeev::FlatCircle fc;
    fc.cx = center.x;
    fc.cy = center.y;
    fc.r2 = r * r;

    flat_circles.push_back(fc);
  }

  std::mt19937_64 rng(thread_seed);

  const double min_x = bbox.bottom_left.x;
  const double max_x = bbox.top_right.x;
  const double min_y = bbox.bottom_left.y;
  const double max_y = bbox.top_right.y;

  const double range_x = max_x - min_x;
  const double range_y = max_y - min_y;

  size_t local_hits = 0;
  for (size_t i = 0; i < thread_tries; ++i)
  {
    const double px = min_x + (rng() >> kRngShift) * kInv * range_x;
    const double py = min_y + (rng() >> kRngShift) * kInv * range_y;

    for (const auto& fc : flat_circles)
    {
      const double dx = px - fc.cx;
      const double dy = py - fc.cy;
      if (dx * dx + dy * dy <= fc.r2)
      {
        ++local_hits;
        break;
      }
    }
  }
  out_hits = local_hits;
}

void donkeev::runChildLoop(const int read_fd, const int write_fd, size_t seed)
{
  std::mt19937_64 master_rng(seed);

  while (true)
  {
    size_t num_circles = 0;
    if (!readAll(read_fd, &num_circles, sizeof(num_circles)))
    {
      break;
    }

    std::vector< donkeev::circle_t > circles;
    circles.reserve(num_circles);
    for (size_t i = 0; i < num_circles; ++i)
    {
      RawCircleData raw{};
      if (!readAll(read_fd, &raw, sizeof(raw)))
      {
        return;
      }
      circles.emplace_back(raw.r, raw.x, raw.y);
    }

    size_t threads_count = 0;
    size_t tries = 0;
    if (!readAll(read_fd, &threads_count, sizeof(threads_count)) || !readAll(read_fd, &tries, sizeof(tries)))
    {
      return;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    donkeev::rectangle_t bbox = circles[0].getBoundingBox();
    for (size_t i = 1; i < circles.size(); ++i)
    {
      const donkeev::rectangle_t curr = circles[i].getBoundingBox();
      if (curr.bottom_left.x < bbox.bottom_left.x)
      {
        bbox.bottom_left.x = curr.bottom_left.x;
      }
      if (curr.bottom_left.y < bbox.bottom_left.y)
      {
        bbox.bottom_left.y = curr.bottom_left.y;
      }
      if (curr.top_right.x > bbox.top_right.x)
      {
        bbox.top_right.x = curr.top_right.x;
      }
      if (curr.top_right.y > bbox.top_right.y)
      {
        bbox.top_right.y = curr.top_right.y;
      }
    }

    const double width = static_cast< double >(bbox.top_right.x - bbox.bottom_left.x);
    const double height = static_cast< double >(bbox.top_right.y - bbox.bottom_left.y);
    const double box_area = width * height;

    std::vector< std::thread > workers;
    std::vector< size_t > hits_per_thread(threads_count, 0);
    workers.reserve(threads_count);

    const size_t base_tries = tries / threads_count;
    const size_t remainder = tries % threads_count;

    for (size_t i = 0; i < threads_count; ++i)
    {
      size_t thread_tries = base_tries + (i < remainder ? 1 : 0);
      size_t thread_seed = master_rng();
      workers.emplace_back(monteCarloWorker, std::cref(circles), std::cref(bbox), thread_tries, thread_seed,
          std::ref(hits_per_thread[i]));
    }

    for (auto& th : workers)
    {
      th.join();
    }

    size_t total_hits = 0;
    for (const size_t h : hits_per_thread)
    {
      total_hits += h;
    }

    const double area = box_area * (static_cast< double >(total_hits) / static_cast< double >(tries));

    auto end_time = std::chrono::high_resolution_clock::now();
    const double time_ms = std::chrono::duration< double, std::milli >(end_time - start_time).count();

    RawResultData result{area, time_ms};
    if (!writeAll(write_fd, &result, sizeof(result)))
    {
      return;
    }
  }
}

void donkeev::closeProcess(Process& proc)
{
  if (proc.pipe_to_child[0] != -1)
  {
    close(proc.pipe_to_child[0]);
    proc.pipe_to_child[0] = -1;
  }
  if (proc.pipe_to_child[1] != -1)
  {
    close(proc.pipe_to_child[1]);
    proc.pipe_to_child[1] = -1;
  }
  if (proc.pipe_from_child[0] != -1)
  {
    close(proc.pipe_from_child[0]);
    proc.pipe_from_child[0] = -1;
  }
  if (proc.pipe_from_child[1] != -1)
  {
    close(proc.pipe_from_child[1]);
    proc.pipe_from_child[1] = -1;
  }
}

donkeev::Process donkeev::spawnProcess(size_t seed, const std::vector< Process >& existing_processes)
{
  Process proc{-1, {-1, -1}, {-1, -1}};

  if (pipe(proc.pipe_to_child) == -1)
  {
    throw std::runtime_error("Failed to create pipe_to_child");
  }

  if (pipe(proc.pipe_from_child) == -1)
  {
    closeProcess(proc);
    throw std::runtime_error("Failed to create pipe_from_child");
  }

  const pid_t pid = fork();
  if (pid < 0)
  {
    closeProcess(proc);
    throw std::runtime_error("Failed to fork process");
  }

  if (pid == 0)
  {
    for (auto existing : existing_processes)
    {
      closeProcess(existing);
    }

    close(proc.pipe_to_child[1]);
    close(proc.pipe_from_child[0]);

    runChildLoop(proc.pipe_to_child[0], proc.pipe_from_child[1], seed);

    close(proc.pipe_to_child[0]);
    close(proc.pipe_from_child[1]);
    _exit(0);
  }

  proc.pid = pid;
  close(proc.pipe_to_child[0]);
  proc.pipe_to_child[0] = -1;
  close(proc.pipe_from_child[1]);
  proc.pipe_from_child[1] = -1;

  return proc;
}

donkeev::ProcessManager::~ProcessManager()
{
  for (auto& pair : processes_)
  {
    closeProcess(pair.second);
    if (pair.second.pid > 0)
    {
      waitpid(pair.second.pid, nullptr, 0);
    }
  }
}

void donkeev::ProcessManager::addProcess(const std::string& name, const Process& proc)
{
  processes_.emplace(name, proc);
}

bool donkeev::ProcessManager::hasProcess(const std::string& name) const
{
  return processes_.count(name) > 0;
}

bool donkeev::ProcessManager::hasCalculation(const std::string& calc_name) const
{
  return calculations_.count(calc_name) > 0;
}

std::vector< donkeev::Process > donkeev::ProcessManager::getAllProcesses() const
{
  std::vector< Process > result;
  result.reserve(processes_.size());
  for (const auto& entry : processes_)
  {
    result.push_back(entry.second);
  }
  return result;
}

void donkeev::ProcessManager::startCalculation(const std::string& process_name, const std::string& calc_name,
    const std::string& set_name, const ShapeManager& shape_manager, size_t threads, size_t tries)
{
  if (calculations_.count(calc_name) > 0)
  {
    throw std::invalid_argument("Calculation with this name already exists");
  }

  auto proc_it = processes_.find(process_name);
  if (proc_it == processes_.end())
  {
    throw std::invalid_argument("Process does not exist");
  }

  const auto& shape_set = shape_manager.getSet(set_name);
  const auto& shape_names = shape_set.shape_names;
  size_t num_circles = shape_names.size();

  const int write_fd = proc_it->second.pipe_to_child[1];

  if (!writeAll(write_fd, &num_circles, sizeof(num_circles)))
  {
    throw std::runtime_error("Failed to write to process pipe");
  }

  for (const auto& s_name : shape_names)
  {
    const auto& circle = shape_manager.getCircle(s_name);
    const point_t center = circle.getCenter();
    RawCircleData raw{circle.getRadius(), center.x, center.y};
    if (!writeAll(write_fd, &raw, sizeof(raw)))
    {
      throw std::runtime_error("Failed to write circle data");
    }
  }

  if (!writeAll(write_fd, &threads, sizeof(threads)) || !writeAll(write_fd, &tries, sizeof(tries)))
  {
    throw std::runtime_error("Failed to write calculation parameters");
  }

  calculations_.emplace(calc_name, CalculationInfo{process_name, false, 0.0, 0.0});
  pending_calcs_[process_name].push(calc_name);
}

void donkeev::ProcessManager::readOneResultBlocking(const std::string& process_name)
{
  auto proc_it = processes_.find(process_name);
  auto& queue = pending_calcs_[process_name];
  if (proc_it == processes_.end() || queue.empty())
  {
    return;
  }

  RawResultData raw{};
  const int read_fd = proc_it->second.pipe_from_child[0];
  if (!readAll(read_fd, &raw, sizeof(raw)))
  {
    throw std::runtime_error("Failed to read calculation result from child");
  }

  const std::string finished_calc_name = queue.front();
  queue.pop();

  auto calc_it = calculations_.find(finished_calc_name);
  if (calc_it != calculations_.end())
  {
    calc_it->second.completed = true;
    calc_it->second.area = raw.area;
    calc_it->second.time_ms = raw.time_ms;
  }
}

void donkeev::ProcessManager::pollProcessResults(const std::string& process_name)
{
  auto proc_it = processes_.find(process_name);
  auto& queue = pending_calcs_[process_name];
  if (proc_it == processes_.end())
  {
    return;
  }

  const int read_fd = proc_it->second.pipe_from_child[0];

  while (!queue.empty())
  {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(read_fd, &read_set);

    timeval timeout{0, 0};
    const int ready = select(read_fd + 1, &read_set, nullptr, nullptr, &timeout);
    if (ready > 0 && FD_ISSET(read_fd, &read_set))
    {
      readOneResultBlocking(process_name);
    }
    else
    {
      break;
    }
  }
}

void donkeev::ProcessManager::printStatus(const std::string& calc_name, std::ostream& out)
{
  auto it = calculations_.find(calc_name);
  if (it == calculations_.end())
  {
    throw std::invalid_argument("Calculation was not started");
  }

  if (!it->second.completed)
  {
    pollProcessResults(it->second.process_name);
  }

  if (!it->second.completed)
  {
    out << it->second.process_name << " <IN PROGRESS>\n";
  }
  else
  {
    const std::ios_base::fmtflags old_flags = out.flags();
    const std::streamsize old_prec = out.precision();
    out << std::fixed << std::setprecision(3) << it->second.process_name << " " << it->second.time_ms << " "
        << it->second.area << "\n";
    out.flags(old_flags);
    out.precision(old_prec);
  }
}

void donkeev::ProcessManager::waitCalculation(const std::string& calc_name, std::ostream& out)
{
  auto it = calculations_.find(calc_name);
  if (it == calculations_.end())
  {
    throw std::invalid_argument("Calculation was not started");
  }

  while (!it->second.completed)
  {
    readOneResultBlocking(it->second.process_name);
  }

  const std::ios_base::fmtflags old_flags = out.flags();
  const std::streamsize old_prec = out.precision();
  out << std::fixed << std::setprecision(3) << it->second.process_name << " " << it->second.time_ms << " "
      << it->second.area << "\n";
  out.flags(old_flags);
  out.precision(old_prec);
}
