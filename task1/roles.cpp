#include <vector>
#include <random>
#include <algorithm>
#include <iostream>
#include <map>
#include <concepts>
#include <utility>
#include <string>
#include <stdexcept>
#include "logger.cpp"
#include "mafia.cpp"


template<typename T>
T choice(const std::vector<T>& players) {
    if (players.empty()) {
        throw std::invalid_argument("players vector is empty");
    }
    static std::default_random_engine generator(std::random_device{}());
    std::vector<T> sample(1);
    std::sample(players.begin(), players.end(), sample.begin(), 1, generator);
    return sample[0];
}




class Player {
public:
    virtual long skill(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& Log, int round)  = 0; 
    virtual long vote(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& Log, int round)  = 0; 
    virtual std::string role() const = 0;
    virtual ~Player() = default;
};


class Doctor:public Player{
    long pred = -1;
    long skill(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round)  {
        std::vector<int> targets;
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            targets.push_back(pair.first);
        }
        int chosenTarget = -1; 
        L:
        if (!targets.empty()) {
            chosenTarget = choice(targets);
        }
        
        if(pred != -1){
            if(pred == chosenTarget){
                goto L;
            }
            else{
                pred=chosenTarget;
            }
        }
        logger.logRound(round, "Доктор "+ std::to_string(id) +" выбрал игрока " + std::to_string(chosenTarget) + " для спасения.");
        return chosenTarget;
    }

    long vote(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round) {
        std::vector<int> targets;
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(pair.first != id){
                targets.push_back(pair.first);
            }
        }
        int chosenTarget = -1; 
        if (!targets.empty()) {
            chosenTarget = choice(targets);
        }
        logger.logRound(round, "Доктор "+ std::to_string(id) +" голосует за игрока " + std::to_string(chosenTarget) + ".");
        return chosenTarget;
    }

    std::string role() const override { return "Доктор"; }
};


class AlCapone: public Player{
    long skill(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round) {
        std::vector<int> targets;
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(player->role() != "Мафиози" && player->role() != "Босс мафии"){
                targets.push_back(pair.first);
            }
        }
        int chosenTarget = -1; 
        if (!targets.empty()) {
            chosenTarget = choice(targets);
        }
        logger.logRound(round, "Босс мафии "+ std::to_string(id) +" выбрала игрока " + std::to_string(chosenTarget) + " для убийства.");
        return chosenTarget;    
    }

    long vote(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round)  {
        std::vector<int> targets;
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(player->role() != "Мафиози" && player->role() != "Босс мафии"){
                targets.push_back(pair.first);
            }
        }
        int chosenTarget = -1; 
        if (!targets.empty()) {
            chosenTarget = choice(targets);
        }
        logger.logRound(round, "Босс мафии "+ std::to_string(id) +" голосует за игрока " + std::to_string(chosenTarget) + ".");
        return chosenTarget;
    }

    std::string role() const override { return "Босс мафии"; }
};


class Mafia: public Player{
    long skill(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round) {
        std::vector<int> targets;
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(player->role() != "Мафиози"){
                targets.push_back(pair.first);
            }
        }
        int chosenTarget = -1; 
        if (!targets.empty()) {
            chosenTarget = choice(targets);
        }
        logger.logRound(round, "Мафия "+ std::to_string(id) +" выбрала игрока " + std::to_string(chosenTarget) + " для убийства.");
        return chosenTarget;    
    }

    long vote(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round)  {
        std::vector<int> targets;
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(player->role() != "Мафиози"){
                targets.push_back(pair.first);
            }
        }
        int chosenTarget = -1; 
        if (!targets.empty()) {
            chosenTarget = choice(targets);
        }
        logger.logRound(round, "Мафия "+ std::to_string(id) +" голосует за игрока " + std::to_string(chosenTarget) + ".");
        return chosenTarget;
    }

    std::string role() const override { return "Мафиози"; }
};



class Civilian:public Player{
    long skill(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round)  {}
    long vote(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round)  {
        std::vector<int> targets;
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(pair.first != id){
                targets.push_back(pair.first);
            }
        }
        int chosenTarget = -1; 
        if (!targets.empty()) {
            chosenTarget = choice(targets);
        }
        logger.logRound(round, "Мирный житель "+ std::to_string(id) +" голосует за игрока " + std::to_string(chosenTarget) + ".");
        return chosenTarget;
    }
    std::string role() const override { return "Мирный житель"; }
};


class Commissioner:public Player{
    long aim = -1;
    long skill(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round)  {
        if(aim != -1 && players.find(aim) != players.end()) {
            int death=aim;
            aim = -1;
            logger.logRound(round, "Комиссар решил застрелить игрока " + std::to_string(death) + " ибо он мафия.");

        }
        else{
        std::vector<int> targets;
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(player->role() != "Комиссар"){
                targets.push_back(pair.first);
            }
        }
        long chosenTarget = -1; 
        if (!targets.empty()) {
                chosenTarget = choice(targets); 
                logger.logRound(round, "Комиссар "+ std::to_string(id) +" выбрал игрока " + std::to_string(chosenTarget) + " для проверки.");
                const std::string& role = players.at(chosenTarget).get()->role();
                if (role == "Мафиози") {
                    aim = chosenTarget;
                    logger.logRound(round, "Комиссар проверил игрока " + std::to_string(chosenTarget) + " и обнаружил, что он мафия.");
                } else {
                    logger.logRound(round, "Комиссар проверил игрока " + std::to_string(chosenTarget) + " и обнаружил, что он не мафия.");
                }
        }
        return chosenTarget;
        }
    }
    


    long vote(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round)  {
        std::vector<int> targets;
        int chosenTarget = -1; 
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(pair.first != id){
                targets.push_back(pair.first);
            }
        }
        if (!targets.empty()) {
            chosenTarget = choice(targets);
        }
        if (aim != -1){
            chosenTarget=aim;
        }
        logger.logRound(round, "Комиссар "+ std::to_string(id) +" голосует за игрока " + std::to_string(chosenTarget) + ".");
        return chosenTarget;
    }


    std::string role() const override { return "Комиссар"; }
};


class Maniac:public Player{
    long skill(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round)  {
        std::vector<int> targets;
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(player->role() != "Маньяк"){
                targets.push_back(pair.first);
            }
        }
        int chosenTarget = -1; 
        if (!targets.empty()) {
            chosenTarget = choice(targets);
        }
        logger.logRound(round, "Маньяк "+ std::to_string(id) +" выбрал игрока " + std::to_string(chosenTarget) + " для убийства.");
        return chosenTarget;
    }

    long vote(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round) {
        std::vector<int> targets;
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(pair.first != id){
                targets.push_back(pair.first);
            }
        }
        int chosenTarget = -1; 
        if (!targets.empty()) {
            chosenTarget = choice(targets);
        }
        logger.logRound(round, "Маньяк "+ std::to_string(id) +" голосует за игрока " + std::to_string(chosenTarget) + ".");
        return chosenTarget;
    }

    std::string role() const override { return "Маньяк"; }
};

