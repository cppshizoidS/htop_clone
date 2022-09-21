#include "linux_parser.h"

#include <bits/stdc++.h>  //for std::stof
#include <dirent.h>
#include <math.h>
#include <unistd.h>

#include <iostream>
#include <numeric>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// I implemented: Read and return the system memory utilization
// -------------------------------------
float LinuxParser::MemoryUtilization() {
  string line;
  string key;
  float value;  // converting string into float at the same time using
                // linestream
  float TotalMem;
  float FreeMem;
  float UsedMem;
  string UsedMem_string;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {  // MemTotal:    7918692 KB
      std::replace(
          line.begin(), line.end(), ' ',
          '_');  // MemTotal:_____7918692_KB, can not use "", only use ''
      std::replace(line.begin(), line.end(), ':',
                   ' ');                       // MemTotal _____7918692_KB
      line.erase(line.end() - 3, line.end());  // MemTotal _____7918692
      line.erase(std::remove(line.begin(), line.end(), '_'),
                 line.end());  // MemTotal 7918692
      std::stringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal") {
          TotalMem = value;
        }
        if (key == "MemFree") {
          FreeMem = value;
        }
      }
    }
  }
  UsedMem = TotalMem - FreeMem;
  UsedMem = UsedMem / TotalMem;
  return UsedMem;
}

// I implemented : Read and return the system uptime
// ---------------------------------
long LinuxParser::UpTime() {
  string line;
  long int TotalUpTime;  // converting string into long int at the same time
                         // using linestream
  long int IdleTime;
  long int UpTime;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    while (
        std::getline(filestream, line)) {  // 13145.68 43993.58 - Up time of the
                                           // system, idle time of the system
      std::stringstream linestream(line);
      linestream >> TotalUpTime >> IdleTime;
    }
    UpTime = TotalUpTime - IdleTime;
  }
  return UpTime;
}

// I implemented:Read and return the number of jiffies for the system
// -----------------------------------
long LinuxParser::Jiffies() {
  vector<int long> CpuLoad;
  long int jiffies;
  CpuLoad = LinuxParser::CpuUtilization();
  jiffies = std::accumulate(CpuLoad.begin(), CpuLoad.end(), 0);
  return jiffies;
}

// I implemented: Read and return the number of active jiffies for a PID
// ----------------------
long LinuxParser::ActiveJiffies(int pid) {
  string pidString;
  pidString = std::to_string(pid);
  string line;
  string value;
  long int activeJiffies;
  vector<string> statFile;
  std::ifstream filestream(kProcDirectory + pidString + kStatFilename);
  if (filestream.is_open()) {
    getline(filestream, line);
    std::stringstream linestream(line);
    while (linestream >> value) {
      statFile.push_back(value);
    }
  }
  activeJiffies = (std::stol(statFile[13]) + std::stol(statFile[14]) +
                   std::stol(statFile[15]) + std::stol(statFile[16]));
  // nth element - its description, 14th - uTime, 15th - sTime, 16th - cuTime,
  // 17th - csTime, all in clock ticks and converted into seconds with sysconf
  return activeJiffies;
}

// I implemented : Read and return the number of active jiffies for the system
// -----------------------------------
long LinuxParser::ActiveJiffies() {
  vector<long int> CpuLoad;
  CpuLoad = LinuxParser::CpuUtilization();
  long int active_jiffies;
  active_jiffies =
      CpuLoad[LinuxParser::kUser_] + CpuLoad[LinuxParser::kNice_] +
      CpuLoad[LinuxParser::kSystem_] + CpuLoad[LinuxParser::kIRQ_] +
      CpuLoad[LinuxParser::kSoftIRQ_] + CpuLoad[LinuxParser::kSteal_];
  return active_jiffies;
}

// I implemented: Read and return the number of idle jiffies for the
// system------------------------------------
long LinuxParser::IdleJiffies() {
  vector<long int> CpuLoad;
  CpuLoad = LinuxParser::CpuUtilization();
  long int idle_jiffies;
  idle_jiffies = CpuLoad[LinuxParser::kIdle_] + CpuLoad[LinuxParser::kIOwait_];
  return idle_jiffies;
}

// I Implemented : Read and return CPU utilization
// --------------------------------
vector<long int> LinuxParser::CpuUtilization() {
  string line;
  long int value;
  string title_of_line;
  vector<long int> CpuLoad;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::stringstream linestream(line);
      linestream >> title_of_line;
      if (title_of_line == "cpu") {
        for (int i = 0; i < 10; i++) {
          linestream >> value;
          CpuLoad.push_back(value);
        }
        return CpuLoad;
      }
    }
  }
  return CpuLoad;
}

// I implemented : Read and return the total number of processes
// -----------------------------------
int LinuxParser::TotalProcesses() {
  string line;
  int totalProcesses;
  string title_of_line;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::stringstream linestream(line);
      linestream >> title_of_line;
      if (title_of_line == "processes") {
        linestream >> totalProcesses;
        return totalProcesses;
      }
    }
  }
  return totalProcesses;
}

int LinuxParser::RunningProcesses() {
  string line;
  int runningProcesses;
  string title_of_line;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::stringstream linestream(line);
      linestream >> title_of_line;
      if (title_of_line == "procs_running") {
        linestream >> runningProcesses;
        return runningProcesses;
      }
    }
  }
  return runningProcesses;
}

string LinuxParser::Command(int pid) {
  string pidString;
  pidString = std::to_string(pid);
  string line;
  string commandLine;
  string title_of_line;
  std::ifstream filestream(kProcDirectory + pidString + kCmdlineFilename);
  if (filestream.is_open()) {
    getline(filestream, commandLine);
    return commandLine;
  }
  return commandLine;
}

string LinuxParser::Ram(int pid) {
  string pidString;
  pidString = std::to_string(pid);
  string line;
  string RamString;
  long int UsedRamKB;
  long int UsedRamMB;
  string Ram;
  string title_of_line;
  std::ifstream filestream(kProcDirectory + pidString + kStatusFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ',
                   '_');  // vmSize:_____7918692_KB, can not use "", only use ''
      std::replace(line.begin(), line.end(), ':',
                   ' ');                       // vmSize _____7918692_KB
      line.erase(line.end() - 3, line.end());  // vmSize _____7918692
      line.erase(std::remove(line.begin(), line.end(), '_'),
                 line.end());  // vmSize 7918692
      std::stringstream linestream(line);
      linestream >> title_of_line >> UsedRamKB;
      if (title_of_line == "VmSize") {
        UsedRamMB = UsedRamKB / 1024;
        // UsedRamMB = roundf(UsedRamMB * 100)/100;
        Ram = std::to_string(UsedRamMB);
        return Ram;
      }
    }
  }
  return Ram;
}

string LinuxParser::Uid(int pid) {
  string pidString;
  pidString = std::to_string(pid);
  string line;
  string UserID;
  string title_of_line;
  std::ifstream filestream(kProcDirectory + pidString + kStatusFilename);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::stringstream linestream(line);
      linestream >> title_of_line >> UserID;
      if (title_of_line == "Uid:") {
        return UserID;
      }
    }
  }
  return UserID;
}

string LinuxParser::User(int pid) {
  string line;
  int FirstOccur;  // to find first occurance of ":"
  string Uid = LinuxParser::Uid(pid);
  int SecondOccur;          // to find second occurance of ":"
  int ThirdOccur;           // to find third occurance of ":"
  string pid_ID;            // to store extracted pid_ID from line
  int FirstLetterPID;       // Index of first letter of PID ID
  int LengthPID;            // Length of PID interm of character
  char separator[] = ":";   // defined separator within the line
  string user_for_process;  // user associated with process
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      FirstOccur = line.find(separator);  // index of first occurance of ":"
      SecondOccur = line.find(
          separator, FirstOccur + 1);  // index of second occurance of ":"
      ThirdOccur = line.find(
          separator, SecondOccur + 1);  // index of third occurance of ":"
      FirstLetterPID = SecondOccur + 1;
      LengthPID = ThirdOccur - SecondOccur - 1;
      pid_ID =
          line.substr(FirstLetterPID, LengthPID);  // extracting pid_ID of line
      if (pid_ID ==
          Uid) {  // comparing pid_ID with userID of pid we are interested in
        user_for_process = line.substr(
            0,
            FirstOccur);  // if a match, returning user associated with process
        return user_for_process;
      }
    }
  }
  return user_for_process;
}

long LinuxParser::UpTime(int pid) {
  string pidString;
  pidString = std::to_string(pid);
  string line;
  string value;
  long int upTime;
  vector<string> statFile;
  std::ifstream filestream(kProcDirectory + pidString + kStatFilename);
  if (filestream.is_open()) {
    getline(filestream, line);
    std::stringstream linestream(line);
    while (linestream >> value) {
      statFile.push_back(value);
    }
    upTime = std::stol(statFile[21]) /
             sysconf(_SC_CLK_TCK);  // 22th element in the statFile vector is
                                    // uptime clock ticks
  }
  return upTime;
  // converting clockticks in to seconds at the same time
}
