/**
 * @file Statistics.cpp
 * @brief ���������� ���������� ��������� � ����������
 *
 * @details ���� ������ ������������� ����������� ����������� ��� �������:
 * - ������ ����������� ���������
 * - �������� �������/��������
 * - ������������������ ����������
 * - ���������� ����������
 * - �������� ���������
 *
 * @section analysis_types ���� ���������
 * 1. �������������� (����� �����)
 * 2. ������������������ (�� �������)
 * 3. ��������� (�� ��������)
 * 4. �������� (���������������� ��������)
 */


#include "FinanceCore.hpp"
#include <iomanip>

 /**
  * @brief ���������� ����� ������ �� ���� ������
  *
  * @details ���������:
  * - ��������� ������ (��� INCOME ����������)
  * - ��������� ������� (��� EXPENSE ����������)
  * - ������ ������ (������ - �������)
  *
  * @note ��� �������� �������������� � ������� ������ (base_currency_)
  *
  * @par ������ ������:
  * @code
  * === ����� ���������� (USD) ===
  * ������: 1500.00
  * �������: 750.50
  * ������: 749.50
  * @endcode
  */

void FinanceCore::showTotalBalance() const {
    double totalIncome = 0;
    double totalExpenses = 0;

    for (const auto& [name, account] : accounts) {
        for (const auto& t : account.get_transactions()) {
            double amount = currency_converter_.convert(
                t.get_amount(),
                t.get_currency(),
                base_currency_
            );

            if (t.get_type() == Transaction::Type::INCOME) {
                totalIncome += amount;
            }
            else {
                totalExpenses += amount;
            }
        }
    }

    std::cout << "\n=== ����� ���������� (" << base_currency_ << ") ===\n"
        << "������: " << std::fixed << std::setprecision(2) << totalIncome << "\n"
        << "�������: " << totalExpenses << "\n";
}

/**
 * @brief ���������� ���������� �� ����������
 *
 * @details ��������� ���������������� �����:
 * - ���������� ���������� �� ����������
 * - ������� �������� ������ � ������� ��� ������
 * - ��������� �������� ������ �� ����������
 *
 * @note ���������� ������������� ����������:
 * 1. �� �������� ���������
 * 2. �� ����������� �������� �����
 *
 * @warning ��������� � ������ ��������� ������������ � "��� ���������"
 */
void FinanceCore::showByCategory() const {
    std::map<std::string, std::pair<double, double>> categories;

    for (const auto& [name, account] : accounts) {
        for (const auto& t : account.get_transactions()) {
            double amount = t.get_amount_in_rub(currency_converter_);
            if (t.get_type() == Transaction::Type::INCOME) {
                categories[t.get_category()].first += amount;
            }
            else {
                categories[t.get_category()].second += amount;
            }
        }
    }

    std::cout << "\n=== ���������� �� ���������� (" << base_currency_ << ") ===\n";
    std::cout << "+----------------------+----------------+----------------+----------------+\n";
    std::cout << "|      ���������       |     ������     |    �������     |     �����      |\n";
    std::cout << "+----------------------+----------------+----------------+----------------+\n";

    for (const auto& [category, amounts] : categories) {
        std::string cat_display = category.empty() ? "��� ���������" : category;
        if (cat_display.length() > 20) cat_display = cat_display.substr(0, 17) + "...";

        std::cout << "| " << std::left << std::setw(20) << cat_display << " | "
            << std::right << std::setw(14) << std::fixed << std::setprecision(2) << amounts.first << " | "
            << std::setw(14) << amounts.second << " | "
            << std::setw(14) << (amounts.first - amounts.second) << " |\n";
    }
    std::cout << "+----------------------+----------------+----------------+----------------+\n" << std::flush;
}

/**
 * @brief ����������� ���������� �� �������
 *
 * @details ������ ��������� �����:
 * - ����������� �� ���� � ������
 * - ������������ �������/��������
 * - ������ ��������� �������
 * - ������������ ��������
 *
 * @note ��� ���������� ������ ���������:
 * - ������� 3 ������ ������
 * - ��������� CultureInfo ��� �����������
 *
 * @par ������ ������:
 * @code
 * | 2023-05 |   1500.00 |     750.50 |      749.50 |
 * | 2023-06 |   2000.00 |    1000.00 |     1000.00 |
 * @endcode
 */
void FinanceCore::showByMonth() const {
  //  std::lock_guard<std::mutex> lock(accounts_mutex_);
    std::map<std::pair<int, int>, std::pair<double, double>> monthly_stats;

    for (const auto& [name, account] : accounts) {
        for (const auto& t : account.get_transactions()) {
            const Date& date = t.get_date();
            auto month_key = std::make_pair(date.get_year(), date.get_month());
            double amount = t.get_amount_in_rub(currency_converter_);

            if (t.get_type() == Transaction::Type::INCOME) {
                monthly_stats[month_key].first += amount;
            }
            else {
                monthly_stats[month_key].second += amount;
            }
        }
    }

    clearConsole();
    std::cout << "\n=== ���������� �� ������� (� RUB) ===\n";
    std::cout << "+------------+--------------+--------------+--------------+\n";
    std::cout << "|   �����    |    ������    |   �������    |    ������    |\n";
    std::cout << "+------------+--------------+--------------+--------------+\n";

    for (const auto& [month, amounts] : monthly_stats) {
        std::string month_str = std::to_string(month.first) + "-" +
            (month.second < 10 ? "0" : "") + std::to_string(month.second);

        std::cout << "| " << std::setw(10) << month_str << " | "
            << std::setw(12) << std::fixed << std::setprecision(2) << amounts.first << " | "
            << std::setw(12) << amounts.second << " | "
            << std::setw(12) << (amounts.first - amounts.second) << " |\n";
    }
    std::cout << "+------------+--------------+--------------+--------------+\n" << std::flush;
}



/**
 * @brief ���������� ��������� ���������� �� �������� �����
 *
 * @details ��������:
 * - ����� ���������� ����������
 * - �������� �� �������
 * - ����������� �������/��������
 * - ������� �������� ��������
 *
 * @warning ���������� ������ �������� ���� (currentAccount)
 *
 * @post ����� ������ ��������� ��������� ����������� �������� � ����
 */
void FinanceCore::showCurrentAccountStats() const {
   // std::lock_guard<std::mutex> lock(accounts_mutex_);

    if (!currentAccount) {
        std::cout << "��� �������� �����!\n";
        return;
    }

    double income = 0;
    double expenses = 0;
    std::map<std::string, double> byCurrency;

    for (const auto& t : currentAccount->get_transactions()) {
        double amount = t.get_amount_in_rub(currency_converter_);
        byCurrency[t.get_currency()] += t.get_signed_amount();

        if (t.get_type() == Transaction::Type::INCOME) {
            income += amount;
        }
        else {
            expenses += amount;
        }
    }

    std::cout << "\n=== ���������� (" << currentAccount->get_name() << ") ===\n"
        << "����������: " << currentAccount->get_transactions().size() << "\n"
        << "������: " << std::fixed << std::setprecision(2) << income << " ���.\n"
        << "�������: " << expenses << " ���.\n"
        << "\n�� �������:\n";

    for (const auto& [currency, amount] : byCurrency) {
        std::cout << "  " << currency << ": " << amount;
        if (currency != "RUB") {
            std::cout << " (?" << convert_currency(amount, currency, "RUB") << " ���.)";
        }
        std::cout << "\n";
    }
}

/**
 * @brief ���������� ������ � ������� �����
 *
 * @details ����������:
 * - ����������� ����� �� ������ ������
 * - ��������������� � ������� ������
 * - ���������� ����������� �����
 *
 * @note ��� ����������� ���������� ������� ����� CurrencyConverter
 */
void FinanceCore::showBalanceByCurrency() const {
    std::map<std::string, double> balances;

    // �������� ������� �� ���� �������
    for (const auto& [name, account] : accounts) {
        for (const auto& t : account.get_transactions()) {
            balances[t.get_currency()] += t.get_signed_amount();
        }
    }

    std::cout << "\n=== ������ �� ������� ===\n";
    for (const auto& [currency, amount] : balances) {
        std::cout << currency << ": " << std::fixed << std::setprecision(2) << amount << "\n";
    }
    std::cout << "\n������� Enter ����� ����������...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

/**
 * @brief ����� ���������� �� �����
 *
 * @param tags ������ ����� ��� ������
 *
 * @details �������� ������:
 * 1. ���� ���������� �� ���� ����� ������������ (AND-������)
 * 2. ������� ���������� � ��������� �������
 * 3. ������������ ��������� ����
 *
 * @note ������� ����� �� �����������
 * @warning ��� ������ ������ ����� ������� ��������������
 */
void FinanceCore::searchByTags(const std::vector<std::string>& tags) const {
    if (tags.empty()) {
        std::cout << "\n������: �� ������� ���� ��� ������\n";
        std::cout << "������� Enter ��� �����������...";
        std::cin.ignore();
        std::cin.get();
        return;
    }

    std::vector<Transaction> result;

    for (const auto& [name, account] : accounts) {
        for (const auto& t : account.get_transactions()) {
            const auto& transaction_tags = t.get_tags();
            if (std::any_of(tags.begin(), tags.end(),
                [&](const auto& tag) {
                    return std::find(transaction_tags.begin(),
                        transaction_tags.end(), tag) != transaction_tags.end();
                })) {
                result.push_back(t);
            }
        }
    }

    clearConsole();
    printTransactionsTable(result, "���������� ������ �� �����");

    // ��������� ����� ����� ��������� � ����
    std::cout << "\n������� Enter ��� �������� � ����...";
    std::cin.ignore();
    std::cin.get();
}
