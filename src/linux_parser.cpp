#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
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

// DONE: An example of how to read data from the filesystem
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

// BONUS: Update this to use std::filesystem
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

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  string title, count, unit;
  string line;
  float memTotal, memAvailable;

  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      linestream >> title >> count >> unit;
      if (title == "MemTotal:"){ memTotal = std::stof(count); }
      if (title == "MemFree:"){ memAvailable = std::stof(count); }
    }
  }

  return (memTotal-memAvailable)/memTotal;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {

  std::ifstream stream(kProcDirectory + kUptimeFilename);
  string uptime;
  string line;

  if (stream.is_open()) {
    while (std::getline(stream, line)){
      std::istringstream linestream(line);
      linestream >> uptime;
    }
  }

  return std::stol(uptime);
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  std::ifstream fs(kProcDirectory+kStatFilename);
  string line;
  long user , nice , system , idle , iowait , irq , softirq , steal;

  while(std::getline(fs, line)){
    std::istringstream ls(line);
    string title;
    ls >> title;
    if (title == "cpu"){
      ls >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    }
  }

  long nonIdleSum{user + nice + system + irq + softirq + steal};
  long idleSum{idle + iowait};

  return nonIdleSum+idleSum;

  // user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice
  // nonIdle+idle
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) {
  // utime #14 + stime #15
  std::ifstream fs(kProcDirectory+std::to_string(pid)+"/status");
  string line;
  vector<string> out;
  while(std::getline(fs, line)){
    string s;
    std::istringstream ls(line);
    while(std::getline(ls, s, ' ')){
      out.push_back(s);
    }
  }

  long res = std::stol(out[14])+std::stol(out[15]);

  return res;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  // nonIdle

  std::ifstream fs(kProcDirectory+kStatFilename);
  string line;
  long user , nice , system , idle , iowait , irq , softirq , steal;

  while(std::getline(fs, line)){
    std::istringstream ls(line);
    string title;
    ls >> title;
    if (title == "cpu"){
      ls >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    }
  }

  long nonIdleSum{user + nice + system + irq + softirq + steal};
  [[maybe_unused]] long idleSum{idle + iowait};

  return nonIdleSum;
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  // Idle
  std::ifstream fs(kProcDirectory+kStatFilename);
  string line;
  long user , nice , system , idle , iowait , irq , softirq , steal;

  while(std::getline(fs, line)){
    std::istringstream ls(line);
    string title;
    ls >> title;
    if (title == "cpu"){
      ls >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    }
  }

  [[maybe_unused]] long nonIdleSum{user + nice + system + irq + softirq + steal};
  [[maybe_unused]] long idleSum{idle + iowait};

  return idleSum;
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  // PrevIdle = previdle + previowait
  // Idle = idle + iowait
  //
  // PrevNonIdle = prevuser + prevnice + prevsystem + previrq + prevsoftirq + prevsteal NonIdle = user + nice + system + irq + softirq + steal
  //
  // PrevTotal = PrevIdle + PrevNonIdle
  // Total = Idle + NonIdle
  //
  //# differentiate: actual value minus the previous one
  // totald = Total - PrevTotal
  // idled = Idle - PrevIdle
  //
  // CPU_Percentage = (totald - idled)/totald

  std::ifstream fs(kProcDirectory + kStatFilename);
  string line;
  float user, nice, system, idle, iowait, irq, softirq, steal;

  while (std::getline(fs, line)) {
    std::istringstream ls(line);
    string title;
    ls >> title;
    if (title == "cpu") {
      ls >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    }
  }

  float nonIdleSum{user + nice + system + irq + softirq + steal};
  float idleSum{idle + iowait};

  float util = nonIdleSum / (nonIdleSum + idleSum);

  vector<string> res;
  res.push_back(std::to_string(util));

  return res;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  // processes 10870
  // procs_running 5
  std::ifstream fs(kProcDirectory + kStatFilename);
  string line;
  [[maybe_unused]] long procs, procsRunning;

  while(std::getline(fs, line)){
    string title;
    long count;
    std::istringstream iss(line);
    iss >> title >> count;
    if (title == "processes") { procs = count; }
    if (title == "procs_running") { procsRunning = count; }

  }
  return procs;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {

  // processes 10870
  // procs_running 5
  std::ifstream fs(kProcDirectory + kStatFilename);
  string line;
  [[maybe_unused]] long procs, procsRunning;

  while(std::getline(fs, line)){
    string title;
    long count;
    std::istringstream iss(line);
    iss >> title >> count;
    if (title == "processes") { procs = count; }
    if (title == "procs_running") { procsRunning = count; }

  }
  return procsRunning;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  // /proc/pid/cmdline
  std::ifstream fs(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  string line, result;

  while (std::getline(fs, line)) {
    std::istringstream iss(line);
    iss >> result;
  }

  return result;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  // proc/pid/status
  // VmSize
  // VmSize:	  157468 kB
  string path = "/proc/" + std::to_string(pid) + "/status";
  std::ifstream f(path);
  string line;
  while (std::getline(f, line)){
    std::istringstream s(line);
    string title, usedMem;
    s >> title >> usedMem;
    if (title =="VmSize:"){
      float mb = std::stoi(usedMem) / 1024;
      return std::to_string(mb).substr(0, 5);
    }
  }
  return string();
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  // proc/pid/status
  // Uid: 1000	1000	1000	1000
   string path = "/proc/"+std::to_string(pid)+"/status";
   std::ifstream f(path);
   string line;
   while (std::getline(f, line)){
     std::istringstream s(line);
     string key, uid;
     s >> key >> uid;
     if (key == "Uid:"){ return uid; }
   }

   return "";

}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int uid) {
  // kPasswordPath
  // name:password:UID:GID:GECOS:directory:shell
  // www-data:x:33:33:www-data:/var/www:/usr/sbin/nologin

  std::ifstream f(kPasswordPath);
  for (string line; std::getline(f, line);){
//    line.replace(line.begin(), line.end(), ":", " ");
    // TODO: fix bug
    vector<string> oneUser{};
    size_t pos = 0;
    string token;
    while ((pos=line.find(":")) != std::string::npos){
      token = line.substr(0, pos);
      oneUser.push_back(token);
      line.erase(0, pos+1);
    }

    if (oneUser[2]==std::to_string(uid)) {
      return oneUser[0].substr(0,6);
    }
  }

  return std::to_string(uid);
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  // /proc/pid/stat
  // 14th element
  string path = "/proc/"+std::to_string(pid)+"/stat";
  std::ifstream f(path);
  vector<string> v{};
  string line, res;
  while(std::getline(f, line)){
    std::istringstream s(line);
    for (int i=0; i<14; i++){
      s >> res;
    }
  }
  return std::stol(res);
}
