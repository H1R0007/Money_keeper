#include "Account.hpp"

void Account::addTransaction(const Transaction& t) {
    transactions.push_back(t);
    balance += t.get_signed_amount();
}

void Account::removeTransaction(int id) {
    auto it = std::find_if(transactions.begin(), transactions.end(),
        [id](const Transaction& t) { return t.get_id() == id; });

    if (it != transactions.end()) {
        balance -= it->get_signed_amount();
        transactions.erase(it);
    }
}

void Account::move_transactions_from(Account&& other) {
    balance = other.balance;
    transactions = std::move(other.transactions);
}

void Account::recalculateBalance() {
    balance = 0;
    for (const auto& t : transactions) {
        balance += t.get_signed_amount();
    }
}