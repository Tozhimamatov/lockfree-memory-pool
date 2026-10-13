# ⚡ High-Performance Lock-Free Memory Pool Allocator (C++20)

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()

A thread-safe, ultra-low latency **Lock-Free Memory Pool Allocator** designed for real-time systems, High-Frequency Trading (HFT), and game engines. Built using **C++20 Concepts**, atomic CAS operations, and ABA problem mitigation via tagged pointers.

## 🎯 Key Features

- **$O(1)$ Deterministic Time Complexity:** Zero dynamic OS allocation calls (`malloc`/`free`) during runtime.
- **Lock-Free Architecture:** Leverages lock-free `std::atomic` Compare-And-Swap (CAS) operations.
- **ABA Problem Mitigation:** Implements atomic tagged pointers (`ptr` + `tag` counter).
- **Cache-Line Aligned:** Structurally aligned to `64 bytes` to eliminate false sharing between CPU threads.
- **Header-Only Library:** Easy integration into existing C++ projects.

## 🛠 Building & Running

### Requirements
- C++20 compliant compiler (GCC 11+, Clang 13+, MSVC 2019+)
- CMake 3.20+

```bash
mkdir build && cd build
cmake ..
cmake --build .
./main_demo
