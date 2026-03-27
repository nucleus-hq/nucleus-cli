#pragma once
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <string>

namespace console {

inline void spinner(const std::string& msg, const std::string& cmd) {
    using namespace std::chrono;

    // Start timer
    auto start = high_resolution_clock::now();

    // Run the command
    int ret = std::system(cmd.c_str());
    (void)ret; // ignore ret for now

    // End timer
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start).count();
    double seconds = duration / 1000.0;

    // Spinner for approximately the same duration
    const char spin_chars[] = "|/-\\";
    size_t i = 0;
    int iterations = static_cast<int>(seconds * 10); // 0.1s per frame

    for (int j = 0; j < iterations; ++j) {
        std::cout << "\r[*] " << msg << " " << spin_chars[i++ % 4] << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "\r"; // clear line
}

} // namespace console
