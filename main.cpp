#include <iostream>
#include <vector>
#include <algorithm>
#include <curl/curl.h>
#include <chrono>

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    return size * nmemb; // Игнорируем полученные данные
}

// Функция для вычисления медианы
long long median(std::vector<long long>& times) {
    std::sort(times.begin(), times.end());
    size_t len = times.size();
    if (len % 2 == 0) {
        return (times[len / 2 - 1] + times[len / 2]) / 2;
    }
    return times[len / 2];
}

int main() {
    CURL* curl;
    CURLcode res;
    std::vector<long long> times;
    int repeats = 10;

    // Инициализация CURL
    curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL initialization failed!" << std::endl;
        return 1;
    }

    // Установка URL
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.bybit.com/v2/public/time");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    // Прогревочный запрос
    std::cout << "Skipping first request (warm-up)..." << std::endl;
    res = curl_easy_perform(curl);

    // Основные запросы
    for (int i = 0; i < repeats; ++i) {
        auto start = std::chrono::high_resolution_clock::now();

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Request " << i + 1 << " failed: " << curl_easy_strerror(res) << std::endl;
            continue;
        }

        auto end = std::chrono::high_resolution_clock::now();
        long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        times.push_back(duration);

        std::cout << "Request " << i + 1 << " time: " << duration << " ms" << std::endl;
    }

    // Вычисление минимального, максимального и медианного времени
    long long minTime = *std::min_element(times.begin(), times.end());
    long long maxTime = *std::max_element(times.begin(), times.end());
    long long medianTime = median(times);

    std::cout << "\nResults over " << repeats << " requests (excluding warm-up):" << std::endl;
    std::cout << "Min time: " << minTime << " ms" << std::endl;
    std::cout << "Max time: " << maxTime << " ms" << std::endl;
    std::cout << "Median time: " << medianTime << " ms" << std::endl;

    // Очистка ресурсов
    curl_easy_cleanup(curl);
    return 0;
}
