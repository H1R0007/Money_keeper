#include "FinanceCore.hpp"

void FinanceCore::showTotalBalance() const {
    double totalIncome = 0;
    double totalExpenses = 0;

    for (const auto& [name, account] : accounts) {
        for (const auto& t : account.get_transactions()) {
            if (t.get_type() == Transaction::Type::INCOME) {
                totalIncome += t.get_amount();
            }
            else {
                totalExpenses += t.get_amount();
            }
        }
    }

    std::cout << "\n=== Общая статистика ===\n"
        << "Доходы: " << totalIncome << " руб.\n"
        << "Расходы: " << totalExpenses << " руб.\n"
        << "Баланс: " << (totalIncome - totalExpenses) << " руб.\n";
}

void FinanceCore::showByCategory() const {
    std::map<std::string, std::pair<double, double>> categories; // категория -> <доходы, расходы>

    for (const auto& [name, account] : accounts) {
        for (const auto& t : account.get_transactions()) {
            if (t.get_type() == Transaction::Type::INCOME) {
                categories[t.get_category()].first += t.get_amount();
            }
            else {
                categories[t.get_category()].second += t.get_amount();
            }
        }
    }

    std::cout << "\n=== По категориям ===\n";
    for (const auto& [category, amounts] : categories) {
        std::cout << category << ": "
            << "+" << amounts.first << " / -" << amounts.second
            << " (итого: " << (amounts.first - amounts.second) << ")\n";
    }
}

void FinanceCore::showByMonth() const {
    std::map<std::pair<int, int>, std::pair<double, double>> monthly_stats;

    for (const auto& [name, account] : accounts) {
        for (const auto& t : account.get_transactions()) {
            const Date& date = t.get_date();
            auto month_key = std::make_pair(date.get_year(), date.get_month());
            double amount = t.get_amount();

            if (t.get_type() == Transaction::Type::INCOME) {
                monthly_stats[month_key].first += amount;
            }
            else {
                monthly_stats[month_key].second += amount;
            }
        }
    }

    clearConsole();
    std::cout << "\n=== Статистика по месяцам ===\n";
    std::cout << "+------------+------------+------------+------------+\n";
    std::cout << "|   Месяц    |   Доходы   |  Расходы   |  Баланс    |\n";
    std::cout << "+------------+------------+------------+------------+\n";

    for (const auto& [month, amounts] : monthly_stats) {
        std::string month_str = std::to_string(month.first) + "-" +
            (month.second < 10 ? "0" : "") + std::to_string(month.second);

        std::cout << "| " << std::setw(10) << month_str << " | "
            << std::setw(10) << std::fixed << std::setprecision(2) << amounts.first << " | "
            << std::setw(10) << amounts.second << " | "
            << std::setw(10) << (amounts.first - amounts.second) << " |\n";
    }
    std::cout << "+------------+------------+------------+------------+\n" << std::flush;
}

void FinanceCore::showCurrentAccountStats() const {
    double income = 0;
    double expenses = 0;

    for (const auto& t : currentAccount->get_transactions()) {
        if (t.get_type() == Transaction::Type::INCOME) {
            income += t.get_amount();
        }
        else {
            expenses += t.get_amount();
        }
    }

    std::cout << "\n=== Статистика (" << currentAccount->get_name() << ") ===\n"
        << "Транзакций: " << currentAccount->get_transactions().size() << "\n"
        << "Доходы: " << income << "\n"
        << "Расходы: " << expenses << "\n"
        << "Баланс: " << currentAccount->get_balance() << "\n";
}