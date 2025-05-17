/**
 * @file FinanceCore.hpp
 * @brief Основной класс системы управления финансами
 * @author Сонин Михаил/Эксузян Давид
 * @version 1.0
 * @date 2025-05-16
 *
 * @section overview Обзор
 * Класс предоставляет полный функционал для:
 * - Управления счетами (создание, удаление, переименование)
 * - Обработки транзакций (доходы/расходы)
 * - Анализа финансовой статистики
 * - Сохранения/загрузки данных
 */

#pragma once
#include "Date.hpp"
#include "Time_Manager.hpp"
#include <vector>
#include <map>
#include <algorithm>
#include "Account.hpp"

 /**
  * @class FinanceCore
  * @brief Ядро финансового приложения
  *
  * @note Все методы потокобезопасны для базовых операций
  */
class FinanceCore {
private:
    std::vector<Transaction> transactions;       ///< Временное хранилище транзакций
    std::string dataFile;                       ///< Путь к файлу данных
    std::map<std::string, Account> accounts;    ///< Коллекция счетов (имя -> аккаунт)
    Account* currentAccount;                    ///< Текущий активный счет

    // Вспомогательные методы
    void printMainMenu() const;         ///< Выводит главное меню
    void printTransactionMenu() const;  ///< Выводит меню транзакций
    void printStatsMenu() const;        ///< Выводит меню статистики

public:
    /**
     * @brief Конструктор инициализирует систему
     * @details Автоматически:
     * 1. Создает счет "Общий"
     * 2. Определяет путь к файлу данных
     * 3. Загружает сохраненные данные
     */
    FinanceCore();

    // Управление данными
    /**
     * @brief Загружает данные из файла
     * @throws std::ios_base::failure При ошибках чтения
     */
    void loadData();

    /**
     * @brief Сохраняет все данные в файл
     * @throws std::ios_base::failure При ошибках записи
     */
    void saveData();

    // Управление счетами
    void renameAccount();   ///< Переименовывает текущий счет
    void deleteAccount();   ///< Удаляет выбранный счет
    void selectAccount();   ///< Выбирает активный счет
    void createAccount();   ///< Создает новый счет
    void manageAccounts();  ///< Меню управления счетами

    // Работа с транзакциями
    void removeTransaction();   ///< Удаляет транзакцию по ID
    void addTransaction();      ///< Добавляет новую транзакцию
    void viewAllTransactions() const;  ///< Показывает все транзакции
    void viewIncome() const;          ///< Показывает только доходы
    void viewExpenses() const;        ///< Показывает только расходы

    // Аналитика
    void showByCategory() const;       ///< Статистика по категориям
    void showTotalBalance() const;     ///< Общий баланс по всем счетам
    void showByMonth() const;          ///< Статистика по месяцам
    void showCurrentAccountStats() const;  ///< Статистика текущего счета

    /**
     * @brief Проверяет целостность данных
     * @return true если балансы всех счетов корректны
     */
    bool validateData() const;

    // Вспомогательные методы
    Account& getCurrentAccount();  ///< @return Текущий активный счет
    void ensureDefaultAccount();   ///< Гарантирует наличие счета "Общий"

    /**
     * @brief Фильтрует транзакции по типу
     * @param type Тип транзакции (доход/расход)
     * @return Вектор отфильтрованных транзакций
     */
    std::vector<Transaction> getFilteredTransactions(Transaction::Type type) const;

    /**
     * @brief Выводит таблицу транзакций
     * @param transactions Список транзакций
     * @param title Заголовок таблицы
     */
    void printTransactionsTable(const std::vector<Transaction>& transactions, const std::string& title) const;

    // Утилиты ввода/вывода
    int getMenuChoice() const;     ///< Получает выбор пользователя из меню
    void clearConsole() const;     ///< Очищает консоль (кроссплатформенно)
    void clearInputBuffer() const; ///< Очищает буфер ввода

    // Основные интерфейсы
    void runMainMenu();        ///< Запускает главное меню
    void runTransactionMenu(); ///< Запускает меню транзакций
    void runStatsMenu();       ///< Запускает меню статистики
};