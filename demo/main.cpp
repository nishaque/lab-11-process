#include <boost/program_options.hpp>
#include <iostream>
#include "../sources/builder.cpp"

namespace po = boost::program_options;

void run(int, char*[]);



int main(int argc, char* argv[]) {
  run(argc, argv);
  return 0;
}



void run(int argc, char* argv[]) {
  try {
    int timeout;
    std::string config;
    bool install, pack;
    po::options_description desc("Allowed options");
    desc.add_options()("help", "выводим вспомогательное сообщение")(
        "config", po::value<std::string>(&config)->default_value("Debug"),
        "указываем конфигурацию сборки (по умолчанию Debug)")(
        "install", "добавляем этап установки (в директорию _install)")(
        "pack", "добавляем этап упаковки (в архив формата tar.gz)")(
        "timeout", po::value<int>(&timeout)->default_value(10),
        "указываем время ожидания (в секундах)");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << "\n";
      return;
    }

    if (vm.count("config")) {
      config = vm["config"].as<std::string>();
    }
    install = vm.count("install");
    pack = vm.count("pack");
    build(config, timeout, install, pack);
  } catch(std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
    return;
  }
}