#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <sys/types.h>

namespace donkeev
{
  struct Process
  {
    pid_t pid;
    int pipe_to_child[2];
    int pipe_from_child[2];
  };
}

#endif
