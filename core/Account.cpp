/**
 * @file Account.cpp
 * @brief ���������� ������� ������ Account
 *
 * @details ���� ������ ��������:
 * - �������� � ������������
 * - ���������� ��������
 * - ��������� ������
 * - ����������� �����
 *
 * @section transaction_processing ��������� ����������
 * 1. ��� ��������� ������� ��������
 * 2. ��������� �������������� ��������
 * 3. ��������������� ����������� ������
 */

#include "Account.hpp"
#include <algorithm>

 /**
  * @brief ��������� ���������� �� ����
  * @param t ���������� ��� ����������
  *
  * @details �������� ������:
  * 1. �������� ������������ ID ����������
  * 2. ���������� � ������ ����������
  * 3. ������������� �������
  *
  * @throws std::invalid_argument ����:
  * - ���������� � ����� ID ��� ����������
  * - ���������� �� ������ ���������
  *
  * @note ���������������� ��������
  * @complexity O(n) ��� �������� ����������
  */
void Account::addTransaction(const Transaction& t) {
    std::lock_guard<std::mutex> lock(transactions_mutex);

    // �������� ���������� �� ID
    if (std::any_of(transactions.begin(), transactions.end(),
        [&t](const auto& existing) { return existing.get_id() == t.get_id(); })) {
        throw std::invalid_argument("Transaction ID already exists");
    }

    transactions.push_back(t);
    balance += t.get_signed_amount();
}

/**
 * @brief ������� ���������� �� ID
 * @param id ���������� ������������� ����������
 * @return true ���� ���������� ���� ������� � �������
 *
 * @details �������� ������:
 * 1. ����� ���������� �� ID
 * 2. ������������� �������
 * 3. �������� �� ������
 *
 * @note ���������������� ��������
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
 * @brief ���������� ���������� �� ������� �����
 * @param other ����-�������� (rvalue reference)
 *
 * @details ��������� ��� ���������� � ������:
 * 1. �������� ������� ����������
 * 2. ��������� ������ ���������
 * 3. ��������� �������� � ������ ���������
 *
 * @warning �� ��������������� - ������ ���������� ��� ��������������� ���������� ������������� �������
 * @post ���� other �������� ��������, �� ������
 */
void Account::move_transactions_from(Account&& other) {
    balance = other.balance;
    transactions = std::move(other.transactions);
}

/**
 * @brief ������������� ������� ������
 * @param converter ��������� �����
 *
 * @details ��������:
 * 1. ���������� ������� ������
 * 2. ��������� ��� ���������� � ������������ � �����
 * 3. ��������� ����� �������� �������
 *
 * @note ������������ ���:
 * - �������� ������
 * - ����������� �����������
 * - ��������� ������ �����
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
 * @brief ��������� ������������ �������
 * @param converter ��������� ����� ��� ��������
 * @return true ���� ������ ������������� ����� ����������
 *
 * @details ���������� �����������: 0.01 (���� ����������)
 * @note ������������ ��� ����������� ������
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
 * @brief ���������� ������ � ��������� ������
 * @param converter ��������� �����
 * @param currency ������� ������ (��� ISO 4217)
 * @return ��������� ������ � ��������� ������
 *
 * @details ������������ ������ ���������� �������� ��� ������������ ��������
 * @throws std::runtime_error ��� ������� �����������
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
 * @brief ���������� ��� �����
 * @param other ���� ��� �����������
 * @param converter ��������� �����
 *
 * @details ��������:
 * 1. ����������� ����� ��� ����� ����������
 * 2. ��������� ���������� �� other
 * 3. ��������� ������������� ������
 *
 * @note ����� ����������� other �������� ��������, �� ������
 * @complexity O(n+m) ��� n � m - ���������� ����������
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