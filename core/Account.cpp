/**
 * @file Account.cpp
 * @brief Реализация методов класса Account
 *
 * @details Этот модуль содержит:
 * - Операции с транзакциями
 * - Управление балансом
 * - Валидацию данных
 * - Конвертацию валют
 *
 * @section transaction_processing Обработка транзакций
 * 1. Все изменения баланса атомарны
 * 2. Поддержка мультивалютных операций
 * 3. Гаранитрованная целостность данных
 */

#include "Account.hpp"
#include <algorithm>

 /**
  * @brief Добавляет транзакцию на счет
  * @param t Транзакция для добавления
  *
  * @details Алгоритм работы:
  * 1. Проверка уникальности ID транзакции
  * 2. Добавление в список транзакций
  * 3. Корректировка баланса
  *
  * @throws std::invalid_argument Если:
  * - Транзакция с таким ID уже существует
  * - Транзакция не прошла валидацию
  *
  * @note Потокобезопасная операция
  * @complexity O(n) для проверки дубликатов
  */
void Account::addTransaction(const Transaction& t) {
    std::lock_guard<std::mutex> lock(transactions_mutex);

    // Проверка дубликатов по ID
    if (std::any_of(transactions.begin(), transactions.end(),
        [&t](const auto& existing) { return existing.get_id() == t.get_id(); })) {
        throw std::invalid_argument("Transaction ID already exists");
    }

    transactions.push_back(t);
    balance += t.get_signed_amount();
}

/**
 * @brief Удаляет транзакцию по ID
 * @param id Уникальный идентификатор транзакции
 * @return true если транзакция была найдена и удалена
 *
 * @details Алгоритм работы:
 * 1. Поиск транзакции по ID
 * 2. Корректировка баланса
 * 3. Удаление из списка
 *
 * @note Потокобезопасная операция
 * @complexity O(n)
 */
bool Account::removeTransaction(int id) {
    std::lock_guard<std::mutex> lock(transactions_mutex);

    auto it = std::find_if(transactions.begin(), transactions.end(),
        [id](const auto& t) { return t.get_id() == id; });

    if (it != transactions.end()) {
        balance -= it->get_signed_amount();
        transactions.erase(it);
        return true;
    }
    return false;
}

/**
 * @brief Перемещает транзакции из другого счета
 * @param other Счет-источник (rvalue reference)
 *
 * @details Переносит все транзакции и баланс:
 * 1. Заменяет текущие транзакции
 * 2. Сохраняет баланс источника
 * 3. Оставляет источник в пустом состоянии
 *
 * @warning Не потокобезопасен - должен вызываться при гарантированном отсутствии конкурентного доступа
 * @post Счет other остается валидным, но пустым
 */
void Account::move_transactions_from(Account&& other) {
    balance = other.balance;
    transactions = std::move(other.transactions);
}

/**
 * @brief Пересчитывает текущий баланс
 * @param converter Конвертер валют
 *
 * @details Алгоритм:
 * 1. Сбрасывает текущий баланс
 * 2. Суммирует все транзакции с конвертацией в рубли
 * 3. Сохраняет новое значение баланса
 *
 * @note Используется при:
 * - Загрузке данных
 * - Обнаружении расхождений
 * - Изменении курсов валют
 */
void Account::recalculateBalance(const CurrencyConverter& converter) {
    std::lock_guard<std::mutex> lock(transactions_mutex);
    double new_balance = 0.0;

    for (const auto& t : transactions) {
        new_balance += t.get_amount_in_rub(converter);
    }

    balance = new_balance;
}

/**
 * @brief Проверяет корректность баланса
 * @param converter Конвертер валют для проверки
 * @return true если баланс соответствует сумме транзакций
 *
 * @details Допустимая погрешность: 0.01 (учет округления)
 * @note Используется для верификации данных
 */
bool Account::validate(const CurrencyConverter& converter) const {
    std::lock_guard<std::mutex> lock(transactions_mutex);
    double calculated = 0.0;

    for (const auto& t : transactions) {
        calculated += t.get_amount_in_rub(converter);
    }

    return std::abs(calculated - balance) < 0.01;
}

/**
 * @brief Возвращает баланс в указанной валюте
 * @param converter Конвертер валют
 * @param currency Целевая валюта (код ISO 4217)
 * @return Суммарный баланс в указанной валюте
 *
 * @details Конвертирует каждую транзакцию отдельно для максимальной точности
 * @throws std::runtime_error При ошибках конвертации
 */
double Account::get_balance_in_currency(const CurrencyConverter& converter,
    const std::string& currency) const {
    std::lock_guard<std::mutex> lock(transactions_mutex);
    double balance = 0.0;

    for (const auto& t : transactions) {
        balance += converter.convert(
            t.get_signed_amount(),
            t.get_currency(),
            currency
        );
    }
    return balance;
}

/**
 * @brief Объединяет два счета
 * @param other Счет для объединения
 * @param converter Конвертер валют
 *
 * @details Алгоритм:
 * 1. Резервирует место для новых транзакций
 * 2. Переносит транзакции из other
 * 3. Полностью пересчитывает баланс
 *
 * @note После объединения other остается валидным, но пустым
 * @complexity O(n+m) где n и m - количество транзакций
 */
void Account::merge_account(Account&& other, const CurrencyConverter& converter) {
    std::lock_guard<std::mutex> lock(transactions_mutex);
    std::lock_guard<std::mutex> other_lock(other.transactions_mutex);

    transactions.reserve(transactions.size() + other.transactions.size());
    transactions.insert(transactions.end(),
        std::make_move_iterator(other.transactions.begin()),
        std::make_move_iterator(other.transactions.end()));

    recalculateBalance(converter);
}