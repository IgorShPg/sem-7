#include <algorithm>
#include <climits>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <limits>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>



class Solution {
public:
    virtual int score() = 0;  
    virtual void print() const = 0;   
	virtual Solution* clone() const = 0;
	virtual ~Solution() = default;
};


class SolutionAnalyze : public Solution {
public:
    std::vector<std::vector<int>> schedule;
    std::vector<int> job_times;

    SolutionAnalyze(int N, int M, std::vector<int>& jobs) : job_times(jobs) {
        schedule.resize(M, std::vector<int>());

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, M - 1);

        for (int job_index = 0; job_index < N; ++job_index) {
            int processor_index = distrib(gen);
            schedule[processor_index].push_back(job_times[job_index]);
        }

    }
    
	

    int score() override {
        std::vector<int> completion_times(schedule.size(), 0);
        for (int i = 0; i < schedule.size(); ++i) {
            for (int j = 0; j < schedule[i].size(); ++j) {
                completion_times[i] += schedule[i][j];
                }
        }
        
        int Tmax = *max_element(completion_times.begin(), completion_times.end());
        int Tmin = *min_element(completion_times.begin(), completion_times.end());
		return Tmax - Tmin;
    }

    void print() const override {
        for (int i = 0; i < schedule.size(); ++i) {
            std::cout <<"Proc "<<i<<": ";
            for (int j = 0; j < schedule[i].size(); ++j) {
                std::cout<<schedule[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

    SolutionAnalyze* clone() const override {
        return new SolutionAnalyze(*this);
    }
};



class IMutation {
public:
	virtual ~IMutation() = default;
    virtual void mutate(SolutionAnalyze& solution) = 0; 
};


class Mutator : public IMutation {
public:
    void mutate(SolutionAnalyze& solution) override;
};


void Mutator::mutate(SolutionAnalyze& solution) {
    SolutionAnalyze& sched = solution;
    int M = sched.schedule.size();
    std::random_device rd;
    std::mt19937 gen(rd());

    int processor_index_from = -1;
    do {
        std::uniform_int_distribution<> processor_distrib(0, M - 1);
        processor_index_from = processor_distrib(gen);
    } while (sched.schedule[processor_index_from].empty());

    std::uniform_int_distribution<> job_distrib(0, sched.schedule[processor_index_from].size() - 1);
    int job_index = job_distrib(gen);
    int job_time = std::move(sched.schedule[processor_index_from][job_index]);

    int processor_index_to = -1;
    do {
        std::uniform_int_distribution<> processor_distrib(0, M - 1);
        processor_index_to = processor_distrib(gen);
    } while (processor_index_to == processor_index_from);
    sched.schedule[processor_index_to].push_back(job_time);
    sched.schedule[processor_index_from].erase(sched.schedule[processor_index_from].begin() + job_index);
}




class Temperatura{
public:
    double temperature;
    virtual ~Temperatura() = default;
    virtual double changeTemperatura(int iteration) = 0;
};


class BoltzmannTemperatureDecrease : public Temperatura {
public:
    double temperature;
    BoltzmannTemperatureDecrease(double temp) : temperature(temp) {}
    double changeTemperatura(int iteration) override {
        return temperature / std::log(1 + iteration);
    }
};


class LogTemperatureDecrease : public Temperatura {
public:
    double temperature;
    LogTemperatureDecrease(double temp) : temperature(temp) {}
    double changeTemperatura(int iteration) override {
        return temperature  * (std::log(1 + iteration) / (1 + iteration));
    }
};



class CauchyTemperatureDecrease : public Temperatura {
public:
    double temperature;
    CauchyTemperatureDecrease(double temp) : temperature(temp) {}
    double changeTemperatura(int iteration) override {
        return temperature  / (1 + iteration);
    }
};



class Simulation {
private:
    Temperatura* temp;
    IMutation* change;
    SolutionAnalyze* global_best;
    int score;
    int no_improvement;

    public:
    Simulation(Temperatura* cooling, IMutation* mutation)
        : temp(cooling), change(mutation) {
            no_improvement=0;
		}

    void work(SolutionAnalyze* init) {
        global_best=init->clone();
		score = global_best->score();
    	double temperature = temp->temperature;
    	for (int i = 0; no_improvement < 10; ++i) {
    	    SolutionAnalyze* new_solution = global_best->clone();
    	    change->mutate(*new_solution);

    	    int new_cost = new_solution->score();
    	    double probability = exp((score - new_cost) / temperature);
    	    if (new_cost < score || (probability > ((double)rand() / RAND_MAX))) {
    	        delete global_best;
    	        global_best = new_solution->clone();
    	        score = new_cost;
                no_improvement = 0; 
    	    } else {
    	        delete new_solution;
                no_improvement++;
    	    }
    	    temperature = temp->changeTemperatura(i + 1);
    	} 
    }

    SolutionAnalyze* best_result() {
        return global_best->clone();
    }
};



std::vector<int> generate(int N) {
    std::vector<int> vec;
    vec.reserve(N); 
    for (int i = 0; i < N; ++i) {
        int randomNumber = rand() % 100 + 1;  
        vec.push_back(randomNumber);
    }
    return vec;
}



std::mutex mtx;
int main(int argc, char* argv[]) {
  
    int num_threads = std::stoi(argv[1]);
    int N = std::stoi(argv[2]);  
    int M = std::stoi(argv[3]);  
    std::vector<int> jobs = generate(N);  

    auto initial_solution = std::make_unique<SolutionAnalyze>(N, M, jobs);
    auto best_global = initial_solution.get();
    
    best_global->print();
    std::cout <<best_global->score()<< std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&initial_solution, &best_global, i]() {
            auto col = std::make_unique<LogTemperatureDecrease>(100.0); 
            auto mut = std::make_unique<Mutator>(); 
            Simulation Annealing(col.get(), mut.get());
            Annealing.work(initial_solution.get());
            SolutionAnalyze* best_local_ptr = Annealing.best_result();
            SolutionAnalyze* best_local = best_local_ptr; 
            std::lock_guard<std::mutex> lock(mtx);

            if (best_local->score() < best_global->score()) {
                best_global = std::move(best_local);
            }
        });
    }   


    for (auto& thread : threads) {
        thread.join();
    } 

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout<< diff.count()<< std::endl;

    std::cout << "Окончательное расписание:" << std::endl;
    best_global->print();
	std::cout << best_global->score() << std::endl;

    return 0;
    
}








