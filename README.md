# backoffxx

A zero-dependency and header-only backoff retry library.

- supports constant, linear, and exponential backoff strategies
- supports full-jitter, decorrelated-jitter backoff strategies
- supports customized strategy
- separates backoff action from backoff strategies
- written in C++ 17

## Basic usage

```cpp
#include <chrono>
#include <iostream>

#include "backoffxx/backoffxx.h"

using namespace std::chrono_literals;

int main() {
    constexpr auto delay = 200ms;
    constexpr auto max_retries = 5u;

    // will automatically retry when failure
    auto result = backoffxx::attempt(backoffxx::make_exponential(delay, max_retries), [] {
        bool ok = false;
        // do sth..
        return ok ? backoffxx::attempt_rc::success
                  : backoffxx::attempt_rc::failure;
    });

    if (!result.ok()) {
        std::cerr << "Failed execute fn even after retries exhausted";
    }

    return 0;
}
```

## Adding to your project

### Integrating with source code

backoffxx is a header-only library, you can simply copy all header files into your project.

---

backoffxx is also CMake sub-project aware, you can put the entire backoffxx folder into your project source folder, and introduce dependency via sub-directory:

```cmake
# Root CMakeLists.txt

cmake_minimum_required(VERSION 3.16)

project(your_project)

set(CMAKE_CXX_STANDARD 17)

add_executable(your_exe, source.cpp)

add_subdirectory(backoffxx)

target_link_libraries(your_exe backoffxx)
```

### Integrating via CPM.cmake

[CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) is a setup-free CMake dependency management.

Simply download the cmake file and add to your CMake project, then

```cmake
cmake_minimum_required(VERSION 3.16)

project(my_project)

set(CMAKE_CXX_STANDARD 17)

include(cmake/CPM.cmake)
CPMAddPackage("gh:kingsamchen/backoffxx")

add_executable(my_exe source.cpp)

target_link_libraries(my_exe backoffxx)
```

## Build instructions

Building backoffxx requires:

- C++ 17 compatible compiler
- CMake 3.16 or higher
- Python 3.5 or higher, if you want to use the convenient build script

### Using provided `anvil.py`

`anvil.py` wraps a few CMake commands for easy use:

```shell
$ cd backoffxx
$ python(3) ./anvil.py                          # Run a release build
$ cd path-to-build && ctest --output-on-failure # Run tests
```

Run `python(3) ./anvil.py --help` for details.

### Using CMake directly

If you want fine control over the configuration and building, feel free to use CMake commands directly:

```shell
$ cd backoffxx
$ cmake -DCMAKE_BUILD_TYPE=Release -B path/to/out -S .
$ cmake --build path/to/out -- -j 8
$ cd path-to-build && ctest --output-on-failure # Run tests
```
