/**
 * @file Account.hpp
 * @brief Управление финансовым счетом пользователя
 *
 * @details Класс Account инкапсулирует:
 * - Хранение списка транзакций
 * - Управление балансом счета
 * - Операции добавления/удаления транзакций
 * - Конвертацию валют
 * - Валидацию данных
 *
 * @section account_rules Правила работы со счетами
 * 1. Каждый счет имеет уникальное имя
 * 2. Баланс всегда синхронизирован с транзакциями
 * 3. Поддерживаются операции с разными валютами
 * 4. Все изменения потокобезопасны
 */

#pragma once
#include "Time_Manager.hpp"
#include <iostream>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <vector>
#include <mutex>

 /**
  * @class Account
  * @brief Класс для управления финансовым счетом
  *
  * @invariant
  * 1. Имя счета не пустое
  * 2. Баланс = сумма всех транзакций
  * 3. Все операции сохраняют целостность данных
  */
class Account {
private:
    std::string name;       ///< Название счета (уникальное)
    double balance;         ///< Текущий баланс (в базовой валюте)
    std::vector<Transaction> transactions;  ///< Список транзакций
    mutable std::mutex transactions_mutex;  ///< Мьютекс для потокобезопасности

    // Запрет копирования
    Account(const Account&) = delete;
    Account& operator=(const Account&) = delete;

public:
    /**
     * @brief Конструктор перемещения
     * @param other Объект для перемещения
     */
    Account(Account&&) = default;

    /**
     * @brief Оператор перемещения
     * @param other Объект для перемещения
     */
    Account& operator=(Account&&) = default;

    /**
     * @brief Конструктор по умолчанию
     * @details Создает счет с именем "Без названия" и нулевым балансом
     */
    Account() : name("Без названия"), balance(0) {}

    /**
     * @brief Основной конструктор
     * @param accountName Название счета
     * @throws std::invalid_argument Если имя пустое
     */
    explicit Account(const std::string& accountName) : name(accountName), balance(0) {
        if (accountName.empty()) {
            throw std::invalid_argument("Имя счета не может быть пустым");
        }
    }

    /// @name Основные операции
    /// @{
    /**
     * @brief Добавляет транзакцию на счет
     * @param t Транзакция для добавления
     * @throws std::invalid_argument При:
     * - Невалидной транзакции
     * - Дубликате ID транзакции
     *
     * @post Баланс автоматически пересчитывается
     * @note Потокобезопасная операция
     */
    void addTransaction(const Transaction& t);

    /**
     * @brief Удаляет транзакцию по ID
     * @param id Уникальный идентификатор транзакции
     * @return true если транзакция была найдена и удалена
     *
     * @post Баланс автоматически пересчитывается
     * @note Сложность: O(n)
     */
    bool removeTransaction(int id);

    /**
     * @brief Перемещает транзакции из другого счета
     * @param other Счет-источник (rvalue reference)
     *
     * @warning После перемещения исходный счет становится невалидным
     * @note Не потокобезопасен - должен вызываться при гарантированном отсутствии конкурентного доступа
     */
    void move_transactions_from(Account&& other);
    /// @}

    /// @name Баланс и валидация
    /// @{
    /**
     * @brief Пересчитывает текущий баланс
     * @param converter Конвертер валют для пересчета
     *
     * @details Суммирует все транзакции с учетом:
     * - Типа (доход/расход)
     * - Валюты (конвертация в базовую)
     *
     * @note Используется при загрузке данных
     */
    void recalculateBalance(const CurrencyConverter& converter);

    /**
     * @brief Проверяет корректность баланса
     * @param converter Конвертер валют для проверки
     * @return true если баланс соответствует сумме транзакций
     *
     * @note Допустимая погрешность: 0.01 (ошибки округления)
     */
    bool validate(const CurrencyConverter& converter) const;

    /**
     * @brief Возвращает баланс в указанной валюте
     * @param converter Конвертер валют
     * @param currency Целевая валюта
     * @return Суммарный баланс в указанной валюте
     *
     * @note Конвертирует каждую транзакцию отдельно для точности
     */
    double get_balance_in_currency(const CurrencyConverter& converter,
        const std::string& currency) const;
    /// @}

    /// @name Геттеры
    /// @{
    /**
     * @brief Возвращает название счета
     * @return Текущее название
     */
    std::string get_name() const { return name; }

    /**
     * @brief Возвращает текущий баланс
     * @return Баланс в базовой валюте
     */
    double get_balance() const { return balance; }

    /**
     * @brief Возвращает список транзакций
     * @return Константная ссылка на вектор транзакций
     */
    const std::vector<Transaction>& get_transactions() const { return transactions; }
    /// @}

    /// @name Сеттеры
    /// @{
    /**
     * @brief Устанавливает новое название счета
     * @param newName Новое название
     * @throws std::invalid_argument Если имя пустое
     */
    void set_name(const std::string& newName) {
        if (newName.empty()) throw std::invalid_argument("Имя счета не может быть пустым");
        name = newName;
    }
    /// @}

    /**
     * @brief Объединяет два счета
     * @param other Счет для объединения
     * @param converter Конвертер валют для пересчета
     *
     * @details Переносит все транзакции из other в текущий счет
     * @post После объединения other остается пустым
     */
    void merge_account(Account&& other, const CurrencyConverter& converter);
};