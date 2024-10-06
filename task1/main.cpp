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
#include "logger.cpp"
#include <tuple>
#include <optional>
#include <ranges>
#include <algorithm>

int pred_doctor = -1;
int player_blade = 0;

template <typename PlayerType>
PlayerType* getPlayerByRole(const std::string& role,std::map<int, mafia::shared_ptr<Player>> players) {
    auto player = players | std::views::filter([&](const auto& entry) {
        return entry.second.get()->role() == role;
    });
    if (!std::ranges::empty(player)) {
        return dynamic_cast<PlayerType*>(player.begin()->second.get());
    }
    return nullptr;
}

template <typename PlayerType>
int getTargetIfAlive(const std::string& role,std::map<int, mafia::shared_ptr<Player>> players ) {
    if (auto* player = getPlayerByRole<PlayerType>(role, players)) {
        return player->getTarget();
    }
    return -1;
}


template <typename PlayerType>
int getAimIfAlive(const std::string& role,std::map<int, mafia::shared_ptr<Player>> players ) {
    if (auto* player = getPlayerByRole<PlayerType>(role, players)) {
        return player->getAim();
    }
    return -1;
}

template <typename PlayerType>
int getCounterIfAlive(const std::string& role,std::map<int, mafia::shared_ptr<Player>> players ) {
    if (auto* player = getPlayerByRole<PlayerType>(role, players)) {
        return player->getCounter();
    }
    return -1;
}


template<typename T>
T role_for_player(const std::vector<T>& players) {
    if (players.empty()) {
        throw std::invalid_argument("players vector is empty");
    }
    static std::default_random_engine generator(std::random_device{}());
    std::vector<T> sample(1);
    std::sample(players.begin(), players.end(), sample.begin(), 1, generator);
    return sample[0];
}



std::map<int, mafia::shared_ptr<Player>> createPlayers(int numPlayers,int helper) {
    std::map<int, mafia::shared_ptr<Player>> players;
    int numMafias = numPlayers / helper;
    for (int i = 0; i < numPlayers; ++i) {
        if (i < 1) {
            players[i] = mafia::shared_ptr<Player>(new AlCapone());
        }else if (i < numMafias-2) {
            players[i] = mafia::shared_ptr<Player>(new Mafia());
        } else if (i < numMafias-1) {
            players[i] = mafia::shared_ptr<Player>(new Ninja());
        }else if (i < numMafias) {
            players[i] = mafia::shared_ptr<Player>(new Killer());
        }else if (i == numMafias) {
            players[i] = mafia::shared_ptr<Player>(new Commissioner());
        } else if (i == numMafias+1) {
            players[i] = mafia::shared_ptr<Player>(new Maniac());
        } else if (i == numMafias + 2){
            players[i] = mafia::shared_ptr<Player>(new Doctor());
        }else if (i == numMafias + 3){
            players[i] = mafia::shared_ptr<Player>(new Blade());
        }else{
           players[i] = mafia::shared_ptr<Player>(new Civilian());
        }
    }
    return players;
}



int UserRole(std::map<int, mafia::shared_ptr<Player>>& players) {
    std::vector<int> targets;
    for (const auto& pair: players){
        targets.push_back(pair.first);       
    }
    int me = role_for_player(targets);
    std::cout << "Вы играете за игрока с номером: " << me << ". Ваша роль: " << players[me].get()->role() << std::endl;
    return me;
}





bool check(std::map<int,mafia::shared_ptr<Player>>& players, Logger& logger) {
    int mafia = 0;
    int civilian = 0;
    int maniac = 0;
    for (const auto& pair: players){
        const Player* player=pair.second.get();
        if (player->role() == "Мафиози" || player->role() == "Босс мафии" || player->role() == "Ниндзя" || player->role() == "Киллер") {
            mafia++;
        } 
        else if(player->role() == "Маньяк") {
            maniac++;
            civilian++;
        }
        else{
            civilian++;
        }
    }
    if (mafia>= civilian) {
        logger.logresult(
    " M     M    AAAAA    FFFFFF  IIIIII   AAAAA  \n"
    " MM   MM   A     A   F         II    A     A \n"
    " M M M M   AAAAAAA   FFFFFF    II    AAAAAAA \n"
    " M  M  M   A     A   F         II    A     A \n"
    " M     M   A     A   F       IIIIII  A     A \n");
        return true;
    }
    if (mafia == 0 && maniac == 0) {
        logger.logresult(
    " CCCCC   IIIIII  V     V  IIIIII   L       IIIIII   AAAAA     N     N\n"
    "C          II    V     V    II     L         II    A     A    NN    N\n"
    "C          II     V   V     II     L         II    AAAAAAA    N N   N\n"
    "C          II      V V      II     L         II    A     A    N  N  N\n"
    " CCCCC   IIIIII     V     IIIIII   LLLLLL  IIIIII  A     A    N   N N\n");
        return true;
    }
    if (maniac == 1 && civilian == 1){
        logger.logresult(
    " M     M    AAAAA    N     N   IIIIII   AAAAA    CCCCC  \n"
    " MM   MM   A     A   NN    N     II    A     A  C       \n"
    " M M M M   AAAAAAA   N N   N     II    AAAAAAA  C       \n"
    " M  M  M   A     A   N  N  N     II    A     A  C       \n"
    " M     M   A     A   N   N N   IIIIII  A     A   CCCCC  \n");  
        return true;
    }
    return false; 
}


void day(std::map<int, mafia::shared_ptr<Player>>& players, Logger& logger, int round, bool user_in_game, int user) {
    std::map<int, int> voteCount;

    std::vector<std::future<void>> futures;
    for (const auto& [id, player] : players) {
        futures.push_back(std::async(std::launch::async, [&]() {
            if (user_in_game && id == user) {
                
            } else {
                Move action = player.get()->vote(players, id, logger, round);
                action.handle.resume();
            }
        }));
    }

    for (auto& future : futures) {
        future.get(); 
    }
    futures.clear();

    for (const auto& [id, player] : players) {
        long vo = player.get()->getTarget();
        if (vo != -1) {
            voteCount[vo]++;
        }
    }

    if (user_in_game) {
        if (players.find(user) == players.end()) {
            std::cout << "К сожалению вы мертвы(((((((((((((((" << std::endl;
        }
        else{
            std::cout << "Ваш голос! Выберите игрока для голосования (ID):" << std::endl;
            for (const auto& pair : players) {
                int id = pair.first;
                if (id != user) {
                    std::cout << "Игрок " << id << " (" << pair.second.get()->role() << ")" << std::endl;
                }
            }
            int playervote;
            L:
            std::cin >> playervote;
            if (players.find(playervote) != players.end() && playervote != user) {
                logger.logRound(round, "Игрок " + std::to_string(user) + " голосует за игрока " + std::to_string(playervote));
            } else if (playervote == user) {
                std::cout << "Нельзя голосовать за себя!!! Введите другое число:" << std::endl;
                goto L;
            } else {
                std::cout << "Некорректный выбор!!! Введите другое число:" << std::endl;
                goto L;
            }
        }
    }
                

   if (!voteCount.empty()) {
    int max = -1;
    int unlucky;

    for (const auto& vote : voteCount) {
        if (vote.second > max) {
            max = vote.second;
            unlucky = vote.first;
        }
    }

    int whoisblade=-1;
    for (const auto& [id, player] : players) {  
            if(player.get()->role() == "Блэйд"){
                whoisblade = id; 
            }
    }

    int comewithme=-1;
    if(whoisblade==unlucky){
        for (const auto& [id, player] : players) {
        long gg = player.get()->getTarget();
        if (gg==whoisblade) {
            comewithme=id;
            break;
        }
    }
    }

    players.erase(unlucky);
    logger.logRound(round, "Игрок " + std::to_string(unlucky) + " решением голосования был кикнут.");
    if(comewithme!=-1){
        players.erase(comewithme);
        logger.logRound(round, "Блэйд забрал игрока " + std::to_string(comewithme) + " так как он первым за него проголосовал");
    }
}


}




void night(std::map<int, mafia::shared_ptr<Player>>& players, Logger& logger, int round, bool user_in_game, int user, int Num) {
    std::vector<int> mafias;
    int donalive=-1;
    int bladealive=-1;
    for (const auto& [id, player] : players) {  
    if (player.get()->role() == "Мафиози" || player.get()->role() == "Босс мафии" || player.get()->role() == "Ниндзя" || player.get()->role() == "Блэйд") {
            mafias.push_back(id);
            if(player.get()->role() == "Босс мафии"){
                donalive = id; 
            }
            else if(player.get()->role() == "Блэйд"){
                bladealive = id; 
            }
        }
    }

    int doctor = -1;
    int comis = -1;
    int mafiachoise = -1;
    int donvoice = -1;
    int maniacvoice = -1;
    int ninja=-1;
    int blade =-1;
    int bladecounter=-1;
    int killervoice = -1;
    std::vector<int> mafiasdesicion;
     std::vector<std::future<void>> futures;
     for (const auto& [id, player] : players) {
        if (user_in_game && id == user) {
        }else{
            futures.push_back(std::async(std::launch::async, [&]() {
                    Move action = player.get()->skill(players, id, logger, round);
                    action.handle.resume();
            }));
        }
    }
    for (auto& future : futures) {
        future.get();
    }
    futures.clear();

    int user_doctor=-1;
    int user_mafia=-1;
    int user_comis=-1;
    int user_maniac=-1;
    int user_boss = -1;
    int user_ninja=-1;
    int user_blade=-1;
    int user_killer = -1;

    if(user_in_game){
        if (players.find(user) == players.end()) {
        std::cout << "К сожалению, Вы мертвы((((((((((((((" << std::endl;
        }
    else{
        std::cout << "Вы живы" << std::endl;
        if (players[user].get()->role() == "Мирный житель"){
            std::cout << "Ваш навык это сон, поэтому идите поспите"<<std::endl;
        }
        else if (players[user].get()->role() == "Комиссар"){
            std::cout<<"Какое действие Вы хотите сделать"<<std::endl;
            std::cout << "0 - Проверить игрока." << std::endl;
            std::cout << "1 - Выстрелить в игрока." << std::endl;
            int var;
            std::cin >> var;
            while (var != 0 && var != 1){
                std::cout << "Число введено некорректно" << std::endl;
                std::cin >> var;
            }
            if (var == 0){
                std::cout << "Выберите ID игрока для проверки: " << std::endl;
                int target;
                std::cin >> target;
                while(players.find(target) == players.end() || target == user){
                    std::cout << "ID введено некорректно" << std::endl;
                    std::cin >> target;
                }
                if (players[target].get()->role() == "Мафия" || players[target].get()->role() == "Босс мафии"){
                    std::cout << "Выбранный игрок явялется мафией."<< std::endl;
                }
                else{
                    std::cout << "Выбранный игрок является  мирным."<< std::endl;
                }
            }
            else if (var == 1){
                std::cout << "Выберите ID игрока для убийства: " << std::endl;
                int target;
                std::cin >> target;
                while(players.find(target) == players.end() || target == user){
                    std::cout << "ID введено некорректно" << std::endl;
                    std::cin >> target;
                }
                user_comis=target;
            }
        }
        else if(players[user].get()->role() == "Доктор"){
            std::cout<<"Кого бы Вы хотели вылечить"<<std::endl;
            int target;
            std::cin >> target;
            while(players.find(target) == players.end() || target == pred_doctor){
                std::cout << "ID введено некорректно" << std::endl;
                std::cin >> target;
            }
            pred_doctor=target;
            user_doctor=target;
        }
        else if(players[user].get()->role() == "Маньяк"){
            std::cout<<"Кого бы Вы хотели убить"<<std::endl;
            int target;
            std::cin >> target;
            while(players.find(target) == players.end() || target==user){
                std::cout << "ID введено некорректно" << std::endl;
                std::cin >> target;
            }
            user_maniac=target;
        }
        else if(players[user].get()->role() == "Киллер"){
            std::cout<<"Кого бы Вы хотели убить"<<std::endl;
            int target;
            std::cin >> target;
            while(players.find(target) == players.end() || target==user){
                std::cout << "ID введено некорректно" << std::endl;
                std::cin >> target;
            }
            user_killer=target;
        }
        else if(players[user].get()->role() == "Блэйд"){
            std::cout<<"Кого бы Вы хотели убить"<<std::endl;
            for (const auto& [id, player] : players) {
                if (id != user) {
                    std::cout << "Игрок " << id << " (" << player.get()->role() << ")" << std::endl;
                }
            }
            int target;
            std::cin >> target;
            while(players.find(target) == players.end()){
                std::cout << "ID введено некорректно" << std::endl;
                std::cin >> target;
            }
            user_blade=target;
            for (const auto& [id, player] : players) {
                if(id==target && player.get()->role()=="Мирный житель"){
                    player_blade++;
                }
            }
        }
        else if(players[user].get()->role() == "Мафиози"){
            std::cout<<"Кого бы Вы хотели убить"<<std::endl;
            for (const auto& [id, player] : players) {
                if (id != user) {
                    std::cout << "Игрок " << id << " (" << player.get()->role() << ")" << std::endl;
                }
            }
            int target;
            std::cin >> target;
            while(players.find(target) == players.end()){
                std::cout << "ID введено некорректно" << std::endl;
                std::cin >> target;
            }
            user_mafia=target;
        }
        else if(players[user].get()->role() == "Ниндзя"){
            std::cout<<"Кого бы Вы хотели убить"<<std::endl;
            for (const auto& [id, player] : players) {
                if (id != user) {
                    std::cout << "Игрок " << id << " (" << player.get()->role() << ")" << std::endl;
                }
            }
            int target;
            std::cin >> target;
            while(players.find(target) == players.end()){
                std::cout << "ID введено некорректно" << std::endl;
                std::cin >> target;
            }
            user_ninja=target;
        }
        else{
            std::cout<<"Кого бы Вы хотели убить"<<std::endl;
            for (const auto& [id, player] : players) {
                if (id != user) {
                    std::cout << "Игрок " << id << " (" << player.get()->role() << ")" << std::endl;
                }
            }
            int target;
            std::cin >> target;
            while(players.find(target) == players.end()){
                std::cout << "ID введено некорректно" << std::endl;
                std::cin >> target;
            }
            user_boss=target;

        }
    }
    

    }

    maniacvoice = getTargetIfAlive<Maniac>("Маньяк",players);
    doctor = getTargetIfAlive<Doctor>("Доктор",players);
    comis = getAimIfAlive<Commissioner>("Комиссар",players);
    ninja=getTargetIfAlive<Ninja>("Ниндзя",players);
    blade=getTargetIfAlive<Blade>("Блэйд",players);
    bladecounter=getCounterIfAlive<Blade>("Блэйд",players);
    killervoice=getTargetIfAlive<Killer>("Киллер",players);
    

    if (donalive != -1) {
        donvoice = getTargetIfAlive<AlCapone>("Босс мафии",players);
    }

    auto mafiaPlayers = players | std::views::filter([](const auto& entry) {
        return entry.second.get()->role() == "Мафиози";
    });

    for (const auto& entry : mafiaPlayers) {
        if (auto* mafioso = dynamic_cast<Mafia*>(entry.second.get())) {
            mafiasdesicion.push_back(mafioso->getTarget());
        }
    }

    if(user_ninja!=-1){
        mafiasdesicion.push_back(user_ninja);
    }
    else{
        mafiasdesicion.push_back(ninja);
    }


    if(user_boss != -1){
        donvoice=user_boss;
    }

    if(user_mafia!=-1){
        mafiasdesicion.push_back(user_mafia);
    }

    bool hasUniquePopular = true;
    std::unordered_map<int, int> frequency;
    for(int num : mafiasdesicion) {
        frequency[num]++;
    }

    int maxFrequency = 0;
    
    for(const auto& pair : frequency) {
        if(pair.second > maxFrequency) {
            maxFrequency = pair.second;
            mafiachoise = pair.first;
            hasUniquePopular = true;
        } else if(pair.second == maxFrequency && pair.first != mafiachoise) {
            hasUniquePopular = false;
        }
    }

    if(!hasUniquePopular) {
        mafiachoise = donvoice;
        if(donvoice==-1){
            mafiachoise=role_for_player(mafiasdesicion);
        }
    }

    if(user_maniac!=-1){
        maniacvoice=user_maniac;
    }


     if(user_killer!=-1){
        killervoice=user_killer;
    }

    if(user_doctor!=-1){
        doctor=user_doctor;
    }

    if(user_comis!=-1){
        comis=user_comis;
    }

     if(user_blade!=-1){
        blade=user_blade;
    }

    if (maniacvoice != -1 && maniacvoice != doctor && maniacvoice!=bladealive) {
        players.erase(maniacvoice);
        logger.logRound(round, "Игрок " + std::to_string(maniacvoice) + " был убит маньяком.");
    }

    if (mafiachoise != -1 && mafiachoise != doctor && mafiachoise!=bladealive) {
        players.erase(mafiachoise);
        logger.logRound(round, "Игрок " + std::to_string(mafiachoise) + " был убит мафией.");
    }

    if (comis != -1  && comis != doctor && comis!=bladealive) {
        players.erase(comis);
        logger.logRound(round, "Игрок " + std::to_string(comis) + " был убит комиссаром.");
    }  

    if (killervoice != -1  && killervoice != doctor && killervoice!=bladealive) {
        players.erase(killervoice);
        logger.logRound(round, "Игрок " + std::to_string(killervoice) + " был убит киллером.");
    }  

    if (blade != -1 && blade != doctor) {
        players.erase(blade);
        logger.logRound(round, "Игрок " + std::to_string(blade) + " был убит Блэйдом.");
        if(bladecounter==3 || player_blade==3){
            players.erase(bladealive);
            logger.logRound(round, "Блэйд  убил трех мирных жителей и поэтому умер.");
        }
    }
}





void start(std::map<int, mafia::shared_ptr<Player>>& players, Logger& logger, bool user_in_game, int user, int count) {
    int round = 1;
    while (true) {
        logger.logRound(round, "Начало раунда " + std::to_string(round));
        night(players, logger, round, user_in_game, user, players.size());
        if (check(players, logger)) break;
        day(players, logger, round, user_in_game, user);
        if (check(players, logger)) break;
        round++;
    }
}




int main(){
    int N;
    std::cout << "Введите количество игроков (должно быть больше 4): ";
    std::cin >> N;

    int you = -1;

    if (N<= 4) {
        std::cerr << "Больше 4!!!!!!!!!!!!!!!!!!!" << std::endl;
        return 1;
    }


    int k;
    std::cout << "Введите  k ";
    std::cin >> k;
    if (k > N || k<3) {
        std::cerr << "Должно быть больше 2 но меньше количество игроков" << std::endl;
        return 1;
    }

    char user_in_game;
    int user_in = 0;
    std::cout << "Будете ли Вы играть  (y если да, n если нет): ";
    std::cin >> user_in_game;
    if (user_in_game == 'y' || user_in_game == 'Y') {
        user_in = 1;
    }

    Logger logger;
    auto players = createPlayers(N, k);

    
    if (user_in) {
       you = UserRole(players);
    }

    for (const auto& pair: players){
        const Player* player=pair.second.get();
        std::cout << 'Player Id:'<<pair.first<<", Role:"<< player->role() <<std::endl;
    }


    for (const auto& [id, player] : players) {
        logger.logRound(0, "Игрок " + std::to_string(id) + ": " + player.get()->role());
    }

    logger.logRound(0, "Игра началась!");
    start(players, logger, user_in, you, N);


    return 0;
}








 



  








