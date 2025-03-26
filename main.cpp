#include <iostream>
#include <thread>
#include <semaphore.h>
#include <chrono>
#include <mutex>
#include <vector>

///kelner
std::mutex waiter;
///zamek do strumienia wyjścia
std::mutex print_mutex;
///Wektor semaforów
std::vector<sem_t> forks;
///tablica posiłków;
int* meals;

///Ucztowanie
void eat(int id, int left, int right)
{

    waiter.lock();
    sem_wait(&forks[left]);
    sem_wait(&forks[right]);
    waiter.unlock();

        print_mutex.lock();
        printf("Philosopher %d is eating...\n",id+1);
        print_mutex.unlock();

        meals[id]++;

        std::this_thread::sleep_for(std::chrono::milliseconds(100+rand()%100));

    sem_post(&forks[left]);
    sem_post(&forks[right]);
}

///Myślenie
void think(int id)
{

    print_mutex.lock();
    printf("Philosopher %d is thinking...\n",id+1);
    print_mutex.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(100+rand()%100));
}

///Narzekanie
void whine(int id)
{

    print_mutex.lock();
    printf("Philosopher %d is hungry!\n",id+1);
    print_mutex.unlock();
}

///Filozof
void philosopher(int id, int quantity)
{
    ///Widelce
    int left = id;
    int right = (id + 1) % quantity;

    while(true)
    {
        ///Myślenie
        think(id);

        ///Czekanie
        whine(id);

        ///Jedzenie
        eat(id,left, right);
    }
}

///Ograniczenie czasowe
void stopper(int seconds,int quantity){

    int i=0;
    while(i<seconds)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        i++;
    }

    print_mutex.lock();

    printf("Eaten meals:\n");
    for(int i =0;i<quantity;i++)
    {
        printf("%3d ", i+1);
    }
    printf("\n");
    for(int i =0;i<quantity;i++)
    {

        printf("%3d ", meals[i]);
    }
    delete[] meals;
    print_mutex.unlock();
    exit(0);

}

int main (int argv, char** args)
{
////////Obsługa wejścia

        ///Ilosc filozofów
        printf("Philosophers quantity: %s\n", args[1]);

        ///Brak liczby filozofów
        if (argv == 1) {
            printf("Incomplete command\n");
            return 0;
        }
        ///Liczba filozofów
        int quantity;

        ///Sprawdzenie czy wprowadzono liczbe
        try {
            quantity = std::stoi(args[1]);
        }
        catch (std::invalid_argument) {
            printf("Invalid argument\n");
            return 0;
        }


        ///Liczba filozofów mniejsza od 2
        if (quantity < 2) {
            printf("Philosophers quantity must be higher than 1\n");
            return 0;
        }

        ///Czas wykonywania
        int seconds;
        if (argv == 3) {
            try {
                seconds = std::stoi(args[2]);
            }
            catch (std::invalid_argument) {
                printf("Invalid argument\n");
                return 0;
            }

            if (seconds < 0) {
                seconds = 2137;
            }

        } else {
            seconds = 2137;
        }

////////////Wywołania wątków + tablica posiłków

    ///Zliczanie posiłków
    meals = new int[quantity];

    for(int i =0;i<quantity;i++)
    {
        meals[i]=0;
    }

    srand(time(NULL));

    ///Inicjalizacja semaforów
    forks.resize(quantity);
    for (int i = 0; i < quantity; ++i) {
        sem_init(&forks[i], 0, 1);
    }

    ///wywołania wątków
    std::thread stop(stopper,seconds,quantity);
    std::vector<std::thread> philosophers;
    philosophers.reserve(quantity);
    for(int i =0;i<quantity;i++)
    {
        philosophers.emplace_back(philosopher, i,quantity);
    }

    for(int i=0;i<quantity;i++)
    {
        philosophers[i].join();
    }
    stop.join();


    return 0;
}
