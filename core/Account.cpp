/**
 * @file Account.cpp
 * @brief Реализация методов класса Account
 * @author Сонин Михаил/Эксузян Давид
 * @date 2025-05-16
 */

#include "Account.hpp"
#include <algorithm>

 /**
  * @brief Добавляет транзакцию на счёт
  * @param t Объект тразакции для добавления
  * @throws std::invalid_argument Если сумма транзакции некорректна
  *
  * @par Пример использования
  * @code
  * Account acc("Основной");
  * acc.addTransaction(Transaction(100.0, "Зарплата"));
  * @endcode
  */
void Account::addTransaction(const Transaction& t) {
    //std::lock_guard<std::mutex> lock(transactions_mutex);

    // Проверка дубликатов по ID
    if (std::any_of(transactions.begin(), transactions.end(),
        [&t](const auto& existing) { return existing.get_id() == t.get_id(); })) {
        throw std::invalid_argument("Transaction ID already exists");
    }

    transactions.push_back(t);
    balance += t.get_signed_amount(); // Для быстрого обновления
}


/**
 * @brief Удаляет транзакцию по ID
 * @param id Уникальный идентификатор транзакции
 * @note Сложность: O(n), где n - количество транзакций
 */
bool Account::removeTransaction(int id) {
   // std::lock_guard<std::mutex> lock(transactions_mutex);

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
 * @param other Другой счет (rvalue reference)
 * @post Исходный счет other остается пустым
 */
void Account::move_transactions_from(Account&& other) {
    balance = other.balance;
    transactions = std::move(other.transactions);
}

/**
 * @brief Пересчитывает текущий баланс
 * @details Используется при:
 * - Загрузке данных из файла
 * - Обнаружении расхождений в балансе
 */
void Account::recalculateBalance(const CurrencyConverter& converter) {
   // std::lock_guard<std::mutex> lock(transactions_mutex);
    double new_balance = 0.0;

    for (const auto& t : transactions) {
        new_balance += t.get_amount_in_rub(converter);
    }

    balance = new_balance;
}

bool Account::validate(const CurrencyConverter& converter) const {
   // std::lock_guard<std::mutex> lock(transactions_mutex);
    double calculated = 0.0;

    for (const auto& t : transactions) {
        calculated += t.get_amount_in_rub(converter);
    }

    return std::abs(calculated - balance) < 0.01; // Учитываем погрешность double
}

double Account::get_balance_in_currency(const CurrencyConverter& converter,
    const std::string& currency) const {
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

void Account::merge_account(Account&& other, const CurrencyConverter& converter) {
  //  std::lock_guard<std::mutex> lock(transactions_mutex);
  //  std::lock_guard<std::mutex> other_lock(other.transactions_mutex);

    transactions.reserve(transactions.size() + other.transactions.size());
    transactions.insert(transactions.end(),
        std::make_move_iterator(other.transactions.begin()),
        std::make_move_iterator(other.transactions.end()));

    recalculateBalance(converter); // Полный пересчёт
}
