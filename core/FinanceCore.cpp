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

Account& FinanceCore::getCurrentAccount() {
    if (currentAccount == nullptr) {
        currentAccount = &accounts["Общий"];
    }
    return *currentAccount;
}

void FinanceCore::ensureDefaultAccount() {
    if (accounts.empty()) {
        accounts["Общий"] = Account("Общий");
    }
    if (currentAccount == nullptr) {
        currentAccount = &accounts["Общий"];
    }
}

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

void FinanceCore::clearConsole() const {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void FinanceCore::clearInputBuffer() const {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}