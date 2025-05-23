/**
 * @file Account.hpp
 * @brief ���������� ���������� ������ ������������
 *
 * @details ����� Account �������������:
 * - �������� ������ ����������
 * - ���������� �������� �����
 * - �������� ����������/�������� ����������
 * - ����������� �����
 * - ��������� ������
 *
 * @section account_rules ������� ������ �� �������
 * 1. ������ ���� ����� ���������� ���
 * 2. ������ ������ ��������������� � ������������
 * 3. �������������� �������� � ������� ��������
 * 4. ��� ��������� ���������������
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
  * @brief ����� ��� ���������� ���������� ������
  *
  * @invariant
  * 1. ��� ����� �� ������
  * 2. ������ = ����� ���� ����������
  * 3. ��� �������� ��������� ����������� ������
  */
class Account {
private:
    std::string name;       ///< �������� ����� (����������)
    double balance;         ///< ������� ������ (� ������� ������)
    std::vector<Transaction> transactions;  ///< ������ ����������
    mutable std::mutex transactions_mutex;  ///< ������� ��� ������������������

    // ������ �����������
    Account(const Account&) = delete;
    Account& operator=(const Account&) = delete;

public:
    /**
     * @brief ����������� �����������
     * @param other ������ ��� �����������
     */
    Account(Account&&) = default;

    /**
     * @brief �������� �����������
     * @param other ������ ��� �����������
     */
    Account& operator=(Account&&) = default;

    /**
     * @brief ����������� �� ���������
     * @details ������� ���� � ������ "��� ��������" � ������� ��������
     */
    Account() : name("��� ��������"), balance(0) {}

    /**
     * @brief �������� �����������
     * @param accountName �������� �����
     * @throws std::invalid_argument ���� ��� ������
     */
    explicit Account(const std::string& accountName) : name(accountName), balance(0) {
        if (accountName.empty()) {
            throw std::invalid_argument("��� ����� �� ����� ���� ������");
        }
    }

    /// @name �������� ��������
    /// @{
    /**
     * @brief ��������� ���������� �� ����
     * @param t ���������� ��� ����������
     * @throws std::invalid_argument ���:
     * - ���������� ����������
     * - ��������� ID ����������
     *
     * @post ������ ������������� ���������������
     * @note ���������������� ��������
     */
    void addTransaction(const Transaction& t);

    /**
     * @brief ������� ���������� �� ID
     * @param id ���������� ������������� ����������
     * @return true ���� ���������� ���� ������� � �������
     *
     * @post ������ ������������� ���������������
     * @note ���������: O(n)
     */
    bool removeTransaction(int id);

    /**
     * @brief ���������� ���������� �� ������� �����
     * @param other ����-�������� (rvalue reference)
     *
     * @warning ����� ����������� �������� ���� ���������� ����������
     * @note �� ��������������� - ������ ���������� ��� ��������������� ���������� ������������� �������
     */
    void move_transactions_from(Account&& other);
    /// @}

    /// @name ������ � ���������
    /// @{
    /**
     * @brief ������������� ������� ������
     * @param converter ��������� ����� ��� ���������
     *
     * @details ��������� ��� ���������� � ������:
     * - ���� (�����/������)
     * - ������ (����������� � �������)
     *
     * @note ������������ ��� �������� ������
     */
    void recalculateBalance(const CurrencyConverter& converter);

    /**
     * @brief ��������� ������������ �������
     * @param converter ��������� ����� ��� ��������
     * @return true ���� ������ ������������� ����� ����������
     *
     * @note ���������� �����������: 0.01 (������ ����������)
     */
    bool validate(const CurrencyConverter& converter) const;

    /**
     * @brief ���������� ������ � ��������� ������
     * @param converter ��������� �����
     * @param currency ������� ������
     * @return ��������� ������ � ��������� ������
     *
     * @note ������������ ������ ���������� �������� ��� ��������
     */
    double get_balance_in_currency(const CurrencyConverter& converter,
        const std::string& currency) const;
    /// @}

    /// @name �������
    /// @{
    /**
     * @brief ���������� �������� �����
     * @return ������� ��������
     */
    std::string get_name() const { return name; }

    /**
     * @brief ���������� ������� ������
     * @return ������ � ������� ������
     */
    double get_balance() const { return balance; }

    /**
     * @brief ���������� ������ ����������
     * @return ����������� ������ �� ������ ����������
     */
    const std::vector<Transaction>& get_transactions() const { return transactions; }
    /// @}

    /// @name �������
    /// @{
    /**
     * @brief ������������� ����� �������� �����
     * @param newName ����� ��������
     * @throws std::invalid_argument ���� ��� ������
     */
    void set_name(const std::string& newName) {
        if (newName.empty()) throw std::invalid_argument("��� ����� �� ����� ���� ������");
        name = newName;
    }
    /// @}

    /**
     * @brief ���������� ��� �����
     * @param other ���� ��� �����������
     * @param converter ��������� ����� ��� ���������
     *
     * @details ��������� ��� ���������� �� other � ������� ����
     * @post ����� ����������� other �������� ������
     */
    void merge_account(Account&& other, const CurrencyConverter& converter);
};