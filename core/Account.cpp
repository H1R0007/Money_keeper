/**
 * @file Account.cpp
 * @brief Реализация методов класса Account
 * @author Сонин Михаил/Эксузян Давид
 * @date 2025-05-16
 */

#include "Account.hpp"

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
    transactions.push_back(t);
    balance += t.get_signed_amount();
}

/**
 * @brief Удаляет транзакцию по ID
 * @param id Уникальный идентификатор транзакции
 * @note Сложность: O(n), где n - количество транзакций
 */
void Account::removeTransaction(int id) {
    auto it = std::find_if(transactions.begin(), transactions.end(),
        [id](const Transaction& t) { return t.get_id() == id; });

    if (it != transactions.end()) {
        balance -= it->get_signed_amount();
        transactions.erase(it);
    }
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
void Account::recalculateBalance() {
    balance = 0;
    for (const auto& t : transactions) {
        balance += t.get_signed_amount();
    }
}