// Copyright 2021 Timoshenko Yulia timoshenkojulie01@gmail.com
#include "builder.hpp"
void Builder::execute_cmake(std::list<std::string>& args) {
  if (status != 0) {
    done = true;
    return;
  }
  boost::process::ipstream pipe_stream;
  process = std::make_unique<boost::process::child>(boost::process::child{
      boost::process::search_path("cmake"), boost::process::args(args),
      boost::process::std_out > pipe_stream});

  for (std::string line; process->running() && std::getline(pipe_stream, line);)
    std::cout << line << std::endl;
  process->wait();
  status = process->exit_code();
}

std::chrono::seconds seconds_now() {
  return std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::system_clock::now().time_since_epoch());
}
void Builder::timeout(int seconds) {
  auto start = seconds_now();
  while (!done and ((seconds_now() - start).count() < seconds)) {
    std::this_thread::yield();
  }
  process->terminate();
  throw std::runtime_error("время ожидания истекло");
}
void Builder::build(const std::string& config, int timeout, bool b_install,
                    bool b_pack) {
  async::spawn([&, this] { this->timeout(timeout); });
  auto make = async::spawn([&, this]() {
    std::list<std::string> args{"-H.", "-B_builds",
                                "-DCMAKE_INSTALL_PREFIX=_install",
                                "-DCMAKE_BUILD_TYPE=" + config};
    this->execute_cmake(args);
  });
  auto build = make.then([&, this]() {
    std::list<std::string> args{"--build", "_builds"};
    this->execute_cmake(args);
  });
  auto install = build.then([&, this]() {
    if (b_install) {
      std::list<std::string> args{"--build", "_builds", "--target", "install"};
      this->execute_cmake(args);
    }
  });
  auto pack = install.then([&]() {
    if (b_pack) {
      if (b_install) {
        std::list<std::string> args{"--build", "_builds", "--target",
                                    "package"};
        this->execute_cmake(args);
      }
    }
    done = true;
  });
  pack.get();
}
