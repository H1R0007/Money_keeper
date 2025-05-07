#pragma once
#include "FinanceCore.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <numeric>

void FinanceCore::saveData() {
    std::ofstream file(dataFile);
    for (const auto& [name, account] : accounts) {
        file << "[Account:" << name << "]\n";
        for (const auto& t : account.get_transactions()) {
            file << t << "\n";
        }
    }
}

void FinanceCore::loadData() {
    std::ifstream file(dataFile);
    std::string line;
    Account* current = nullptr;

    while (std::getline(file, line)) {
        if (line.find("[Account:") != std::string::npos) {
            std::string name = line.substr(9, line.size() - 10);
            accounts[name] = Account(name);
            current = &accounts[name];
        }
        else if (current) {
            std::istringstream iss(line);
            Transaction t;
            if (iss >> t) {
                current->addTransaction(t);
            }
        }
    }
}

void FinanceCore::viewIncome() const {
    std::cout << "\n=== ������ ===\n";
    for (const auto& t : transactions) {
        if (t.get_type() == Transaction::Type::INCOME) {
            std::cout << t.get_summary() << "\n";
        }
    }
}

void FinanceCore::viewExpenses() const {
    std::cout << "\n=== ������� ===\n";
    for (const auto& t : transactions) {
        if (t.get_type() == Transaction::Type::EXPENSE) {
            std::cout << t.get_summary() << "\n";
        }
    }
}

FinanceCore::FinanceCore() {
    accounts["�����"] = Account("�����");
    currentAccount = &accounts["�����"];
    loadData();
}

void FinanceCore::printMainMenu() const {
    std::cout << "\n=== ������� ���� ==="
        << "\n������� ����: " << currentAccount->get_name()
        << " (������: " << currentAccount->get_balance() << ")"
        << "\n1. �������� ����������"
        << "\n2. ������� ��������"
        << "\n3. ����������"
        << "\n4. ���������� �������"
        << "\n5. ������� ����������"
        << "\n6. �����"
        << "\n�������� ��������: ";
}

void FinanceCore::showStatistics() const {
    int choice;
    do {
        std::cout << "\n=== ���������� ==="
            << "\n1. ����� ������"
            << "\n2. �� ����������"
            << "\n3. �� �������"
            << "\n4. �����"
            << "\n�������� ��������: ";

        std::cin >> choice;

        switch (choice) {
        case 1: showTotalBalance(); break;
        case 2: showByCategory(); break;
        case 3: showByMonth(); break;
        case 4: return;
        default: std::cout << "�������� �����!\n";
        }
    } while (true);
}

void FinanceCore::showTotalBalance() const {
    double balance = std::accumulate(transactions.begin(), transactions.end(), 0.0,
        [](double sum, const Transaction& t) {
            return sum + t.get_signed_amount();
        });

    std::cout << "\n=== ����� ������ ===\n";
    std::cout << "������� ������: " << balance << " ���.\n";
}

void FinanceCore::showByCategory() const {
    std::map<std::string, double> categories;

    for (const auto& t : transactions) {
        categories[t.get_category()] += t.get_signed_amount();
    }

    std::cout << "\n=== �� ���������� ===\n";
    for (const auto& [category, amount] : categories) {
        std::cout << category << ": " << amount << " ���.\n";
    }
}

void FinanceCore::showByMonth() const {
    std::map<std::pair<int, int>, double> months; // <year, month>

    for (const auto& t : transactions) {
        auto key = std::make_pair(t.get_date().get_year(),
            t.get_date().get_month());
        months[key] += t.get_signed_amount();
    }

    std::cout << "\n=== �� ������� ===\n";
    for (const auto& [month, amount] : months) {
        std::cout << month.first << "-" << month.second << ": "
            << amount << " ���.\n";
    }
}


void FinanceCore::runMainMenu() {
    int choice;
    do {
        printMainMenu();
        std::cin >> choice;
        std::cin.ignore(); // ������� ������

        switch (choice) {
        case 1: addTransaction(); break;
        case 2: runTransactionMenu(); break;
        case 3: runStatsMenu(); break;
        case 4: saveData(); return;
        default: std::cout << "�������� �����!\n";
        }
    } while (true);
}

void FinanceCore::addTransaction() {
    Transaction newTrans;
    int step = 1;
    bool cancelled = false;

    while (step <= 5 && !cancelled) {
        try {
            switch (step) {
            case 1: { // ��� 1: ��� ��������
                std::cout << "\n=== ����� ���������� ===\n";
                std::cout << "1. �����\n2. ������\n0. ������\n�������� ���: ";
                int type;
                std::cin >> type;

                if (type == 0) {
                    cancelled = true;
                    std::cout << "��������.\n";
                    break;
                }

                if (type != 1 && type != 2) {
                    throw std::invalid_argument("�������� ��� ��������");
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

                newTrans.set_amount(amount);
                step++;
                break;
            }

            case 3: { // ��� 3: ���������
                std::cout << "������� ��������� (0 ��� ������): ";
                std::string category;
                std::cin.ignore();
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

            case 4: { // ��� 4: ����
                std::cout << "���� (���� �� ��, 0 0 0 ��� �������, -1 ��� ������): ";
                int y, m, d;
                std::cin >> y >> m >> d;

                if (y == -1) {
                    cancelled = true;
                    std::cout << "��������.\n";
                    break;
                }

                if (y == 0 && m == 0 && d == 0) {
                    // ���������� ������� ���� (�� ���������)
                    step++;
                    break;
                }

                Date customDate(y, m, d);
                newTrans.set_date(customDate);
                step++;
                break;
            }

            case 5: { // ��� 5: ��������
                std::cout << "������� �������� (�����������, 0 ��� ������): ";
                std::string desc;
                std::cin.ignore();
                std::getline(std::cin, desc);

                if (desc == "0") {
                    cancelled = true;
                    std::cout << "��������.\n";
                    break;
                }

                newTrans.set_description(desc);

                // �����������
                currentAccount->addTransaction(newTrans);
                std::cout << "���������� ������� ���������!\n";
                step++;
                break;
            }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "������: " << e.what() << "\n���������� �����.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

void FinanceCore::removeTransaction() {
    if (currentAccount->get_transactions().empty()) {
        std::cout << "��� ���������� ��� ��������.\n";
        return;
    }

    viewAllTransactions();
    std::cout << "������� ID ���������� ��� �������� (0 ��� ������): ";
    int id;
    std::cin >> id;

    if (id == 0) return;

    currentAccount->removeTransaction(id);
    std::cout << "���������� �������.\n";
}

void FinanceCore::runTransactionMenu() {
    int choice;
    do {
        std::cout << "\n=== ������� ���������� ==="
            << "\n1. ��� ����������"
            << "\n2. ������ ������"
            << "\n3. ������ �������"
            << "\n4. �����"
            << "\n�������� ��������: ";

        std::cin >> choice;

        switch (choice) {
        case 1: viewAllTransactions(); break;
        case 2: viewIncome(); break;
        case 3: viewExpenses(); break;
        case 4: return;
        default: std::cout << "�������� �����!\n";
        }
    } while (true);
}

void FinanceCore::viewAllTransactions() const {
    std::cout << "\n=== ��� ���������� ===\n";
    for (const auto& t : transactions) {
        std::cout << t.get_summary() << "\n";
    }
}

void FinanceCore::printTransactionMenu() const {
    std::cout << "\n=== ���� ���������� ==="
        << "\n1. �������� ��� ����������"
        << "\n2. �������� ������"
        << "\n3. �������� �������"
        << "\n4. ����������� �� ���� (����� �������)"
        << "\n5. ����������� �� �����"
        << "\n6. �����"
        << "\n�������� ��������: ";
}

void FinanceCore::printStatsMenu() const {
    std::cout << "\n=== ���� ���������� ==="
        << "\n1. ����� ������"
        << "\n2. ���������� �� ����������"
        << "\n3. ���������� �� �������"
        << "\n4. ���-5 ����� ������� ��������"
        << "\n5. �����"
        << "\n�������� ��������: ";
}

void FinanceCore::showTopExpenses() const {
    std::vector<Transaction> expenses;

    // �������� ��� �������
    std::copy_if(transactions.begin(), transactions.end(),
        std::back_inserter(expenses),
        [](const Transaction& t) {
            return t.get_type() == Transaction::Type::EXPENSE;
        });

    // ��������� �� �������� �����
    std::sort(expenses.begin(), expenses.end(),
        [](const Transaction& a, const Transaction& b) {
            return a.get_amount() > b.get_amount();
        });

    // ������� ���-5 ��� ������, ���� �������� ������ 5
    std::cout << "\n=== ���-5 ����� ������� �������� ===\n";
    int count = std::min(5, static_cast<int>(expenses.size()));
    for (int i = 0; i < count; ++i) {
        std::cout << i + 1 << ". " << expenses[i].get_summary() << "\n";
    }

    if (expenses.empty()) {
        std::cout << "��� ������ � ��������.\n";
    }
}

void FinanceCore::runStatsMenu() {
    int choice;
    do {
        printStatsMenu();
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
        case 1:
            showTotalBalance();
            break;
        case 2:
            showByCategory();
            break;
        case 3:
            showByMonth();
            break;
        case 4:
            showTopExpenses();
            break;
        case 5:
            return;
        default:
            std::cout << "�������� �����! ���������� �����.\n";
        }

        if (choice >= 1 && choice <= 4) {
            std::cout << "\n������� Enter ��� �����������...";
            std::cin.get();
        }
    } while (true);
}

void FinanceCore::manageAccounts() {
    int choice;
    do {
        std::cout << "\n=== ���������� ������� ==="
            << "\n1. ������� ����"
            << "\n2. ������� ����"
            << "\n3. ������� ����"
            << "\n4. �����"
            << "\n�������� ��������: ";

        std::cin >> choice;

        switch (choice) {
        case 1: createAccount(); break;
        case 2: deleteAccount(); break;
        case 3: selectAccount(); break;
        case 4: return;
        default: std::cout << "�������� �����!\n";
        }
    } while (true);
}

void FinanceCore::createAccount() {
    std::string name;
    std::cout << "������� �������� �����: ";
    std::cin.ignore();
    std::getline(std::cin, name);

    if (accounts.find(name) != accounts.end()) {
        std::cout << "���� � ����� ������ ��� ����������!\n";
        return;
    }

    accounts[name] = Account(name);
    std::cout << "���� ������!\n";
}

void FinanceCore::selectAccount() {
    if (accounts.empty()) {
        std::cout << "��� ��������� ������.\n";
        return;
    }

    std::cout << "\n=== ����� ����� ===\n";
    int index = 1;
    for (const auto& [name, account] : accounts) {
        std::cout << index++ << ". " << name
            << " (������: " << account.get_balance() << ")\n";
    }
    std::cout << "0. ������\n";

    std::cout << "�������� ����: ";
    int choice;
    std::cin >> choice;

    if (choice == 0) {
        std::cout << "��������.\n";
        return;
    }

    if (choice < 1 || choice > accounts.size()) {
        std::cout << "�������� �����!\n";
        return;
    }

    auto it = accounts.begin();
    std::advance(it, choice - 1);
    currentAccount = &(it->second);
    std::cout << "������ ����: " << it->first << "\n";
}

void FinanceCore::deleteAccount() {
    if (accounts.size() <= 1) { // ������ ������� ��������� ����
        std::cout << "������ �������� ���� �� ���� ����!\n";
        return;
    }

    std::cout << "\n=== �������� ����� ===\n";
    int index = 1;
    for (const auto& [name, account] : accounts) {
        std::cout << index++ << ". " << name << "\n";
    }
    std::cout << "0. ������\n";

    std::cout << "�������� ���� ��� ��������: ";
    int choice;
    std::cin >> choice;

    if (choice == 0) {
        std::cout << "��������.\n";
        return;
    }

    if (choice < 1 || choice > accounts.size()) {
        std::cout << "�������� �����!\n";
        return;
    }

    auto it = accounts.begin();
    std::advance(it, choice - 1);

    // ���� ������� ������� ����, ������������� �� "�����"
    if (currentAccount == &(it->second)) {
        currentAccount = &accounts["�����"];
    }

    accounts.erase(it);
    std::cout << "���� ������.\n";
}

void FinanceCore::renameAccount() {
    std::cout << "\n=== �������������� ����� ===\n";
    std::cout << "������� ���: " << currentAccount->get_name() << "\n";
    std::cout << "����� ��� (��� 0 ��� ������): ";

    std::string newName;
    std::cin.ignore();
    std::getline(std::cin, newName);

    if (newName == "0") {
        std::cout << "��������.\n";
        return;
    }

    if (accounts.find(newName) != accounts.end()) {
        std::cout << "���� � ����� ������ ��� ����������!\n";
        return;
    }

    // ������� ����� ���� � ������������� �������
    accounts[newName] = std::move(*currentAccount);
    accounts[newName].set_name(newName);

    // ������� ������ ������ (���� ��� �� "�����" ����)
    if (currentAccount->get_name() != "�����") {
        accounts.erase(currentAccount->get_name());
    }

    currentAccount = &accounts[newName];
    std::cout << "���� ������������.\n";
}