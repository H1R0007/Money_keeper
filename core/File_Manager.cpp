#include "FinanceCore.hpp"
#include <fstream>
#include <filesystem>

void FinanceCore::saveData() {
    std::ofstream file("transactions.dat");
    if (!file) {
        std::cerr << "ОШИБКА: Не могу открыть файл для записи!\n";
        return;
    }

    for (const auto& [name, account] : accounts) {
        file << "[Account:" << name << "]\n";
        for (const auto& t : account.get_transactions()) {
            file << t << "\n";
        }
    }
    std::cout << "Данные сохранены в: " << std::filesystem::absolute("transactions.dat") << "\n";
}

void FinanceCore::loadData() {
    std::ifstream file(dataFile);
    accounts.clear(); // Очищаем существующие счета

    // Создаем общий счет по умолчанию
    accounts["Общий"] = Account("Общий");
    currentAccount = &accounts["Общий"];

    if (!file.is_open()) {
        std::cerr << "Файл данных не найден. Создан новый общий счет.\n";
        return;
    }

    std::string currentAccountName;
    std::string line;
    bool hasValidData = false;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // Обработка заголовка аккаунта
        if (line.find("[Account:") != std::string::npos) {
            size_t start = line.find(':') + 1;
            size_t end = line.find(']');
            if (end == std::string::npos) continue;

            currentAccountName = line.substr(start, end - start);
            accounts[currentAccountName] = Account(currentAccountName);
            hasValidData = true;
        }
        // Обработка транзакции
        else if (!currentAccountName.empty()) {
            try {
                std::istringstream iss(line);
                Transaction t;
                if (iss >> t) {
                    accounts[currentAccountName].addTransaction(t);
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Ошибка чтения транзакции: " << e.what()
                    << "\nСтрока: " << line << "\n";
            }
        }
    }

    if (!hasValidData) {
        std::cerr << "В файле нет валидных данных. Используется общий счет по умолчанию.\n";
    }
}