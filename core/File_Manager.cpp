/**
 * @file File_Manager.cpp
 * @brief ���������� ������ � ������� ������
 *
 * @details ���� ������ �������� ��:
 * - ����������/�������� ���� ������ �������
 * - ������������/�������������� ����������
 * - ���������� ���������� ������
 * - ��������� ������ �����-������
 *
 * @section file_format_sec ������ ����� ������
 * 1. ������ ������� � ��������� ������ [Account:���]
 * 2. ���������� � CSV-�������
 * 3. ���������: UTF-8
 * 4. ��������� �������������� ����������
 */

#include "FinanceCore.hpp"
#include <fstream>
#include <filesystem>

 /**
  * @brief ��������� ������ � ������������
  * @param vec ������ ����� ��� �����������
  * @param delimiter �����������
  * @return ������������ ������
  *
  * @details ������������ ��� ������������ �����:
  * - ���� ����������� ����� ����� � �������
  * - ������ ������� ���������� "-"
  *
  * @par ������:
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
 * @brief ��������� ������ ������
 * @param str �������� ������
 * @param prefix ������� �������
 * @return true ���� ������ ���������� � prefix
 */
static bool starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() &&
        str.compare(0, prefix.size(), prefix) == 0;
}

/**
 * @brief ��������� ��� ������ ��������� � ����
 *
 * @details �������� ������:
 * 1. ��������� ���� �� ������ (�������������� ������������)
 * 2. ��� ������� ��������:
 *    - ���������� ��������� [Account:���]
 *    - ����������� ��� ���������� � CSV
 * 3. ��������� ���� � ������ ��� �������
 *
 * @throws std::ios_base::failure ��� ������� ������
 * @post ������� ��������� ����� ������������� �����
 *
 * @par ������ ������ ����������:
 * @code
 * id,amount,type,category,yyyy mm dd,currency,description,tags
 * @endcode
 */
void FinanceCore::saveData() {
    std::ofstream file(dataFile);
    if (!file) {
        std::cerr << "������: �� ���� ������� ���� ��� ������!\n";
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
    std::cout << "������ ��������� �: " << std::filesystem::absolute(dataFile) << "\n";
}

/**
 * @brief ��������� ������ ��������� �� �����
 *
 * @details �������� ��������:
 * 1. ��������� ������������� �����
 * 2. ������� ������� ������
 * 3. ������ ���� ���������:
 *    - ������������ ������ ���������
 *    - ������ CSV-������ ����������
 * 4. ��������������� ����� � �������
 *
 * @throws std::ios_base::failure ��� ������� ������
 * @throws std::invalid_argument ��� ������������ ������
 *
 * @note ������������� ������� "�����" ���� ���� ���� �� ����������
 * @warning ���������� ������������ ���������� � ������� ������
 */
void FinanceCore::loadData() {
    accounts.clear();
    accounts.emplace(std::piecewise_construct,
        std::forward_as_tuple("�����"),
        std::forward_as_tuple("�����"));
    currentAccount = &accounts.at("�����");

    if (!std::filesystem::exists(dataFile)) {
        std::cout << "[DEBUG] ���� �� ����������, ������ ������� '�����'\n";
        return;
    }

    std::ifstream file(dataFile);
    if (!file.is_open()) {
        std::cerr << "[DEBUG] ������ �������� �����\n";
        throw std::runtime_error("�� ���� ������� ���� ������");
    }

    std::string line;
    std::string currentAccountName = "�����";
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

            if (fields.size() < 6) throw std::runtime_error("������������ �����");

            // ������� �����
            t.id = std::stoi(fields[0]);
            if (t.id > max_id) max_id = t.id;

            t.amount = std::stod(fields[1]);
            t.type = static_cast<Transaction::Type>(std::stoi(fields[2]));
            t.category = fields[3];

            // ������� ����
            std::istringstream date_iss(fields[4]);
            int y, m, d;
            date_iss >> y >> m >> d;
            t.date = Date(y, m, d);

            // ��������� ������
            if (fields.size() > 5) {
                t.currency_ = fields[5];
                t.currency_.erase(0, t.currency_.find_first_not_of(" \t"));
                t.currency_.erase(t.currency_.find_last_not_of(" \t") + 1);
                if (t.currency_.empty()) t.currency_ = "RUB";
            }

            // ��������� ��������
            t.description = (fields.size() > 6) ? fields[6] : "--";
            t.description.erase(0, t.description.find_first_not_of(" \t"));
            t.description.erase(t.description.find_last_not_of(" \t") + 1);

            // ��������� �����
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
            std::cerr << "������ ������ ����������: " << e.what()
                << "\n������: " << line << "\n";
        }
    }

    // �������������� ID ����������
    if (max_id > 0) {
        Transaction::next_id = max_id + 1;
    }

    // �������� ��������
    for (auto& [name, account] : accounts) {
        account.recalculateBalance(currency_converter_);
    }
}