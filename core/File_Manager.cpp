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

static std::string join_strings(const std::vector<std::string>& vec, const char delimiter) {
    std::string result;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i != 0) result += delimiter;
        result += vec[i];
    }
    return result;
}

bool starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() &&
        str.compare(0, prefix.size(), prefix) == 0;
}
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
    std::ofstream file(dataFile);
    if (!file) {
        std::cerr << "ОШИБКА: Не могу открыть файл для записи!\n";
        return;
    }

    for (const auto& [name, account] : accounts) {
        file << "[Account:" << name << "]\n";
        for (const auto& t : account.get_transactions()) {
            file << t.get_id() << ","
                << t.get_amount() << ","
                << static_cast<int>(t.get_type()) << ","
                << t.get_category() << ","
                << t.get_date() << ","
                << t.get_currency() << ","
                << t.get_description() << ","
                << (t.get_tags().empty() ? "-" : join_strings(t.get_tags(), ';')) << "\n";
        }
    }
    std::cout << "Данные сохранены в: " << std::filesystem::absolute(dataFile) << "\n";
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
    accounts.clear();
    accounts.emplace(std::piecewise_construct,
        std::forward_as_tuple("Общий"),
        std::forward_as_tuple("Общий"));
    currentAccount = &accounts.at("Общий");

    std::cout << "[DEBUG] Загрузка данных...\n";
    if (!std::filesystem::exists(dataFile)) {
        std::cout << "[DEBUG] Файл не существует, создан аккаунт 'Общий'\n";
        return;
    }

    std::ifstream file(dataFile);
    if (!file.is_open()) {
        std::cerr << "[DEBUG] Ошибка открытия файла\n";
        throw std::runtime_error("Не могу открыть файл данных");
    }

    std::string line;
    std::string currentAccountName = "Общий";
    bool hasConversionNeeded = false;
    int max_id = 0; // Для отслеживания максимального ID

    while (std::getline(file, line)) {
        std::cout << "[DEBUG] Прочитана строка: " << line << "\n";
        if (line.empty()) continue;

        if (starts_with(line, "[Account:")) {
            size_t end = line.find(']');
            if (end == std::string::npos) continue;

            currentAccountName = line.substr(9, end - 9);
            if (accounts.find(currentAccountName) == accounts.end()) {
                accounts.emplace(std::piecewise_construct,
                    std::forward_as_tuple(currentAccountName),
                    std::forward_as_tuple(currentAccountName));
            }
            continue;
        }

        Transaction t;
        try {
            std::istringstream iss(line);
            std::string field;
            std::vector<std::string> fields;

            while (std::getline(iss, field, ',')) {
                if (!field.empty()) fields.push_back(field);
            }

            if (fields.size() < 6) throw std::runtime_error("Недостаточно полей");

            // Парсим ID и обновляем max_id
            t.id = std::stoi(fields[0]);
            if (t.id > max_id) max_id = t.id;

            // Остальной парсинг
            t.amount = std::stod(fields[1]);
            t.type = static_cast<Transaction::Type>(std::stoi(fields[2]));
            t.category = fields[3];

            std::istringstream date_iss(fields[4]);
            int y, m, d;
            date_iss >> y >> m >> d;
            t.date = Date(y, m, d);

            if (fields.size() > 5) {
                t.currency_ = fields[5];
                t.currency_.erase(0, t.currency_.find_first_not_of(" \t"));
                t.currency_.erase(t.currency_.find_last_not_of(" \t") + 1);
                if (t.currency_.empty()) t.currency_ = "RUB";
            }
            else {
                t.currency_ = "RUB";
                hasConversionNeeded = true;
            }

            t.description = (fields.size() > 6) ? fields[6] : "--";
            t.description.erase(0, t.description.find_first_not_of(" \t"));
            t.description.erase(t.description.find_last_not_of(" \t") + 1);


            if (fields.size() > 7 && fields[7] != "-") {
                std::istringstream tags_stream(fields[7]);
                std::string tag;
                while (std::getline(tags_stream, tag, ';')) {
                    if (!tag.empty()) {
                        t.add_tag(tag);
                    }
                }
            }
            accounts[currentAccountName].addTransaction(t);
        }
        catch (const std::exception& e) {
            std::cerr << "Ошибка чтения транзакции: " << e.what()
                << "\nСтрока: " << line << "\n";
        }
    }

    // Устанавливаем next_id для новых транзакций
    if (max_id > 0) {
        Transaction::next_id = max_id + 1;
        std::cout << "[DEBUG] Установлено next_id: " << Transaction::next_id << "\n";
    }

    // Пересчитываем балансы
    for (auto& [name, account] : accounts) {
        account.recalculateBalance(currency_converter_);
    }
}
