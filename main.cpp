/**
 * @file main.cpp
 * @brief Главный исполняемый файл финансового приложения
 *
 * @details Реализует:
 * - Точку входа в программу
 * - Обработку критических ошибок
 * - Настройку локализации
 * - Управление жизненным циклом приложения
 *
 * @section platform_spec Особенности платформы
 * Содержит платформозависимый код для очистки экрана консоли.
 * Поддерживает Windows и Unix-подобные системы.
 */

#pragma once
#define _CRT_SECURE_NO_WARNINGS  // Отключает предупреждения безопасности CRT для MSVC
#include <Windows.h>             // Заголовочный файл для Windows API
#include <clocale>               // Для работы с локализацией
#include <iostream>              // Стандартный ввод/вывод
#include "core/FinanceCore.hpp"  // Основной класс приложения
#include <cstdlib>               // Для EXIT_SUCCESS/FAILURE

 /**
  * @brief Очищает экран консоли
  *
  * @details Использует платформозависимые команды:
  * - "cls" для Windows
  * - "clear" для Unix-подобных систем
  *
  * @note Функция использует system(), что может быть уязвимостью
  * в production-коде. В данном случае безопасно.
  */
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/**
 * @brief Точка входа в программу
 * @return Код завершения программы (EXIT_SUCCESS/EXIT_FAILURE)
 *
 * @details Выполняет:
 * 1. Настройку русской локали
 * 2. Очистку экрана
 * 3. Создание и запуск основного менеджера приложения
 * 4. Обработку исключений верхнего уровня
 *
 * @section error_handling Обработка ошибок
 * Ловит все исключения типа std::exception и выводит
 * сообщение об ошибке перед завершением программы.
 *
 * @warning Не перехватывает другие типы исключений
 * (не унаследованные от std::exception)
 */
int main() {

    setlocale(LC_ALL, "Russian");
    try {
        clearScreen();
        FinanceCore manager; // Создание основного объекта приложения

        manager.runMainMenu(); // Запуск главного меню

    }
    catch (const std::exception& e) {
        std::cerr << "\nКритическая ошибка: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
