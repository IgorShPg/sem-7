#include <filesystem>
#include <iostream>
#include <fstream>
#pragma once

class Logger {
    std::filesystem::path log_directory;

public:
    Logger() {
        log_directory = std::filesystem::current_path() / "LOGGER";
        std::filesystem::create_directory(log_directory);
    }

    void logRound(int round, const std::string& log) {
        std::filesystem::path log_file_path = log_directory / ("round" + std::to_string(round) + ".log");
        std::ofstream log_file(log_file_path, std::ios::app);
        if (log_file.is_open()) {
            log_file << log << std::endl;
            log_file.close();
        } else {
            std::cerr << "Error opening log file: " << log_file_path << std::endl;
        }
    }

    void logFinal(const std::string& final_log) {
        std::ofstream final_file(log_directory / "result.log", std::ios::app);
        final_file << final_log << std::endl;
        final_file.close();
    }

    void setLogDirectory(const std::filesystem::path& path) {
        log_directory = path;
        std::filesystem::create_directory(log_directory);
    }

    void clearLogs() {
        for (const auto& entry : std::filesystem::directory_iterator(log_directory)) {
            std::filesystem::remove(entry);
        }
    }
};