/**
 * @file File_Manager.cpp
 * @brief Реализация методов сохранения и загрузки данных
 * @author Сонин Михаил/Эксузян Давид
 * @version 1.0
 * @date 2025-05-16
 */

#include "FinanceCore.hpp"
#include <fstream>
#include <filesystem>

 /**
  * @brief Сохраняет все данные аккаунтов в файл
  * @details Формат файла:
  * - Каждый аккаунт начинается с метки [Account:ИмяАккаунта]
  * - Транзакции сохраняются последовательно для каждого аккаунта
  *
  * @post Создается/перезаписывается файл transactions.dat в текущей директории
  * @throws std::ios_base::failure При ошибках записи в файл
  *
  * @par Пример файла:
  * @code
  * [Account:Основной]
  * 2023 5 15 1500.00 Зарплата
  * 2023 5 20 -750.50 Аренда
  * [Account:Сбережения]
  * 2023 5 10 500.00 Накопления
  * @endcode
  */
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

/**
 * @brief Загружает данные аккаунтов из файла
 * @details Формат файла должен соответствовать saveData()
 *
 * @pre Файл данных должен существовать (иначе создается аккаунт "Общий")
 * @post Все существующие аккаунты очищаются перед загрузкой
 * @throws std::ios_base::failure При ошибках чтения файла
 * @throws std::invalid_argument При некорректных данных транзакций
 *
 * @note Автоматически создает аккаунт "Общий" если файл не существует или пуст
 *
 * @warning При обнаружении некорректных транзакций они пропускаются с выводом ошибки
 */
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