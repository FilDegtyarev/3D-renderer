#include "exceptions/exceptions.h"

#include <exception>
#include <iostream>

namespace detail {
namespace exceptions {
void react() {
  try {
    throw;
  } catch (std::exception& e) {
    // обрабатываешь известные исключения
    std::cerr << "Known exception" << std::endl;
  } catch (...) {
    // обрабатываешь незивестные исключения
    std::cerr << "Unknown exception" << std::endl;
  }
}
} // namespace exceptions
} // namespace detail
