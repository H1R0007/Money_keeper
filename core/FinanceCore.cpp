/**
 * @file FinanceCore.cpp
 * @brief ���������� ���� ���������� �������
 *
 * @details ���� ������ ��������:
 * - ������������� �������
 * - ������ � ������� ������
 * - �������� ��������
 * - ������� �������
 *
 * @section init_sec ������� �������������
 * 1. ����������� ����� � ������
 * 2. �������� ����������� ����������
 * 3. �������� ����������� ������
 * 4. ������������� �������� ������
 */

#include "FinanceCore.hpp"
#include "currency/CurrencyFetcher.hpp"
#include <thread>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <numeric>
#include <future>
#ifdef _WIN32
#include <windows.h>
#endif

 /**
  * @brief �������� ������ ���� � ����� ������
  * @param filename ��� �����
  * @return ���������� ���� � ������� {executable_path}/data/{filename}
  *
  * @details �������� ������:
  * 1. ���������� ���� � ������������ �����
  * 2. ������� ������������� /data
  * 3. ���������� ������ ����
  *
  * @throws std::filesystem::filesystem_error ��� ������� ������ � �������� ��������
  */
std::string FinanceCore::getDataPath(const std::string& filename) {
    std::filesystem::path exePath = std::filesystem::current_path();
    std::filesystem::path dataPath = exePath / "data" / filename;
    return dataPath.string();
}

/**
 * @brief ���������� ������� �������� ����
 * @return ������ �� Account
 *
 * @details ��������:
 * - ���� currentAccount �� ����������, ���������� ���� "�����"
 * - ������� �� ���������� nullptr
 *
 * @note ��������������� �� ���� accounts_mutex_
 */
Account& FinanceCore::getCurrentAccount() {
    if (!currentAccount) {
        currentAccount = &accounts.at("�����");
    }
    return *currentAccount;
}

/**
 * @brief ������������ ������� ������ �� ���������
 *
 * @details ��������:
 * 1. ���� accounts ����, ������� ���� "�����"
 * 2. ���� currentAccount ��������������, ������������� ��� �� "�����"
 *
 * @post ����������� accounts.size() >= 1
 * @post ����������� currentAccount != nullptr
 */
void FinanceCore::ensureDefaultAccount() {
    if (accounts.empty()) {
        accounts.try_emplace("�����", "�����");
    }
    if (!currentAccount) {
        currentAccount = &accounts.at("�����");
    }
}

/**
 * @brief �������� ����������� �������
 *
 * @details ������ ������� �������������:
 * 1. ��������� ����� � ������ (������� ��������):
 *    - Linux: /proc/self/exe
 *    - Windows: ������� ����������
 *    - Fallback: ������������� ����
 * 2. �������� ����������� ����������
 * 3. ������������� ������������ �����
 * 4. ����������� ���������� �������� ������
 * 5. �������� ����������� ������
 *
 * @throws std::runtime_error ��� ����������� ������� �������������
 */
FinanceCore::FinanceCore() {
    // ������������� �����
    std::filesystem::path dataPath;
    try {
        // Linux-������ ����������� ����
        dataPath = std::filesystem::canonical("/proc/self/exe").parent_path() / "transactions.dat";
    }
    catch (...) {
        try {
            // Windows fallback
            dataPath = getDataPath("transactions.dat");
        }
        catch (...) {
            // ��������� fallback
            dataPath = "transactions.dat";
        }
    }

    dataFile = dataPath.string();
    std::cout << "���� ������ ����� �������� �: " << dataFile << std::endl;

    // �������� ����������
    ensureDataDirectory();
    std::filesystem::create_directories("CurrencyDat");

    // ������������� ���������
    accounts.try_emplace("�����", "�����");
    currentAccount = &accounts.at("�����");

    // ����������� ���������� ������ �����
    try {
        std::promise<bool> promise;
        auto future = promise.get_future();
        update_currency_rates([&promise](bool success) {
            promise.set_value(success);
            });
        future.wait();

        if (!future.get()) {
            std::cout << "��������������: �� ������� �������� ����� �����\n";
        }
    }
    catch (...) {
        std::cerr << "������ ��� ���������� ������ �����\n";
    }

    ensureDefaultAccount();

    loadData();
}

/**
 * @brief ��������� ����������� ���������� ������
 * @return true ���� ������� ���� ������ ������������� �����������
 *
 * @details �������� ��������:
 * 1. ��� ������� ����� ��������� ����� ���� ����������
 * 2. ���������� � ������� ��������
 * 3. ���������� �����������: 0.01 (������ ����������)
 *
 * @note ������������ ��� �������� ������
 */
bool FinanceCore::validateData() const {
    if (accounts.empty()) return false;

    for (const auto& [name, account] : accounts) {
        double calculated = 0;
        for (const auto& t : account.get_transactions()) {
            calculated += t.get_amount_in_rub(currency_converter_);
        }

        if (std::abs(calculated - account.get_balance()) > 0.01) {
            return false;
        }
    }
    return true;
}

/**
 * @brief �������� ����� ������������ �� ����
 * @return ����� �� 0 �� N (� ����������� �� ����)
 *
 * @details �����������:
 * - ������ �� ������������� �����
 * - ������� ������ ����� ����������
 * - ���� �� ��������� ��������� ��������
 *
 * @note ���������� clearInputBuffer() ��� �������
 */
int FinanceCore::getMenuChoice() const {
    int choice;
    while (true) {
        if (std::cin >> choice) {
            clearInputBuffer();
            return choice;
        }
        else {
            std::cout << "������ �����. ����������, ������� �����: ";
            std::cin.clear();
            clearInputBuffer();
        }
    }
}

/**
 * @brief ������� ������� �����������������
 *
 * @details ����������:
 * - Windows: WinAPI (FillConsoleOutputCharacter)
 * - Unix: ANSI escape codes
 *
 * @post ������ ��������������� � ������� (0,0)
 */
void FinanceCore::clearConsole() const {
#ifdef _WIN32
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = { 0, 0 };
    DWORD count;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hStdOut, &csbi);
    FillConsoleOutputCharacter(hStdOut, ' ', csbi.dwSize.X * csbi.dwSize.Y, coord, &count);
    SetConsoleCursorPosition(hStdOut, coord);
#else
    std::cout << "\033[2J\033[1;1H";
#endif
}

/**
 * @brief ������� ����� ����� std::cin
 *
 * @details ������������ �����:
 * - cin >> ��� �������������� �������
 * - ������ �����
 *
 * @note �������� � ����� ����������� ���������� ��������
 */
void FinanceCore::clearInputBuffer() const {
    std::cin.clear();
    if (std::cin.rdbuf()->in_avail() > 0) {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

/**
 * @brief ��������� ����� ����� ����������
 * @param callback ������� ��������� ������ (bool success)
 *
 * @details ������������������ ��������:
 * 1. ��������� CurrencyFetcher � ��������� ������
 * 2. ��� ������ ��������� ����� � ����
 * 3. ��� ������� �������� ��������� ����������� �����
 * 4. �������� callback � �����������
 *
 * @note ���������� std::async ��� �������������
 */
void FinanceCore::update_currency_rates(std::function<void(bool)> callback) {
    CurrencyFetcher fetcher;
    fetcher.fetch_rates([this, callback](const auto& new_rates) {
        bool success = false;

        // 1. ���������� ������
        if (!new_rates.empty()) {
            currency_converter_.set_rates(new_rates);
            currency_converter_.save_rates_to_file("CurrencyDat/currency_rates.json");
            success = true;
        }
        else {
            success = currency_converter_.load_rates_from_file("CurrencyDat/currency_rates.json");
        }

        // 2. �������� ��������
        if (success) {
            std::lock_guard<std::mutex> acc_lock(accounts_mutex_);
            for (auto& [name, account] : accounts) {
                account.recalculateBalance(currency_converter_);
            }
        }

        callback(success);
        });
}

/**
 * @brief ������������ ����� ����� ��������
 * @param amount �������� �����
 * @param from �������� ������ (��� ISO 4217)
 * @param to ������� ������ (�� ��������� RUB)
 * @return ����������������� �����
 *
 * @throws std::runtime_error ���:
 * - ���������������� ������
 * - ���������� ������ �����
 * - ������� ����������
 *
 * @note ���������� ������� ����� CurrencyConverter
 */
double FinanceCore::convert_currency(double amount, const std::string& from,
    const std::string& to) const {
    return currency_converter_.convert(amount, from, to);
}

/**
 * @brief ������������� ������� ������ �������
 * @param currency ��� ������ (ISO 4217)
 *
 * @throws std::invalid_argument ���� ������ �� ��������������
 * @post ��� ������ ����� � ��������� ������
 */
void FinanceCore::setBaseCurrency(const std::string& currency) {
    if (currency_converter_.is_currency_supported(currency)) {
        base_currency_ = currency;
    }
    else {
        throw std::invalid_argument("������ �� ��������������");
    }
}

/**
 * @brief ���������� ������� ������� ������
 * @return ��� ������ (�������� "RUB")
 */
std::string FinanceCore::getBaseCurrency() const {
    return base_currency_;
}

/**
 * @brief ������� ���������� ��� ������ ���� �����������
 *
 * @details ��������� � �������:
 * - �������� ���������� ������
 * - ������������� ��� ������ �����
 *
 * @note ���������� ������������� ��� �������������
 */
void FinanceCore::ensureDataDirectory() {
    std::filesystem::path dataDir = std::filesystem::path(dataFile).parent_path();
    if (!std::filesystem::exists(dataDir)) {
        std::filesystem::create_directories(dataDir);
    }
}