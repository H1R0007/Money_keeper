#pragma once
#include "core/FinanceCore.hpp"
#include <iostream>
#include <cstdlib>
#include <clocale>
#include <Windows.h> // Только для Windows

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int main() {


    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, "Russian");

    try {
        clearScreen();
        FinanceCore manager;

        std::cout << "=== Финансовый менеджер ===\n"
            << "Добро пожаловать!\n"
            << "Используйте меню для навигации\n\n";

        manager.runMainMenu();

        std::cout << "\nДанные сохранены.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "\nКритическая ошибка: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
