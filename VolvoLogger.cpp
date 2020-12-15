// VolvoLogger.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//

#include "J2534.hpp"
#include "LogParameters.hpp"
#include "Logger.h"
#include "LoggerApplication.hpp"

#include "Registry.hpp"
#include <boost/program_options.hpp>
#include <codecvt>
#include <conio.h>
#include <iostream>
#include <locale>
#include <memory>
#include <thread>

using namespace m4x1m1l14n::Registry;

std::wstring toWstring(const std::string &str) {
  using convert_type = std::codecvt_utf8<wchar_t>;
  std::wstring_convert<convert_type, wchar_t> converter;
  return converter.from_bytes(str);
}

std::string toString(const std::wstring &str) {
  using convert_type = std::codecvt_utf8<wchar_t>;
  std::wstring_convert<convert_type, wchar_t> converter;
  return converter.to_bytes(str);
}

bool processRegistry(const std::string &keyName, std::string &libraryPath,
                     std::string &deviceName) {
  const auto key = LocalMachine->Open(keyName);
  const auto canXonXoff{key->GetInt32(TEXT("CAN_XON_XOFF"))};
  if (canXonXoff > 0) {
    libraryPath = key->GetString(TEXT("FunctionLibrary"));
    deviceName = key->GetString(TEXT("Name"));
    return false;
  }
  return true;
}

std::pair<std::string, std::string> getLibraryParams() {
  std::string libraryPath;
  std::string deviceName;
  const std::string rootKeyName{TEXT("Software\\PassThruSupport.04.04")};
  const auto key = LocalMachine->Open(rootKeyName);
  key->EnumerateSubKeys(
      [&rootKeyName, &libraryPath, &deviceName](const auto &subKeyName) {
        return processRegistry(rootKeyName + TEXT("\\") + subKeyName,
                               libraryPath, deviceName);
      });
  return {libraryPath, deviceName};
}

bool getRunOptions(int argc, const char *argv[], unsigned long &baudrate,
                   std::string &paramsFilePath, std::string &outputPath) {
  using namespace boost::program_options;
  options_description descr;
  descr.add_options()(
      "baudrate,b", value<unsigned long>()->default_value(500000),
      "CAN bus speed")("variables,v", value<std::string>()->required(),
                       "Path to memory variables")(
      "output,o", value<std::string>()->required(), "Path to save logs");
  command_line_parser parser{argc, argv};
  parser.options(descr);
  variables_map vm;
  store(parser.run(), vm);
  if (vm.count("variables") && vm.count("output")) {
    baudrate = vm["baudrate"].as<unsigned long>();
    paramsFilePath = vm["variables"].as<std::string>();
    outputPath = vm["output"].as<std::string>();
    return true;
  } else {
    std::cout << descr;
    return false;
  }
}

BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType) {
  logger::LoggerApplication::instance().stop();
  return TRUE;
}

int main(int argc, const char *argv[]) {
  if (!SetConsoleCtrlHandler(HandlerRoutine, TRUE)) {
    throw std::runtime_error("Can't set console control hander");
  }
  unsigned long baudrate = 0;
  std::string paramsFilePath;
  std::string outputPath;
  if (getRunOptions(argc, argv, baudrate, paramsFilePath, outputPath)) {
    const auto libraryParams{getLibraryParams()};
    if (!libraryParams.first.empty()) {
      try {
        std::unique_ptr<j2534::J2534> j2534{
            std::make_unique<j2534::J2534>(libraryParams.first)};
        j2534->PassThruOpen(libraryParams.second);
        logger::LogParameters params{paramsFilePath};
        logger::LoggerApplication::instance();
        logger::LoggerApplication::instance().start(baudrate, std::move(j2534),
                                                    params, outputPath);
        while (logger::LoggerApplication::instance().isStarted()) {
          std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        logger::LoggerApplication::instance().stop();
      } catch (const std::exception &ex) {
        std::cout << ex.what() << std::endl;
      } catch (const char *ex) {
        std::cout << ex << std::endl;
      } catch (...) {
        std::cout << "exception" << std::endl;
      }
    }
  }
  return 0;
}
