/**
 * @file FinanceCore.cpp
 * @brief Реализация методов FinanceCore
 */

#pragma once
#include "FinanceCore.hpp"
#include "currency/CurrencyFetcher.hpp"
#include <thread>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <numeric>
#include <future>
#ifdef _WIN32
#include <windows.h> // Для очистки консоли
#endif

 /**
  * @brief Получает текущий активный счет
  * @details Если текущий счет не установлен, использует "Общий"
  * @return Ссылка на текущий счет
  * @warning Не возвращает nullptr (гарантирует валидный счет)
  */

std::string FinanceCore::getDataPath(const std::string& filename) {
    std::filesystem::path exePath = std::filesystem::current_path(); 
    std::filesystem::path dataPath = exePath / "data" / filename; 
    return dataPath.string();
}

Account& FinanceCore::getCurrentAccount() {
   // std::lock_guard<std::mutex> lock(accounts_mutex_);
    if (!currentAccount) {
        currentAccount = &accounts.at("Общий");
    }
    return *currentAccount;
}

/**
 * @brief Гарантирует наличие счета по умолчанию
 * @post Если accounts пуст, создается счет "Общий"
 * @post currentAccount никогда не будет nullptr
 */
void FinanceCore::ensureDefaultAccount() {
   // std::lock_guard<std::mutex> lock(accounts_mutex_);
    if (accounts.empty()) {
        accounts.try_emplace("Общий", "Общий");
    }
    if (!currentAccount) {
        currentAccount = &accounts.at("Общий");
    }
}

/**
 * @brief Конструктор инициализирует систему
 * @details Определяет путь к файлу данных в порядке приоритета:
 * 1. Путь к исполняемому файлу (Linux)
 * 2. Текущая директория
 * 3. Аварийный fallback
 *
 * @note Автоматически вызывает loadData()
 */
FinanceCore::FinanceCore() {
    std::cout << "[DEBUG] Путь к файлу данных: "
        << std::filesystem::absolute(dataFile) << "\n";
    // std::lock_guard<std::mutex> lock(accounts_mutex_);
    accounts.try_emplace("Общий", "Общий");
    currentAccount = &accounts.at("Общий");

    std::filesystem::path dataPath;

    try {
        // Linux-способ
        dataPath = std::filesystem::canonical("/proc/self/exe").parent_path() / "transactions.dat";
    }
    catch (...) {
        try {
            // Fallback для Windows и других систем
            dataPath = getDataPath("transactions.dat");
        }
        catch (...) {
            // Аварийный fallback
            dataPath = "transactions.dat";
        }
    }

    dataFile = dataPath.string(); // Сохраняем как строку

    std::cout << "Файл данных будет сохранен в: " << dataFile << std::endl;

    ensureDataDirectory();

    std::filesystem::create_directories("CurrencyDat");

    try {
        std::promise<bool> promise;
        auto future = promise.get_future();
        update_currency_rates([&promise](bool success) {
            promise.set_value(success);
            });
        future.wait();

        if (!future.get()) {
            std::cout << "Предупреждение: не удалось обновить курсы валют\n";
        }
    }
    catch (...) {
        std::cerr << "Ошибка при обновлении курсов валют\n";
    }

    // Отладочный вывод загруженных данных
    std::cout << "[DEBUG] Загружено аккаунтов: " << accounts.size() << "\n";
    for (const auto& [name, account] : accounts) {
        std::cout << "[DEBUG] Аккаунт: " << name
            << ", транзакций: " << account.get_transactions().size() << "\n";
    }

    ensureDefaultAccount();

    loadData();

}


/**
 * @brief Валидирует финансовые данные
 * @details Проверяет:
 * - Наличие хотя бы одного счета
 * - Корректность балансов (сумма транзакций == балансу)
 * @return true если все данные корректны
 */
bool FinanceCore::validateData() const {
  //  std::lock_guard<std::mutex> lock(accounts_mutex_);

    if (accounts.empty()) return false;

    for (const auto& [name, account] : accounts) {
        double calculated = 0;
        for (const auto& t : account.get_transactions()) {
            calculated += t.get_amount_in_rub(currency_converter_);
        }

        if (std::abs(calculated - account.get_balance()) > 0.01) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Получает выбор пользователя из меню
 * @return Числовой выбор (1-N)
 * @note Цикл продолжается до получения корректного ввода
 */
int FinanceCore::getMenuChoice() const {
    int choice;
    while (true) {
        if (std::cin >> choice) {
            clearInputBuffer();
            return choice;
        }
        else {
            std::cout << "Ошибка ввода. Пожалуйста, введите число: ";
            std::cin.clear();
            clearInputBuffer();
        }
    }
}

/**
 * @brief Очищает консоль (кроссплатформенно)
 * @details Использует:
 * - "cls" на Windows
 * - "clear" на Unix-системах
 */
void FinanceCore::clearConsole() const {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/**
 * @brief Очищает буфер ввода
 * @details Используется после cin >> для предотвращения проблем с вводом
 */
void FinanceCore::clearInputBuffer() const {
    std::cin.clear();
    if (std::cin.rdbuf()->in_avail() > 0) {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

void FinanceCore::update_currency_rates(std::function<void(bool)> callback) {
    CurrencyFetcher fetcher;
    fetcher.fetch_rates([this, callback](const auto& new_rates) {
        bool success = false;

        // 1. Обновляем курсы через публичный метод
        if (!new_rates.empty()) {
            currency_converter_.set_rates(new_rates); 
            currency_converter_.save_rates_to_file("CurrencyDat/currency_rates.json");
            success = true;
        }
        else {
            success = currency_converter_.load_rates_from_file("CurrencyDat/currency_rates.json");
        }

        // 2. Пересчет балансов
        if (success) {
            std::lock_guard<std::mutex> acc_lock(accounts_mutex_);
            for (auto& [name, account] : accounts) {
                account.recalculateBalance(currency_converter_);
            }
        }

        callback(success);
        });
}

double FinanceCore::convert_currency(double amount, const std::string& from,
    const std::string& to) const {
    return currency_converter_.convert(amount, from, to);
}

void FinanceCore::setBaseCurrency(const std::string& currency) {
    if (currency_converter_.is_currency_supported(currency)) {
        base_currency_ = currency;
    }
    else {
        throw std::invalid_argument("Валюта не поддерживается");
    }
}

std::string FinanceCore::getBaseCurrency() const {
    return base_currency_;
}
