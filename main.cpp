#pragma once
#define _CRT_SECURE_NO_WARNINGS  
#include <Windows.h>          
#include <clocale>               
#include <iostream>              
#include "core/FinanceCore.hpp"
#include <cstdlib>

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int main() {

    setlocale(LC_ALL, "Russian");
    try {
        clearScreen();
        FinanceCore manager;

        manager.runMainMenu();

    }
    catch (const std::exception& e) {
        std::cerr << "\nКритическая ошибка: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
