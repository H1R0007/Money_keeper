#pragma once
#include "Date.hpp"
#include "Time_Manager.hpp"
#include <vector>
#include <map>
#include <algorithm>
#include "Account.hpp"

class FinanceCore {
private:
    std::vector<Transaction> transactions;
    std::string dataFile = "C:/Users/Zeta/source/repos/Money_keeper/data/transactions.dat";
    std::map<std::string, Account> accounts;
    Account* currentAccount;

    // Вспомогательные методы
    void printMainMenu() const;
    void printTransactionMenu() const;
    void printStatsMenu() const;

public:
    FinanceCore();

    // Основные функции
    void renameAccount();
    void deleteAccount();
    void selectAccount();
    void createAccount();
    void manageAccounts();
    void removeTransaction();
    void loadData();
    void saveData();
    void addTransaction();
    void viewAllTransactions() const;
    void viewIncome() const;
    void viewExpenses() const;
    void showStatistics() const;
    void showByCategory() const;
    void showTotalBalance() const;
    void showByMonth() const;
    void showTopExpenses() const;

    // Интерфейс
    void runMainMenu();
    void runTransactionMenu();
    void runStatsMenu();
};