/**
 * @file FinanceCore.cpp
 * @brief Реализация методов FinanceCore
 */

#pragma once
#include "FinanceCore.hpp"
#include <thread>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <numeric>
#ifdef _WIN32
#include <windows.h> // Для очистки консоли
#endif

 /**
  * @brief Получает текущий активный счет
  * @details Если текущий счет не установлен, использует "Общий"
  * @return Ссылка на текущий счет
  * @warning Не возвращает nullptr (гарантирует валидный счет)
  */
Account& FinanceCore::getCurrentAccount() {
    if (currentAccount == nullptr) {
        currentAccount = &accounts["Общий"];
    }
    return *currentAccount;
}

/**
 * @brief Гарантирует наличие счета по умолчанию
 * @post Если accounts пуст, создается счет "Общий"
 * @post currentAccount никогда не будет nullptr
 */
void FinanceCore::ensureDefaultAccount() {
    if (accounts.empty()) {
        accounts["Общий"] = Account("Общий");
    }
    if (currentAccount == nullptr) {
        currentAccount = &accounts["Общий"];
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
    accounts["Общий"] = Account("Общий");
    currentAccount = &accounts["Общий"];

    std::filesystem::path dataPath;

    try {
        // Linux-способ
        dataPath = std::filesystem::canonical("/proc/self/exe").parent_path() / "transactions.dat";
    }
    catch (...) {
        try {
            // Fallback для Windows и других систем
            dataPath = std::filesystem::current_path() / "transactions.dat";
        }
        catch (...) {
            // Аварийный fallback
            dataPath = "transactions.dat";
        }
    }

    dataFile = dataPath.string(); // Сохраняем как строку

    std::cout << "Файл данных будет сохранен в: " << dataFile << std::endl;

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
    if (accounts.empty()) return false;
    for (const auto& [name, account] : accounts) {
        double balance = 0;
        for (const auto& t : account.get_transactions()) {
            balance += t.get_signed_amount();
        }
        if (std::abs(balance - account.get_balance()) > 0.01) {
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
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
