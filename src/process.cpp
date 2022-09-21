#include "process.h"

#include <bits/stdc++.h>
#include <math.h>
#include <unistd.h>

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

int Process::Pid() { return this->PID_; }

float Process::CpuUtilization() {
  float uptime = LinuxParser::UpTime(PID_);
  float ActiveJiffSecs = LinuxParser::ActiveJiffies(PID_);
  float Sysuptime = LinuxParser::UpTime();
  float seconds = (Sysuptime - uptime);
  float CPU_Use;
  if (seconds > 0) CPU_Use = ActiveJiffSecs / (seconds * sysconf(_SC_CLK_TCK));
  return CPU_Use;
}

string Process::Command() { return LinuxParser::Command(PID_); }

string Process::Ram() {
  string ram = LinuxParser::Ram(PID_);
  std::stringstream ramstream(ram);
  ramstream >> ram_;  // storing value of Ram into private float variable ram_
  return ram;
}

string Process::User() { return LinuxParser::User(PID_); }

long int Process::UpTime() { return LinuxParser::UpTime(PID_); }

bool Process::operator<(Process const& a) const {
  return (a.ram_ > this->ram_);
}
