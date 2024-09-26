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






std::unordered_map<int, mafia::shared_ptr<Player>> createPlayers(int numPlayers, bool includeUser) {
    std::unordered_map<int, mafia::shared_ptr<Player>> players;

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

    if (includeUser) {
        players[numPlayers - 1] = mafia::shared_ptr<Player>(new Civilian()); // Пример: последний игрок - пользователь
    }


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
    Host host(logger, players);
    host.startGame();

    return 0;
}



