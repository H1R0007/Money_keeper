/**
 * @file FinanceCore.hpp
 * @brief �������� ����� ������� ���������� ���������
 *
 * @details ���� �������, ��������������:
 * - ���������� ������� � ������������
 * - ���������� ��������� � ����������
 * - ����������/�������� ������
 * - �������� ��������
 * - ���������������� ����
 *
 * @section architecture_sec �����������
 * 1. ���������������� �������� ������ (std::map)
 * 2. ���������������� �������� (std::mutex)
 * 3. ���������� ���������� � ����������
 * 4. ��������� dependency injection ��� CurrencyConverter
 */

#pragma once
#include "Date.hpp"
#include "Time_Manager.hpp"
#include <vector>
#include <map>
#include <algorithm>
#include "Account.hpp"
#include "currency/CurrencyConverter.hpp"
#include <filesystem>
#include <future>

 /**
  * @class FinanceCore
  * @brief ����������� ����� ����������� ���������
  *
  * @invariant
  * 1. ������ ���������� ������� ���� ���� ("�����")
  * 2. currentAccount ������� �� nullptr
  * 3. ������� ������ ���������������� � ������������
  */
class FinanceCore {
private:
    std::string base_currency_ = "RUB";       ///< ������� ������ ��� ����������
    std::string dataFile;                     ///< ���� � ����� ������
    std::map<std::string, Account> accounts;  ///< ��������� ������
    Account* currentAccount;                  ///< ������� �������� ����
    CurrencyConverter currency_converter_;    ///< ��������� �����
    mutable std::mutex accounts_mutex_;      ///< ������� ��� ������������������

public:
    // ������������/���������
    FinanceCore(const FinanceCore&) = delete;
    FinanceCore& operator=(const FinanceCore&) = delete;
    FinanceCore(FinanceCore&&) = default;
    FinanceCore& operator=(FinanceCore&&) = default;

    void printMainMenu() const;

    /**
     * @brief �������� ����������� �������
     * @details ��������������:
     * 1. ����������� ���� "�����"
     * 2. ���� � ������
     * 3. ��������� ����������� ������
     * 4. ��������� ����� �����
     *
     * @throws std::runtime_error ��� ������� �������������
     */
    FinanceCore();

    /// @name ���������� �������
    /// @{
    /**
     * @brief ��������� ������ �� �����
     * @throws std::ios_base::failure ��� ������� ������
     * @post ��������������� ��� ����� � ����������
     */
    void loadData();

    /**
     * @brief ��������� ��� ������ � ����
     * @throws std::ios_base::failure ��� ������� ������
     * @note ������ �����: CSV � �������� �� ������
     */
    void saveData();
    /// @}

    /// @name ���������� �������
    /// @{
    /**
     * @brief ������� ����� ����
     * @throws std::invalid_argument ��� ��������� �����
     * @post ��������� ���� � ��������� accounts
     */
    void createAccount();

    /**
     * @brief ������� ��������� ����
     * @pre � ������� ������ �������� ������� ���� ����
     * @throws std::logic_error ��� ������� ������� ��������� ����
     */
    void deleteAccount();

    /**
     * @brief �������� �������� ����
     * @details ���������� ������������� ���� ������
     * @post ������������� currentAccount
     */
    void selectAccount();

    /**
     * @brief ��������������� ������� ����
     * @note ��� "������" ����� ������� �����
     * @throws std::invalid_argument ��� ��������� �����
     */
    void renameAccount();

    /**
     * @brief ������� ���� ���������� �������
     * @details ���������� create/delete/select/rename
     */
    void manageAccounts();
    /// @}

    /// @name ������ � ������������
    /// @{
    /**
     * @brief ��������� ����� ����������
     * @details ������������� ������������ ������:
     * 1. ����� ����
     * 2. ���� �����
     * 3. �������� ���������
     * 4. ��������� ����
     * 5. ���������� ��������
     * 6. ���������� ������
     */
    void addTransaction();

    /**
     * @brief ������� ���������� �� ID
     * @param id ���������� �������������
     * @return true ���� ���������� ������� � �������
     */
    void removeTransaction();

    /**
     * @brief ���������� ��� ���������� �������� �����
     * @details ��������������� ��������� �����
     */
    void viewAllTransactions() const;

    /**
     * @brief ���������� ������ �������� ����������
     */
    void viewIncome() const;

    /**
     * @brief ���������� ������ ��������� ����������
     */
    void viewExpenses() const;
    /// @}

    /// @name ��������� � ������
    /// @{
    /**
     * @brief ����� ������ �� ���� ������
     */
    void showTotalBalance() const;

    /**
     * @brief ���������� �� ����������
     * @details ���������� ���������� �� ����������
     */
    void showByCategory() const;

    /**
     * @brief ���������� ����������
     */
    void showByMonth() const;

    /**
     * @brief ���������� �� �������� �����
     */
    void showCurrentAccountStats() const;

    /**
     * @brief ������ � ������� �����
     */
    void showBalanceByCurrency() const;

    /**
     * @brief ����� ���������� �� �����
     * @param tags ������ ����� ��� ������
     */
    void searchByTags(const std::vector<std::string>& tags) const;
    /// @}

    /// @name �������� ��������
    /// @{
    /**
     * @brief ������������� ������� ������
     * @param currency ��� ������ (ISO 4217)
     * @throws std::invalid_argument ��� ���������������� �����
     */
    void setBaseCurrency(const std::string& currency);

    /**
     * @brief ���������� ������� ������� ������
     * @return ��� ������ (�������� "RUB")
     */
    std::string getBaseCurrency() const;

    /**
     * @brief ������������ ����� ����� ��������
     * @param amount �������� �����
     * @param from �������� ������
     * @param to ������� ������ (�� ��������� RUB)
     * @return ����������������� �����
     * @throws std::runtime_error ��� ������� �����������
     */
    double convert_currency(double amount, const std::string& from,
        const std::string& to = "RUB") const;

    /**
     * @brief ��������� ����� ����� ����������
     * @param callback ������� ��������� ������
     */
    void update_currency_rates(std::function<void(bool success)> callback);
    /// @}

    /// @name ��������������� ������
    /// @{
    /**
     * @brief ���������� ������� �������� ����
     * @return ������ �� Account
     * @warning �� ���������� nullptr (����������� ����������)
     */
    Account& getCurrentAccount();

    /**
     * @brief ����������� ������� ����� �� ���������
     * @post ������� "�����" ���� ���� accounts ����
     */
    void ensureDefaultAccount();

    /**
     * @brief ��������� ���������� �� ����
     * @param type ��� ���������� (INCOME/EXPENSE)
     * @return ������ ����������
     */
    std::vector<Transaction> getFilteredTransactions(Transaction::Type type) const;

    /**
     * @brief ������� ������� ����������
     * @param transactions ������ ����������
     * @param title ��������� �������
     */
    void printTransactionsTable(const std::vector<Transaction>& transactions,
        const std::string& title) const;

    /**
     * @brief �������� ����� ������������ �� ����
     * @return �������� ����� (1-N)
     * @note ������������� �� ��������� ����������� �����
     */
    int getMenuChoice() const;

    /**
     * @brief ������� ������� (�����������������)
     */
    void clearConsole() const;

    /**
     * @brief ������� ����� �����
     */
    void clearInputBuffer() const;
    /// @}

    /// @name �������� ����������
    /// @{
    /**
     * @brief ������� ���� ����������
     */
    void runMainMenu();

    /**
     * @brief ���� ������ � ������������
     */
    void runTransactionMenu();

    /**
     * @brief ���� ����������
     */
    void runStatsMenu();

    /**
     * @brief ���� ������
     */
    void runSearchMenu();
    /// @}

    /**
     * @brief ��������� ����������� ������
     * @return true ���� ������� ���� ������ ���������
     * @note ������������ ��� �������� ������
     */
    bool validateData() const;

    /**
     * @brief ������� ���������� ��� ������ ���� �����������
     */
    void ensureDataDirectory();

    /**
     * @brief ���������� ������ ���� � ����� ������
     * @param filename ��� �����
     * @return ���������� ����
     */
    std::string getDataPath(const std::string& filename);

    /**
     * @brief ���� ������ ������
     */
    void showCurrencyMenu();
};