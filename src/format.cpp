#include <string>
#include <sstream>

#include "format.h"

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) {
  int s, m, h;
  long input{seconds};

  s = seconds % 60;
  input = (input-s)/60; // minutes
  m = input % 60;
  h = (input-m) / 60; // hours

  string output;
  output = ZeroPadNumber(h) + ":" + ZeroPadNumber(m) + ":" + ZeroPadNumber(s);

  return output;
}

string Format::ZeroPadNumber(int num)
{
  std::stringstream ss;

  // the number is converted to string with the help of stringstream
  ss << num;
  string ret;
  ss >> ret;

  // Append zero chars
  int str_length = ret.length();
  for (int i = 0; i < 2 - str_length; i++)
    ret = "0" + ret;
  return ret;
}