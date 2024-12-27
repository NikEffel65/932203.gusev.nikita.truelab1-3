#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

class Monitor {
private:
    std::mutex mtx;
    std::condition_variable cv;
    bool eventReady = false; // Индикатор готовности события

public:
    // Функция-поставщик
    void producer() {
        while (true) {
            // Имитируем задержку в 1 секунду
            std::this_thread::sleep_for(std::chrono::seconds(1));

            std::unique_lock<std::mutex> lock(mtx);
            eventReady = true;
            std::cout << "Event produced" << std::endl;

            // Уведомляем поток-потребитель
            cv.notify_one();
        }
    }

    // Функция-потребитель
    void consumer() {
        while (true) {
            std::unique_lock<std::mutex> lock(mtx);

            // Ожидание события с временным освобождением мьютекса
            cv.wait(lock, [this]() { return eventReady; });

            // Обработка события
            std::cout << "Event consumed" << std::endl;
            eventReady = false;
        }
    }
};

int main() {
    Monitor monitor;

    // Создаём потоки для поставщика и потребителя
    std::thread producerThread(&Monitor::producer, &monitor);
    std::thread consumerThread(&Monitor::consumer, &monitor);

    // Ждём завершения потоков (в данном случае они бесконечны, поэтому программа
    // будет работать до принудительного завершения)
    producerThread.join();
    consumerThread.join();

    return 0;
}
