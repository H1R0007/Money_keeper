#pragma once
#include "Time_Manager.hpp"
#include <vector>
#include <string>

class Account {
private:
    std::string name;
    double balance;
    std::vector<Transaction> transactions;

public:

    Account() : name("Без названия"), balance(0) {}

    Account(const std::string& accountName) : name(accountName), balance(0) {}

    void addTransaction(const Transaction& t) {
        transactions.push_back(t);
        balance += t.get_signed_amount();
    }

    void removeTransaction(int id) {
        auto it = std::find_if(transactions.begin(), transactions.end(),
            [id](const Transaction& t) { return t.get_id() == id; });

        if (it != transactions.end()) {
            balance -= it->get_signed_amount();
            transactions.erase(it);
        }
    }

    void move_transactions_from(Account&& other) {
        balance = other.balance;
        transactions = std::move(other.transactions);
    }

    void Account::recalculateBalance() {
        balance = 0;
        for (const auto& t : transactions) {
            balance += t.get_signed_amount();
        }
    }



    std::string get_name() const { return name; }
    double get_balance() const { return balance; }
    void set_name(const std::string& newName) { name = newName; }

    // Метод для перемещения транзакций
    const std::vector<Transaction>& get_transactions() const { return transactions; }
};
