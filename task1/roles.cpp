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



struct Move {
    struct promise_type {
        Move get_return_object() {
            return Move{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        void return_void() {}
        void unhandled_exception() {}
    };

    std::coroutine_handle<promise_type> handle;

    ~Move() { handle.destroy(); }
};


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
    long target = -1; 
    long aim = -1;
    int counter=0;
    virtual Move skill(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& Log, int round)  = 0; 
    virtual Move vote(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& Log, int round)  = 0; 
    virtual std::string role() const = 0;
    virtual ~Player() = default;
    virtual long getTarget() const {
        return -1; 
    }
    virtual long getAim() const {
        return -1; 
    }

    virtual int getCounter() const {
        return -1; 
    }
};




class Blade: public Player{
    public:
    long target = -1; 
    long aim = -1;
    int counter=0;
    Move skill(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round) override {
        std::vector<int> targets;
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(player->role() != "Блэйд"){
            targets.push_back(pair.first);
            }
        }
        int chosenTarget = -1; 
        if (!targets.empty()) {
            chosenTarget = choice(targets);
        }

        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(player->role() != "Мафиози" && player->role() != "Босс мафии" && player->role() != "Ниндзя" && chosenTarget==pair.first){
                counter++;
            }
        }

        
        const_cast<Blade*>(this)->target = chosenTarget;
        logger.logRound(round, "Блэйд "+ std::to_string(id) +" выбрал игрока " + std::to_string(chosenTarget) + " для убийства.");
        //return chosenTarget;    
        co_return;
    }

    Move vote(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round)  override {
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
        const_cast<Blade*>(this)->target = chosenTarget;
        logger.logRound(round, "Блэйд "+ std::to_string(id) +" голосует за игрока " + std::to_string(chosenTarget) + ".");
        //return chosenTarget;
        co_return;
    }
    long getTarget() const override {
        return target;
    }

    long getAim() const override{
        return -1; 
    }

    int getCounter() const {
        return counter; 
    }

    std::string role() const override { return "Блэйд"; }
};



class Ninja: public Player{
    public:
    long target = -1; 
    long aim = -1;
    int counter=0;
    Move skill(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round) override {
        std::vector<int> targets;
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(player->role() != "Мафиози" && player->role() != "Босс мафии" && player->role() != "Ниндзя"){
                targets.push_back(pair.first);
            }
        }
        int chosenTarget = -1; 
        if (!targets.empty()) {
            chosenTarget = choice(targets);
        }
        const_cast<Ninja*>(this)->target = chosenTarget;
        logger.logRound(round, "Ниндзя "+ std::to_string(id) +" выбрала игрока " + std::to_string(chosenTarget) + " для убийства.");
        //return chosenTarget;    
        co_return;
    }

    Move vote(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round)  override {
        std::vector<int> targets;
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(player->role() != "Мафиози" && player->role() != "Босс мафии" && player->role() != "Ниндзя"){
                targets.push_back(pair.first);
            }
        }
        int chosenTarget = -1; 
        if (!targets.empty()) {
            chosenTarget = choice(targets);
        }
        const_cast<Ninja*>(this)->target = chosenTarget;
        logger.logRound(round, "Ниндзя "+ std::to_string(id) +" голосует за игрока " + std::to_string(chosenTarget) + ".");
        //return chosenTarget;
        co_return;
    }
    long getTarget() const override {
        return target;
    }

    long getAim() const override{
        return -1; 
    }

    int getCounter() const {
        return -1; 
    }

    std::string role() const override { return "Ниндзя"; }
};



class Doctor: public Player{
    public:
    long pred = -1;
    long target = -1; 
    long aim = -1;
    int counter=0;
    Move skill(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round) override {
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
        const_cast<Doctor*>(this)->target = chosenTarget;
        logger.logRound(round, "Доктор "+ std::to_string(id) +" выбрал игрока " + std::to_string(chosenTarget) + " для спасения.");
        //return chosenTarget;
        co_return;
    }

    Move vote(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round) override {
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
        const_cast<Doctor*>(this)->target = chosenTarget;
        logger.logRound(round, "Доктор "+ std::to_string(id) +" голосует за игрока " + std::to_string(chosenTarget) + ".");
        //return chosenTarget;
        co_return;
    }
    long getTarget() const override {
        return target;
    }

    long getAim() const override{
        return -1; 
    }

    int getCounter() const {
        return -1; 
    }

    std::string role() const override { return "Доктор"; }
};


class AlCapone: public Player{
    public:
    long target = -1; 
    long aim = -1;
    int counter=0;
    Move skill(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round) override {
        std::vector<int> targets;
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(player->role() != "Мафиози" && player->role() != "Босс мафии" && player->role() != "Ниндзя"){
                targets.push_back(pair.first);
            }
        }
        int chosenTarget = -1; 
        if (!targets.empty()) {
            chosenTarget = choice(targets);
        }
        const_cast<AlCapone*>(this)->target = chosenTarget;
        logger.logRound(round, "Босс мафии "+ std::to_string(id) +" выбрал игрока " + std::to_string(chosenTarget) + " для убийства.");
        //return chosenTarget;   
        co_return; 
    }

    Move vote(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round) override {
        std::vector<int> targets;
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(player->role() != "Мафиози" && player->role() != "Босс мафии" && player->role() != "Ниндзя"){
                targets.push_back(pair.first);
            }
        }
        int chosenTarget = -1; 
        if (!targets.empty()) {
            chosenTarget = choice(targets);
        }
        const_cast<AlCapone*>(this)->target = chosenTarget;
        logger.logRound(round, "Босс мафии "+ std::to_string(id) +" голосует за игрока " + std::to_string(chosenTarget) + ".");
        //return chosenTarget;
        co_return;
    }
    long getTarget() const override {
        return target;
    }

    long getAim() const  override{
        return -1; 
    }

    int getCounter() const {
        return -1; 
    }
    std::string role() const override { return "Босс мафии"; }
};


class Mafia: public Player{
    public:
    long target = -1; 
    long aim = -1;
    int counter=0;
    Move skill(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round) override {
        std::vector<int> targets;
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(player->role() != "Мафиози" && player->role() != "Босс мафии" && player->role() != "Ниндзя"){
                targets.push_back(pair.first);
            }
        }
        int chosenTarget = -1; 
        if (!targets.empty()) {
            chosenTarget = choice(targets);
        }
        const_cast<Mafia*>(this)->target = chosenTarget;
        logger.logRound(round, "Мафия "+ std::to_string(id) +" выбрала игрока " + std::to_string(chosenTarget) + " для убийства.");
        //return chosenTarget;    
        co_return;
    }

    Move vote(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round)  override {
        std::vector<int> targets;
        for (const auto& pair: players){
            const Player* player=pair.second.get();
            if(player->role() != "Мафиози" && player->role() != "Босс мафии" && player->role() != "Ниндзя"){
                targets.push_back(pair.first);
            }
        }
        int chosenTarget = -1; 
        if (!targets.empty()) {
            chosenTarget = choice(targets);
        }
        const_cast<Mafia*>(this)->target = chosenTarget;
        logger.logRound(round, "Мафия "+ std::to_string(id) +" голосует за игрока " + std::to_string(chosenTarget) + ".");
        //return chosenTarget;
        co_return;
    }
    long getTarget() const override {
        return target;
    }

    long getAim() const override{
        return -1; 
    }

    int getCounter() const {
        return -1; 
    }

    std::string role() const override { return "Мафиози"; }
};



class Civilian:public Player{
    long target = -1; 
    long aim = -1;
    int counter=0;
    Move skill(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round)  override {
        //return -1;
        co_return;
    }
    Move vote(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round)  override {
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
        const_cast<Civilian*>(this)->target = chosenTarget;
        logger.logRound(round, "Мирный житель "+ std::to_string(id) +" голосует за игрока " + std::to_string(chosenTarget) + ".");
        //return chosenTarget;
        co_return;
    }

    long getAim() const override{
        return -1; 
    }

    long getTarget() const override {
        return target;
    }

    int getCounter() const {
        return -1; 
    }

    std::string role() const override { return "Мирный житель"; }
};


class Commissioner:public Player{
    public:
    long aim = -1;
    long prov = -1;
    long findm = -1;
    long target = -1; 
    int counter=0;
    Move skill(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round) override {
        aim = findm;
        const_cast<Commissioner*>(this)->aim = findm;
        if(aim != -1 && players.find(aim) != players.end()) {
            logger.logRound(round, "Комиссар решил застрелить игрока " + std::to_string(aim) + " ибо он мафия.");
            //return aim;
            co_return;
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
                if (role == "Мафиози" || role == "Босс мафии") {
                    findm = chosenTarget;
                    logger.logRound(round, "Комиссар проверил игрока " + std::to_string(chosenTarget) + " и обнаружил, что он мафия.");
                } else {
                    logger.logRound(round, "Комиссар проверил игрока " + std::to_string(chosenTarget) + " и обнаружил, что он не мафия.");
                }
        }
        prov=chosenTarget;
        //return chosenTarget;
        co_return;
        }

        
    }
    


    Move vote(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round) override {
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
        const_cast<Commissioner*>(this)->target = chosenTarget;
        logger.logRound(round, "Комиссар "+ std::to_string(id) +" голосует за игрока " + std::to_string(chosenTarget) + ".");
        //return chosenTarget;
        co_return;
    }

    long getTarget() const override {
        return target;
    }

    long getAim() const override{
        return aim; 
    }

    int getCounter() const {
        return -1; 
    }

    std::string role() const override { return "Комиссар"; }
};


class Maniac:public Player{
    public:
    long target = -1; 
    long aim = -1;
    int counter=0;
    Move skill(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round) override {
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
        const_cast<Maniac*>(this)->target = chosenTarget;
        logger.logRound(round, "Маньяк "+ std::to_string(id) +" выбрал игрока " + std::to_string(chosenTarget) + " для убийства.");
        //return chosenTarget;
        co_return;
    }

    Move vote(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& logger, int round) override {
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
        const_cast<Maniac*>(this)->target = chosenTarget;
        logger.logRound(round, "Маньяк "+ std::to_string(id) +" голосует за игрока " + std::to_string(chosenTarget) + ".");
        //return chosenTarget;
        co_return;
    }

    long getTarget() const override {
        return target;
    }

    long getAim() const override{
        return -1; 
    }

    int getCounter() const {
        return -1; 
    }

    std::string role() const override { return "Маньяк"; }
};

