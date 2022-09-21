#include "system.h"

#include <unistd.h>

#include <algorithm>  //for sorting the vector
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

// I implemented: Return the system's CPU-----------------
Processor& System::Cpu() { return cpu_; }

// I implemented : Return a container composed of the system's processes
vector<Process>& System::Processes() {
  vector<int> ListofPIDs = LinuxParser::Pids();
  processes_.clear();  // clearing any element leftover from last call of the
                       // vector to stop seg error!
  for (const int pid : ListofPIDs) {  // creating new Process object everytime
                                      // with set pid value
    processes_.emplace_back(pid);
  }
  std::sort(processes_.begin(), processes_.end());
  return processes_;
}

std::string System::Kernel() { return LinuxParser::Kernel(); }
float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

std::string System::OperatingSystem() { return LinuxParser::OperatingSystem(); }


int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }


int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }


long int System::UpTime() { return LinuxParser::UpTime(); }
