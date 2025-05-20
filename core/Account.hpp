/**
 * @file Account.hpp
 * @brief Заголовочный файл класса Account для управления финансовым счетом
 * @author Сонин Михаил/Эксузян Давид
 * @version 1.0
 * @date 2025-05-15
 */

 /// @see Transaction Класс транзакции
 /// @see FinanceCore Основной класс приложения
#pragma once
#include "Time_Manager.hpp"
#include <iostream>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <vector>

 /**
  * @class Account
  * @brief Класс для управления финансовым счетом пользователя
  *
  * Предоставляет функционал для:
  * - Учета транзакций (доходы/расходы)
  * - Управления балансом счета
  * - Переименования счета
  */
class Account {
private:
    std::string name;       ///< Название счета
    double balance;         ///< Текущий баланс (в базовой валюте)
    std::vector<Transaction> transactions;  //!< Список транзакций
    mutable std::mutex transactions_mutex;

    Account(const Account&) = delete;
    Account& operator=(const Account&) = delete;

public:

    Account(Account&&) = default;
    Account& operator=(Account&&) = default;

    /**
     * @brief Конструктор по умолчанию
     * @details Создает счет с именем "Без названия" и нулевым балансом
     */
    Account() : name("Без названия"), balance(0) {}

    /**
     * @brief Конструктор с именем счета
     * @param accountName Название создаваемого счета
     */
    explicit Account(const std::string& accountName) : name(accountName), balance(0) {}

    /**
     * @brief Добавляет транзакцию на счет
     * @param t Объект транзакции для добавления
     * @post Баланс счета автоматически пересчитывается
     */
    void addTransaction(const Transaction& t);

    /**
     * @brief Удаляет транзакцию по ID
     * @param id Уникальный идентификатор транзакции
     * @return true если транзакция была найдена и удалена
     * @post Баланс счета автоматически пересчитывается
     */
    bool removeTransaction(int id);

    /**
     * @brief Перемещает транзакции из другого счета
     * @param other Другой счет (rvalue reference)
     * @warning После перемещения исходный счет становится невалидным
     */
    void move_transactions_from(Account&& other);

    /**
     * @brief Пересчитывает текущий баланс
     * @details Суммирует все транзакции для вычисления актуального баланса
     */
    void recalculateBalance(const CurrencyConverter& converter);

    /**
     * @brief Получает название счета
     * @return Текущее название счета
     */
    std::string get_name() const { return name; }

    /**
     * @brief Получает текущий баланс
     * @return Баланс в базовой валюте
     */
    double get_balance() const { return balance; }

    /**
     * @brief Устанавливает новое название счета
     * @param newName Новое название счета
     */
    void set_name(const std::string& newName) { name = newName; }

    /**
     * @brief Получает список транзакций
     * @return Константная ссылка на вектор транзакций
     */
    const std::vector<Transaction>& get_transactions() const { return transactions; }

    double get_balance_in_currency(const CurrencyConverter& converter,
        const std::string& currency) const;

    bool validate(const CurrencyConverter& converter) const;
    void merge_account(Account&& other, const CurrencyConverter& converter);
};