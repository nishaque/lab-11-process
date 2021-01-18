#include "builder.h"
#include <async++.h>
#include <vector>
#include <chrono>
#include <iostream>
#include <atomic>
#include <boost/process.hpp>
#include <boost/process/initializers.hpp>
#include <boost/iostreams/stream.hpp>

std::chrono::seconds seconds_now(){
  return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
}

void build(const std::string& config, int timeout, bool b_install, bool b_pack){
  std::atomic_bool done = false;
  auto start = seconds_now();
  std::vector<boost::process::child> processes;
  auto timing = async::spawn([&] {
    while(!done and ((seconds_now() - start).count() < timeout)) {
      std::this_thread::yield();
    }
    if (done) return;
    for (auto& c : processes) {
      boost::process::terminate(c);
    }
    std::cout<<"Время ожидания истекло" << std::endl;
  });

  timing.then([&] () {

  });

  boost::process::pipe pipe_stdout = boost::process::create_pipe();

  boost::iostreams::file_descriptor_source source_stdout(pipe_stdout.source, boost::iostreams::close_handle);

  boost::iostreams::stream<boost::iostreams::file_descriptor_source> stream_stdout(source_stdout);

  auto cmake_path = "/usr/bin/cmake";

  auto make = async::spawn([&]() {
    {
      boost::iostreams::file_descriptor_sink sink_stdout(
          pipe_stdout.sink, boost::iostreams::close_handle);
      auto p =
          boost::process::execute(
          boost::process::initializers::set_args(
              std::vector<std::string>{
          cmake_path, "-H.", "-B_builds", "-DCMAKE_INSTALL_PREFIX=_install",
          "-DCMAKE_BUILD_TYPE=" + config}),
              boost::process::initializers::throw_on_error(),
          boost::process::initializers::bind_stdout(sink_stdout),
      boost::process::initializers::inherit_env());
      processes.push_back(p);
      boost::process::wait_for_exit(p);
    }
  });
  auto build = make.then([&](){
    {
      boost::iostreams::file_descriptor_sink sink_stdout(
          pipe_stdout.sink, boost::iostreams::close_handle);
      auto p =
          boost::process::execute(
              boost::process::initializers::set_args(
                  std::vector<std::string>{cmake_path, "--build", "_builds"}),
              boost::process::initializers::throw_on_error(),
              boost::process::initializers::bind_stdout(sink_stdout),
              boost::process::initializers::inherit_env());
      processes.push_back(p);
      boost::process::wait_for_exit(p);
    }
  });
  auto install = build.then([&]() {
    if (b_install)
    {
      boost::iostreams::file_descriptor_sink sink_stdout(
          pipe_stdout.sink, boost::iostreams::close_handle);
      auto p =
          boost::process::execute(
              boost::process::initializers::set_args(
                  std::vector<std::string>{cmake_path, "--build", "_builds", "--target", "install"}),
              boost::process::initializers::throw_on_error(),
              boost::process::initializers::bind_stdout(sink_stdout));
      processes.push_back(p);
      boost::process::wait_for_exit(p);
    }
  });
  auto pack = install.then([&]() {
    if (b_pack)
    {
      boost::iostreams::file_descriptor_sink sink_stdout(
          pipe_stdout.sink, boost::iostreams::close_handle);
      auto p =
          boost::process::execute(
              boost::process::initializers::set_args(
                  std::vector<std::string>{cmake_path, "--build", "_builds", "--target", "package"}),
              boost::process::initializers::throw_on_error(),
              boost::process::initializers::bind_stdout(sink_stdout));
      processes.push_back(p);
      boost::process::wait_for_exit(p);
    }
    done = true;
  });
  pack.get();
}
