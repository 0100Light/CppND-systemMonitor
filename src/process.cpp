#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include "linux_parser.h"
#include "process.h"

using namespace std;

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
  long uptime = LinuxParser::UpTime();

  string path = "/proc/"+std::to_string(this->pid_)+"/stat";
  std::ifstream f(path);

  std::string line;
  vector<string> vec;
  while(std::getline(f, line)){
    istringstream s(line);
    string item;
    while ( s>>item ){
      vec.push_back(item);
    }
  }
  f.close();

  // total time
  // #14 utime - CPU time spent in user code, measured in clock ticks
  // #15 stime - CPU time spent in kernel code, measured in clock ticks
  // #16 cutime - Waited-for children's CPU time spent in user code (in clock ticks)
  // #17 cstime - Waited-for children's CPU time spent in kernel code (in clock ticks)
  // #22 starttime - Time when the process started, measured in clock ticks

  float totalTime = stof(vec[13])+stof(vec[14])+stof(vec[15])+stof(vec[16]);
  float startTime = stof(vec[21]);

//  cout << to_string(totalTime)+", "+to_string(startTime) << "\n";

//  Hertz = 100;
//  seconds = uptime - (starttime / Hertz);
//  cpu_usage = 100 * ((total_time / Hertz) / seconds);

  float sec = (float)uptime -((float)startTime / 100);
  float cpuUtil = 100*((float)totalTime / 100) / sec;

  return cpuUtil;
}



// TODO: Return the command that generated this process
string Process::Command() {
  if (this->command.length()>451){
    return this->command.substr(0, 45) + "...";
  } else {
    return this->command;
  }
}

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