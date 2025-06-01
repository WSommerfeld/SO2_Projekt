#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <semaphore.h>
#include <cstdlib>
#include <chrono>
#include <cstdio>
#include <unistd.h>


int* meals;


sem_t* forks;


std::mutex print_mutex;

//waiter mutex
std::mutex waiter;

void eat(int id, int left, int right)
{
    waiter.lock();
    sem_wait(&forks[left]);
    sem_wait(&forks[right]);
    waiter.unlock();

    print_mutex.lock();
    printf("Philosopher %d is eating...\n", id + 1);
    print_mutex.unlock();

    meals[id]++;

    std::this_thread::sleep_for(std::chrono::milliseconds(100 + rand() % 100));

    sem_post(&forks[left]);
    sem_post(&forks[right]);
}

void think(int id)
{
    print_mutex.lock();
    printf("Philosopher %d is thinking...\n", id + 1);
    print_mutex.unlock();

    std::this_thread::sleep_for(std::chrono::milliseconds(100 + rand() % 100));
}

void whine(int id)
{
    print_mutex.lock();
    printf("Philosopher %d is hungry!\n", id + 1);
    print_mutex.unlock();
}

void philosopher(int id, int quantity)
{
    int left = id;
    int right = (id + 1) % quantity;

    while (true)
    {
        think(id);
        whine(id);
        eat(id, left, right);
    }
}

void stopper(int seconds, int quantity)
{
    int i = 0;
    while (i < seconds)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        i++;
    }

    print_mutex.lock();
    printf("Eaten meals:\n");
    for (int i = 0; i < quantity; i++)
    {
        printf("%3d ", i + 1);
    }
    printf("\n");
    for (int i = 0; i < quantity; i++)
    {
        printf("%3d ", meals[i]);
    }
    printf("\n");

    delete[] meals;
    print_mutex.unlock();

    exit(0);
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <number_of_philosophers> <seconds>\n";
        return 1;
    }

    int quantity = std::atoi(argv[1]);
    int seconds = std::atoi(argv[2]);

    if (quantity <= 1)
    {
        std::cerr << "Number of philosophers must be greater than 1\n";
        return 1;
    }


    meals = new int[quantity]{0};


	//forks vector (semaphores)
    forks = new sem_t[quantity];
    for (int i = 0; i < quantity; i++)
    {
        sem_init(&forks[i], 0, 1);
    }


	//philosopher threads vector
    std::vector<std::thread> philosophers;
    philosophers.reserve(quantity);
    for (int i = 0; i < quantity; i++)
    {
        philosophers.emplace_back(philosopher, i, quantity);
    }


    std::thread stop(stopper, seconds, quantity);


    stop.join();


    for (int i = 0; i < quantity; i++)
    {
        sem_destroy(&forks[i]);
    }
    delete[] forks;


    for (auto& p : philosophers)
    {
        if (p.joinable())
            p.join();
    }

    return 0;
}

