#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <random>
#include <concepts>
#include <coroutine>
#include <utility> 
#include <map>
#include <thread>
#include <future>
#include "mafia.cpp"
#include <mutex>
#include <chrono>
#include <ctime>
#include <filesystem>
#include "roles.cpp"


class Logger {
    std::filesystem::path log_directory;

public:
    Logger() {
        log_directory = std::filesystem::current_path() / "LOGGER";
        std::filesystem::create_directory(log_directory);
    }

    void logRound(int round, const std::string& log) {
        std::filesystem::path log_file_path = log_directory / ("round_" + std::to_string(round) + ".log");
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






std::map<int, mafia::shared_ptr<Player>> createPlayers(int numPlayers, bool includeUser) {
    std::map<int, mafia::shared_ptr<Player>> players;

    // Примерный расчет количества мафий, при N/k, где k >= 3
    int numMafias = numPlayers / 3;
 
    for (int i = 0; i < numPlayers; ++i) {
        if (i < numMafias) {
          
            players[i] = mafia::shared_ptr<Player>(new Mafia());
  
        } else if (i == numMafias) {
            players[i] = mafia::shared_ptr<Player>(new Commissioner());
        } else if (i == numMafias + 1) {
            players[i] = mafia::shared_ptr<Player>(new Maniac());
        } else {
            players[i] = mafia::shared_ptr<Player>(new Civilian());
        }
    }

    /*if (includeUser) {
        players[numPlayers - 1] = mafia::shared_ptr<Player>(new Civilian()); // Пример: последний игрок - пользователь
    }*/


    return players;
}




int main(){
    int N;
    std::cout << "Enter number of players (must be greater than 4): ";
    std::cin >> N;
     if (N<= 4) {
        std::cerr << "Number of players must be greater than 4." << std::endl;
        return 1;
    }

    char user_in_game;
    int user_in = 0;
    std::cout << "Is the user a player? (y for yes, n for no): ";
    std::cin >> user_in_game;
    if (user_in_game == 'y' || user_in_game == 'Y') {
        user_in = 1;
    }

    Logger logger;
    auto players = createPlayers(N, user_in);
    
    for (const auto& pair: players){
        const Player* player=pair.second.get();
        std::cout << 'Player Id:'<<pair.first<<", Role:"<< player->role() <<std::endl;
    }

    return 0;
}



