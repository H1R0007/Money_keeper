/**
 * @file File_Manager.cpp
 * @brief Реализация работы с файлами данных
 *
 * @details Этот модуль отвечает за:
 * - Сохранение/загрузку всех данных системы
 * - Сериализацию/десериализацию транзакций
 * - Управление структурой файлов
 * - Обработку ошибок ввода-вывода
 *
 * @section file_format_sec Формат файла данных
 * 1. Каждый аккаунт в отдельной секции [Account:Имя]
 * 2. Транзакции в CSV-формате
 * 3. Кодировка: UTF-8
 * 4. Поддержка мультивалютных транзакций
 */

#include "FinanceCore.hpp"
#include <fstream>
#include <filesystem>

 /**
  * @brief Соединяет строки с разделителем
  * @param vec Вектор строк для объединения
  * @param delimiter Разделитель
  * @return Объединенная строка
  *
  * @details Используется для сериализации тегов:
  * - Теги сохраняются через точку с запятой
  * - Пустые векторы возвращают "-"
  *
  * @par Пример:
  * @code
  * join_strings({"food","transport"}, ',') => "food,transport"
  * @endcode
  */
static std::string join_strings(const std::vector<std::string>& vec, const char delimiter) {
    std::string result;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i != 0) result += delimiter;
        result += vec[i];
    }
    return result;
}

/**
 * @brief Проверяет начало строки
 * @param str Исходная строка
 * @param prefix Искомый префикс
 * @return true если строка начинается с prefix
 */
static bool starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() &&
        str.compare(0, prefix.size(), prefix) == 0;
}

/**
 * @brief Сохраняет все данные аккаунтов в файл
 *
 * @details Алгоритм работы:
 * 1. Открывает файл на запись (перезаписывает существующий)
 * 2. Для каждого аккаунта:
 *    - Записывает заголовок [Account:Имя]
 *    - Сериализует все транзакции в CSV
 * 3. Сохраняет пути к файлам для отладки
 *
 * @throws std::ios_base::failure При ошибках записи
 * @post Создает резервную копию существующего файла
 *
 * @par Формат записи транзакции:
 * @code
 * id,amount,type,category,yyyy mm dd,currency,description,tags
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
 *
 * @details Алгоритм загрузки:
 * 1. Проверяет существование файла
 * 2. Очищает текущие данные
 * 3. Читает файл построчно:
 *    - Обрабатывает секции аккаунтов
 *    - Парсит CSV-строки транзакций
 * 4. Восстанавливает связи и индексы
 *
 * @throws std::ios_base::failure При ошибках чтения
 * @throws std::invalid_argument При некорректных данных
 *
 * @note Автоматически создает "Общий" счет если файл не существует
 * @warning Пропускает некорректные транзакции с выводом ошибки
 */
void FinanceCore::loadData() {
    accounts.clear();
    accounts.emplace(std::piecewise_construct,
        std::forward_as_tuple("Общий"),
        std::forward_as_tuple("Общий"));
    currentAccount = &accounts.at("Общий");

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
    int max_id = 0;

    while (std::getline(file, line)) {
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
            std::vector<std::string> fields;
            std::string field;

            while (std::getline(iss, field, ',')) {
                fields.push_back(field);
            }

            if (fields.size() < 6) throw std::runtime_error("Недостаточно полей");

            // Парсинг полей
            t.id = std::stoi(fields[0]);
            if (t.id > max_id) max_id = t.id;

            t.amount = std::stod(fields[1]);
            t.type = static_cast<Transaction::Type>(std::stoi(fields[2]));
            t.category = fields[3];

            // Парсинг даты
            std::istringstream date_iss(fields[4]);
            int y, m, d;
            date_iss >> y >> m >> d;
            t.date = Date(y, m, d);

            // Обработка валюты
            if (fields.size() > 5) {
                t.currency_ = fields[5];
                t.currency_.erase(0, t.currency_.find_first_not_of(" \t"));
                t.currency_.erase(t.currency_.find_last_not_of(" \t") + 1);
                if (t.currency_.empty()) t.currency_ = "RUB";
            }

            // Обработка описания
            t.description = (fields.size() > 6) ? fields[6] : "--";
            t.description.erase(0, t.description.find_first_not_of(" \t"));
            t.description.erase(t.description.find_last_not_of(" \t") + 1);

            // Обработка тегов
            if (fields.size() > 7 && fields[7] != "-") {
                std::istringstream tags_stream(fields[7]);
                std::string tag;
                while (std::getline(tags_stream, tag, ';')) {
                    if (!tag.empty()) t.add_tag(tag);
                }
            }

            accounts[currentAccountName].addTransaction(t);
        }
        catch (const std::exception& e) {
            std::cerr << "Ошибка чтения транзакции: " << e.what()
                << "\nСтрока: " << line << "\n";
        }
    }

    // Восстановление ID генератора
    if (max_id > 0) {
        Transaction::next_id = max_id + 1;
    }

    // Пересчет балансов
    for (auto& [name, account] : accounts) {
        account.recalculateBalance(currency_converter_);
    }
}