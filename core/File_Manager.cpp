#include "FinanceCore.hpp"
#include <fstream>
#include <filesystem>

void FinanceCore::saveData() {
    std::ofstream file("transactions.dat");
    if (!file) {
        std::cerr << "������: �� ���� ������� ���� ��� ������!\n";
        return;
    }

    for (const auto& [name, account] : accounts) {
        file << "[Account:" << name << "]\n";
        for (const auto& t : account.get_transactions()) {
            file << t << "\n";
        }
    }
    std::cout << "������ ��������� �: " << std::filesystem::absolute("transactions.dat") << "\n";
}

void FinanceCore::loadData() {
    std::ifstream file(dataFile);
    accounts.clear(); // ������� ������������ �����

    // ������� ����� ���� �� ���������
    accounts["�����"] = Account("�����");
    currentAccount = &accounts["�����"];

    if (!file.is_open()) {
        std::cerr << "���� ������ �� ������. ������ ����� ����� ����.\n";
        return;
    }

    std::string currentAccountName;
    std::string line;
    bool hasValidData = false;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // ��������� ��������� ��������
        if (line.find("[Account:") != std::string::npos) {
            size_t start = line.find(':') + 1;
            size_t end = line.find(']');
            if (end == std::string::npos) continue;

            currentAccountName = line.substr(start, end - start);
            accounts[currentAccountName] = Account(currentAccountName);
            hasValidData = true;
        }
        // ��������� ����������
        else if (!currentAccountName.empty()) {
            try {
                std::istringstream iss(line);
                Transaction t;
                if (iss >> t) {
                    accounts[currentAccountName].addTransaction(t);
                }
            }
            catch (const std::exception& e) {
                std::cerr << "������ ������ ����������: " << e.what()
                    << "\n������: " << line << "\n";
            }
        }
    }

    if (!hasValidData) {
        std::cerr << "� ����� ��� �������� ������. ������������ ����� ���� �� ���������.\n";
    }
}