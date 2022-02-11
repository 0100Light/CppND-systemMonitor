#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid){
  this->pid_ = pid;
  this->command = LinuxParser::Command(pid);
  this->ram = LinuxParser::Ram(pid);
  this->uptime = LinuxParser::UpTime(pid);
}

// TODO: Return this process's ID
int Process::Pid() { return this->pid_; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() {
  return (float) LinuxParser::UpTime(this->pid_)/LinuxParser::UpTime()*100;
}

// TODO: Return the command that generated this process
string Process::Command() { return this->command; }

// TODO: Return this process's memory utilization
string Process::Ram() { return this->ram; }

// TODO: Return the user (name) that generated this process
string Process::User() {
  int uid;
  try {
    uid = std::stoi(LinuxParser::Uid(this->pid_));
  } catch (std::invalid_argument &e){
    return "unknown";
  }
  this->user = LinuxParser::User(uid);
  return this->user.substr(0,6); // for better display
}

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() { return this->uptime; }

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a) const {
//  return a.pid_ < this->pid_;
  return a.ram < this->ram;
}