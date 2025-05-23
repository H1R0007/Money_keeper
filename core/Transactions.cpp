/**
 * @file Transactions.cpp
 * @brief ���������� �������� � ����������� ������������
 *
 * @details ���� ������ �������� ���������� ������� ���:
 * - ���������� ����� ���������� ����� ������������� ���������
 * - �������� ������������ ����������
 * - ����������� ���������� � ��������� ��������������
 * - ������ � ������ ����������
 *
 * @section transaction_flow_sec ������� �������
 * 1. �������� ���������� ����� ������������ ������
 * 2. ��������� ���� �������� ������
 * 3. ����������� ����� ��� �������������
 * 4. ���������� �����
 * 5. ���������� � ������� �������
 *
 * @warning ��� �������� �������� �������� � ������������� �������,
 *          ���� ������������ ����� �������� (�����/������)
 */

#include "FinanceCore.hpp"
#include <Windows.h>
#include <iomanip>

 /**
 * @brief ��������� ����� ���������� ����� ������������� ������
 *
 * @details ��������� ��������� ������� �������� ����������:
 * 1. ����� ���� �������� (�����/������)
 * 2. ���� ����� � ������
 * 3. �������� ���������
 * 4. ��������� ���� (�� ��������� �������)
 * 5. ���������� ��������
 * 6. ���������� ������
 *
 * @throws std::invalid_argument ��� ���������� ������� ������
 * @post ��� �������� ���������� ���������� ����������� � currentAccount
 *
 * @note �����������:
 * - ��������� ������ �� ����� ����� (���� 0)
 * - �������������� ��������� ���� �����
 * - ������������ ����� � ������ ��� ��������� �����
 * - ����������� �� ���������� ����� (MAX_TAGS)
 *
 * @par ������ �������:
 * @code{.unparsed}
 * === ����� ���������� ===
 * 1. ����� | 2. ������ | 0. ������
 * > 1
 * ������� �����: 100
 * ������� ������: USD
 * ���������: ��������
 * ���� [YYYY-MM-DD]: 2023-05-20
 * ��������: ����� �� ���
 * @endcode
 */

void FinanceCore::addTransaction() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    Transaction newTrans;
    int step = 1;
    bool cancelled = false;

    while (step <= 7 && !cancelled) { // ��������� ���������� ����� �� 6
        try {
            switch (step) {

            case 1: { // ��� ��������
                std::cout << "\n=== ����� ���������� ===\n";
                std::cout << "1. �����\n2. ������\n0. ������\n�������� ���: ";
                int type = getMenuChoice();

                if (type == 0) {
                    cancelled = true;
                    std::cout << "��������.\n";
                    break;
                }
                newTrans.set_type(type == 1 ? Transaction::Type::INCOME : Transaction::Type::EXPENSE);
                step++;
                break;
            }

            case 2: { // ��� 2: �����
                std::cout << "������� ����� (0 ��� ������): ";
                double amount;
                std::cin >> amount;

                if (amount == 0) {
                    cancelled = true;
                    std::cout << "��������.\n";
                    break;
                }

                // �������� ��������� ������
                std::cout << "������� ������ (RUB, USD, EUR): ";
                std::string currency;
                std::cin >> currency;

                if (!currency_converter_.is_currency_supported(currency)) {
                    std::cout << "������ �� ��������������. ������������ RUB\n";
                    currency = "RUB";
                }

                newTrans.set_amount(amount);
                newTrans.set_currency(currency);
                step++;
                break;
            }

            case 3: { // ���������
                std::cout << "������� ��������� (0 ��� ������): ";
                std::string category;
                clearInputBuffer();
                std::getline(std::cin, category);

                if (category == "0") {
                    cancelled = true;
                    std::cout << "��������.\n";
                    break;
                }
                newTrans.set_category(category);
                step++;
                break;
            }

            case 4: { // ����
                std::cout << "���� (����-��-��, enter ��� �������, 0 ��� ������): ";
                std::string date_str;
                std::cin.ignore(0);
                std::getline(std::cin, date_str);

                if (date_str == "0") {
                    cancelled = true;
                    std::cout << "��������.\n";
                    break;
                }

                if (date_str.empty()) {
                    step++; // ���������� ������� ���� �� ���������
                    break;
                }

                try {
                    auto date = Date::from_string(date_str);
                    newTrans.set_date(date);
                    step++;
                }
                catch (...) {
                    throw std::invalid_argument("�������� ������ ����. ����������� ����-��-��");
                }
                break;
            }

            case 5: { // ��������
                std::cout << "������� �������� (enter ����� ����������, 0 ��� ������): ";
                std::string desc;
                clearInputBuffer();
                std::getline(std::cin, desc);

                if (desc == "0") {
                    cancelled = true;
                    std::cout << "��������.\n";
                    break;
                }
                newTrans.set_description(desc);
                step++;
                break;
            }
            case 6: { // ��� 6: ����
                const auto& available_tags = Transaction::get_available_tags();
                bool tag_adding_finished = false;

                while (!tag_adding_finished) {
                    clearConsole();
                    std::cout << "\n=== ���������� ������ ("
                        << newTrans.get_tags().size() << "/"
                        << Transaction::MAX_TAGS << ") ===\n";

                    // ����� ������� �����
                    if (!newTrans.get_tags().empty()) {
                        std::cout << "������� ����: ";
                        for (size_t i = 0; i < newTrans.get_tags().size(); ++i) {
                            std::cout << (i > 0 ? ", " : "") << "[" << newTrans.get_tags()[i] << "]";
                        }
                        std::cout << "\n\n";
                    }

                    // ����� ��������� �����
                    std::cout << "��������� ����:\n";
                    for (size_t i = 0; i < available_tags.size(); ++i) {
                        std::cout << i + 1 << ". " << available_tags[i] << "\n";
                    }

                    std::cout << "\n0. ��������� ���������� �����\n";
                    if (!newTrans.get_tags().empty()) {
                        std::cout << "99. ������� ���\n";
                    }
                    std::cout << "�������� ��������: ";

                    int choice = getMenuChoice();

                    if (choice == 0) {
                        tag_adding_finished = true;
                        step++;
                    }
                    else if (choice == 99 && !newTrans.get_tags().empty()) {
                        // �������� ����
                        std::cout << "�������� ��� ��� ��������:\n";
                        for (size_t i = 0; i < newTrans.get_tags().size(); ++i) {
                            std::cout << i + 1 << ". " << newTrans.get_tags()[i] << "\n";
                        }
                        std::cout << "0. ������\n> ";

                        int tag_choice = getMenuChoice();
                        if (tag_choice > 0 && tag_choice <= newTrans.get_tags().size()) {
                            newTrans.remove_tag(tag_choice - 1); // ���������� ����� �����
                        }
                    }
                    else if (choice > 0 && choice <= available_tags.size()) {
                        // ���������� ����
                        try {
                            const std::string& selected_tag = available_tags[choice - 1];
                            if (std::find(newTrans.get_tags().begin(), newTrans.get_tags().end(), selected_tag)
                                != newTrans.get_tags().end()) {
                                std::cout << "���� ��� ��� ��������!\n";
                            }
                            else if (newTrans.get_tags().size() >= Transaction::MAX_TAGS) {
                                std::cout << "��������� ����� ����� (" << Transaction::MAX_TAGS << ")\n";
                            }
                            else {
                                newTrans.add_tag(selected_tag); // ���������� ����� �����
                            }
                        }
                        catch (const std::exception& e) {
                            std::cerr << "������: " << e.what() << "\n";
                        }
                        std::cout << "������� Enter ��� �����������...";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    }
                }
                break;
            }

            case 7: { // �����������
                currentAccount->addTransaction(newTrans);

                double rubAmount = currency_converter_.convert(
                    newTrans.get_amount(),
                    newTrans.get_currency(),
                    "RUB"
                );

                std::cout << "\n���������� ���������!\n"
                    << "�����: " << newTrans.get_amount() << " " << newTrans.get_currency()
                    << " (?" << std::fixed << std::setprecision(2) << rubAmount << " RUB)\n";

                if (!newTrans.get_tags().empty()) {
                    std::cout << "����: ";
                    for (const auto& tag : newTrans.get_tags()) {
                        std::cout << "[" << tag << "] ";
                    }
                    std::cout << "\n";
                }

                step++;
                break;
            }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "������: " << e.what() << "\n";
            std::cin.clear();
            clearInputBuffer();
            std::cout << "������� Enter ��� �����������...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
        }
    }
}

/**
 * @brief ������� ���������� �� ID
 *
 * @details ������� ��������:
 * 1. ����������� ������ ���� ����������
 * 2. ���� ID ���������� ��� ��������
 * 3. ����� � �������� �� currentAccount
 *
 * @pre � currentAccount ������ ������������ ����������
 * @throws std::out_of_range ���� ���������� � ��������� ID �� �������
 *
 * @note ������������ ������ �������� (���� 0)
 * @warning ��������� ���������� ���������� ������������
 */
void FinanceCore::removeTransaction() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    //std::lock_guard<std::mutex> lock(accounts_mutex_);

    if (currentAccount->get_transactions().empty()) {
        std::cout << "��� ���������� ��� ��������.\n";
        return;
    }

    viewAllTransactions();
    std::cout << "������� ID ���������� ��� �������� (0 ��� ������): ";
    int id = getMenuChoice();

    if (id == 0) return;

    if (currentAccount->removeTransaction(id)) {
        std::cout << "���������� �������.\n";
    }
    else {
        std::cout << "���������� � ID " << id << " �� �������.\n";
    }
}


/**
 * @brief ������� ��������� ������������� ����������
 *
 * @param transactions ������ ���������� ��� �����������
 * @param title ��������� �������
 *
 * @details ������ �������:
 * +------+------------+----------+------------+------------+--------------+
 * |  ID  |    ����    |   ���    |   �����    |  ������    |  ���������   |
 * +------+------------+----------+------------+------------+--------------+
 *
 * @note �����������:
 * - �������������� ������������ ��������
 * - ������� ������� ��������� �����
 * - �������� ��������� ����� �������� (����������� ����� WinAPI)
 * - ��������� UTF-8 ��������
 */

void FinanceCore::printTransactionsTable(const std::vector<Transaction>& transactions, const std::string& title) const {
    if (transactions.empty()) {
        std::cout << "\n��� ���������� (" << title << ") ��� �����������.\n";
        return;
    }

    std::cout << "\n=== " << title << " (" << transactions.size() << ") ===\n";
    std::cout << "+------+------------+----------+------------+------------+--------------+\n";
    std::cout << "|  ID  |    ����    |   ���    |   �����    |  ������    |  ���������   |\n";
    std::cout << "+------+------------+----------+------------+------------+--------------+\n";

    for (const auto& t : transactions) {
        std::cout << "| " << std::setw(4) << t.get_id() << " | "
            << t.get_date().to_string() << " | "
            << std::setw(8) << (t.get_type() == Transaction::Type::INCOME ? "�����" : "������") << " | "
            << std::setw(10) << std::fixed << std::setprecision(2) << t.get_amount() << " | "
            << std::setw(10) << t.get_currency() << " | "
            << std::setw(12) << (t.get_category().empty() ? "-" : t.get_category().substr(0, 12)) << " |\n";
    }
    std::cout << "+------+------------+----------+------------+------------+--------------+\n" << std::flush;
}

/**
 * @brief ���������� ������ �������� ����������
 *
 * @details ��������� ���������� currentAccount �� ���� INCOME
 * � ������� �� � ��������� ������� ����� printTransactionsTable()
 *
 * @see printTransactionsTable()
 * @see getFilteredTransactions()
 */
void FinanceCore::viewIncome() const {
    auto incomes = getFilteredTransactions(Transaction::Type::INCOME);
    printTransactionsTable(incomes, "������");
}

/**
 * @brief ���������� ������ ��������� ����������
 *
 * @details ��������� ���������� currentAccount �� ���� EXPENSE
 * � ������� �� � ��������� ������� ����� printTransactionsTable()
 *
 * @see printTransactionsTable()
 * @see getFilteredTransactions()
 */
void FinanceCore::viewExpenses() const {
    auto expenses = getFilteredTransactions(Transaction::Type::EXPENSE);
    printTransactionsTable(expenses, "�������");
}

/**
 * @brief ��������� ���������� �� ����
 *
 * @param type ��� ���������� (INCOME/EXPENSE)
 * @return ������ ���������� ���������� ����
 *
 * @note ���������� ����� ���������� (�� ������)
 * @complexity O(n), ��� n - ���������� ����������
 */

std::vector<Transaction> FinanceCore::getFilteredTransactions(Transaction::Type type) const {
    std::vector<Transaction> result;
    for (const auto& t : currentAccount->get_transactions()) {
        if (t.get_type() == type) {
            result.push_back(t);
        }
    }
    return result;
}

/**
 * @brief ���������� ��� ���������� �������� �����
 *
 * @details ������ ������:
 * +------+------------+----------+------------+------------+--------------+--------------+
 * |  ID  |    ����    |   ���    |   �����    |  ������    |  ���������   |  ��������    |
 * +------+------------+----------+------------+------------+--------------+--------------+
 *
 * @post ������� ������� ����� �������
 * @note ���������� ���������� ������� ��������� ����� (��������� "...")
 */
void FinanceCore::viewAllTransactions() const {
    clearConsole();

    std::cout << "\n=== ��� ���������� ===\n";
    std::cout << "+------+------------+----------+------------+------------+--------------+--------------+\n";
    std::cout << "|  ID  |    ����    |   ���    |   �����    |  ������    |  ���������   |  ��������    |\n";
    std::cout << "+------+------------+----------+------------+------------+--------------+--------------+\n";

    for (const auto& t : currentAccount->get_transactions()) {
        std::cout << "| " << std::setw(4) << t.get_id() << " | "
            << t.get_date().to_string() << " | "
            << std::setw(8) << (t.get_type() == Transaction::Type::INCOME ? "�����" : "������") << " | "
            << std::setw(10) << std::fixed << std::setprecision(2) << t.get_amount() << " | "
            << std::setw(10) << t.get_currency() << " | "
            << std::setw(12) << (t.get_category().empty() ? "-" : t.get_category()) << " | "
            << std::setw(12) << (t.get_description().empty() ? "-" : t.get_description()) << " |\n";
    }
    std::cout << "+------+------------+----------+------------+------------+--------------+--------------+\n" << std::flush;
}

/**
 * @brief ���������� ������ ��������� �����
 *
 * @return ����������� ������ �� ������ ���������������� �����
 *
 * @details ����������� ���� ��������:
 * - �������� ��������� �������� (���, ���������)
 * - ��������� ������� (��������, ����������)
 * - ����������� ����� (�����, ��������� �����)
 *
 * @note ���� ������������ �� ������� ����
 */
const std::vector<std::string>& Transaction::get_available_tags() {
    static const std::vector<std::string> TAGS = {
        "��������", "�����������", "��������",
        "���������", "�����", "�����", "������",
        "�����������", "����", "�������", "�����",
        "��������", "������", "����", "��������",
        "�����������", "�����", "�����",
        "����������", "�������������", "��������",
        "������", "������", "������",
        "�������", "�����������", "������"
    };
    return TAGS;
}