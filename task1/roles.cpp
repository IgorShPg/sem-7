#include <vector>
#include <random>
#include <algorithm>
#include <iostream>

template <typename T>
T choose_random(const std::vector<T>& players) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    if (players.empty()) {
        throw std::invalid_argument("players vector is empty");
    }
    std::vector<T> sample(1);
    std::sample(players.begin(), players.end(), sample.begin(), 1, gen);
    return sample[0];
}


class Player {
public:
    //virtual void move(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& Log, int round) const = 0; 
    //virtual void vote(const std::map<int, mafia::shared_ptr<Player>>& players, int id, Logger& Log, int round) const = 0; 
    virtual std::string role() const = 0;
    virtual ~Player() = default;
};


class Mafia: public Player{
    std::string role() const override { return "Мафиози"; }
};


class Civilian:public Player{
    std::string role() const override { return "Мирный житель"; }
};


class Commissioner:public Player{
    std::string role() const override { return "Комиссар"; }
};


class Maniac:public Player{
    std::string role() const override { return "Маньяк"; }
};

