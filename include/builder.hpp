// Copyright 2021 Timoshenko Yulia timoshenkojulie01@gmail.com
#include <async++.h>

#include <atomic>
#include <boost/iostreams/stream.hpp>
#include <boost/process.hpp>
#include <boost/process/initializers.hpp>
#include <chrono>
#include <iostream>
#include <vector>

#ifndef INCLUDE_BUILDER_HPP
#define INCLUDE_BUILDER_HPP
#include <string>

class Builder {
 public:
  void build(const std::string&, int, bool, bool);

 private:
  int status = 0;
  std::unique_ptr<boost::process::child> process;
  void execute_cmake(std::list<std::string>& args);
  void timeout(int);
  bool done = false;
};
#endif  // INCLUDE_BUILDER_HPP
