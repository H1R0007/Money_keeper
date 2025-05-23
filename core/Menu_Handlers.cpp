/**
 * @file Menu_Handlers.cpp
 * @brief ���������� ���������������� ���� � ������������� ��������
 *
 * @details ���� ������ �������� ��� ������ �������������� � �������������:
 * - ������� ���� �������
 * - ���� ���������� ������������
 * - ���� ���������� � �������
 * - ���� ���������� �������
 * - ��������������� �������
 *
 * @section ui_principles �������� UI
 * 1. ���������������: ������ ����� ���� ����
 * 2. ���������������: ����������� �������� �����
 * 3. ������������: ��������� �����
 * 4. �����������: ��������� ������� ������
 */

#include "FinanceCore.hpp"
#ifdef _WIN32
#include <windows.h> // ��� ������� �������
#endif

 /**
  * @brief ������� ���� ���� ����������
  *
  * @details ��������� �������� workflow �������:
  * 1. ����������� �������� ����
  * 2. ��������� ������ ������������
  * 3. ����� ��������������� �������
  * 4. ���������� ������
  *
  * @par ��������� ����:
  * @code{.unparsed}
  * +-------------------------------+
  * |      ���������� ��������     |
  * | �������� ������: RUB          |
  * | ����� ������: 1500.00         |
  * | ������� ����: ��������        |
  * | ������ �����: 1000.00         |
  * +-------------------------------+
  * | 1. �������� ����������        |
  * | 2. ����������� �������        |
  * | 3. ����������                 |
  * | ...                           |
  * | 0. �����                      |
  * +-------------------------------+
  * @endcode
  *
  * @note ������������� ��������� ������ ��� ������
  */

void FinanceCore::runMainMenu() {
    int choice;
    bool exitRequested = false;

    while (!exitRequested) {
        printMainMenu();
        choice = getMenuChoice();


        // ��������� ������
        switch (choice) {
        case 1:
            addTransaction();
            break;
        case 2:
            runTransactionMenu();
            break;
        case 3:
            runStatsMenu();
            break;
        case 4:
            manageAccounts();
            break;
        case 5:
            removeTransaction();
            break;
        case 6: {
            update_currency_rates([](bool success) {
                std::cout << (success ? "����� ���������!\n" : "������ ����������!\n");
                });
            break;
        }
        case 7:
            showCurrencyMenu();
            break;
        case 8:
            showBalanceByCurrency();
            break;
        case 9:
            runSearchMenu();
            break;
        case 0:
            saveData();
            std::cout << "+----------------------------+\n";
            std::cout << "| ������ ���������. �� ��������! |\n";
            std::cout << "+----------------------------+\n";
            exitRequested = true;
            break;
        default:
            std::cout << " ������: �������� ����� ����!\n";
            std::cout << "������� Enter ��� �����������...";
            std::cin.ignore();
            std::cin.get();
        }
    }
}

/**
 * @brief ���� ������ � �������� ����������
 *
 * @details �������������:
 * - ������ ������ ����������
 * - ���������� �� ����� (������/�������)
 * - ����� �� ���������
 * - ������� ������
 *
 * @par ������ �������������:
 * @code
 * === ������� ���������� ===
 * 1. ��� ����������
 * 2. ������ ������
 * 3. ������ �������
 * 4. �����
 * �������� ��������: 2
 * @endcode
 *
 * @see FinanceCore::viewAllTransactions()
 * @see FinanceCore::viewIncome()
 * @see FinanceCore::viewExpenses()
 */
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

/**
 * @brief ���� ���������� ���������
 *
 * @details �������� ��� ���� �������:
 * - ����� ������
 * - ������ �� ����������
 * - �������� �� �������
 * - ���������� �� �����
 *
 * @note ��� ������ ���������� ������� ������� ������
 *
 * @warning ��� ��������� ������� ��������� ������� 3 ������ ������
 */
void FinanceCore::runStatsMenu() {
    int choice;
    do {
        std::cout << "\n=== ���������� ==="
            << "\n1. ����� ������"
            << "\n2. �� ����������"
            << "\n3. �� �������"
            << "\n4. �� �������� �����"
            << "\n5. �����"
            << "\n��������: ";

        choice = getMenuChoice(); // ���������� ����� �������

        switch (choice) {
        case 1: showTotalBalance(); break;
        case 2: showByCategory(); break;
        case 3: showByMonth(); break;
        case 4: showCurrentAccountStats(); break;
        case 5: return; // ����� � ������ �������
        default: std::cout << "�������� �����!\n";
        }
    } while (true);
}

//���������� �������

/**
 * @brief ������� ����� ����
 * @throws std::invalid_argument ���� ��� ����� ��� ����������
 * @post ��������� ����� ���� � accounts
 *
 * @par ������:
 * @code
 * ������� �������� �����: ����������
 * ���� ������!
 * @endcode
 */

 /**
  * @details ��� ��������������:
  * 1. ��������� ����� ���� � ������������� �������
  * 2. ������ ���� ��������� (����� "������")
  * 3. ����������� currentAccount
  */
void FinanceCore::createAccount() {
    std::string name;
    std::cout << "������� �������� �����: ";
    std::cin.ignore();
    std::getline(std::cin, name);

    //std::lock_guard<std::mutex> lock(accounts_mutex_); // ��������� ����������

    if (accounts.find(name) != accounts.end()) {
        std::cout << "���� � ����� ������ ��� ����������!\n";
        return;
    }

    accounts.emplace(std::piecewise_construct,
        std::forward_as_tuple(name),
        std::forward_as_tuple(name));
    std::cout << "���� ������!\n";
}

/**
 * @brief �������� �������� ����
 * @details ���������� ������ ���� ������ � ���������
 * @post ������������� currentAccount �� ���������
 *
 * @warning ��� ������ currentAccount �� ����������
 */
void FinanceCore::selectAccount() {
   // std::lock_guard<std::mutex> lock(accounts_mutex_);

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
    currentAccount = &accounts.at(it->first);
    std::cout << "������ ����: " << it->first << "\n";
}

/**
 * @brief ������� ��������� ����
 * @pre � ������� ������ �������� ���� �� ���� ����
 * @post ���� �������� ������� ����, ������������� �� "�����"
 *
 * @throws std::logic_error ��� ������� ������� ��������� ����
 */
void FinanceCore::deleteAccount() {
   // std::lock_guard<std::mutex> lock(accounts_mutex_);

    if (accounts.size() <= 1) {
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
        currentAccount = &accounts.at("�����");
    }

    accounts.erase(it);
    std::cout << "���� ������.\n";
}


/**
 * @brief ��������������� ������� ����
 * @param[in] newName ����� ��� �����
 * @throws std::invalid_argument ���� ��� ��� ������
 *
 * @note ��� "������" ����� ������� �����, � �� ���������������
 */
void FinanceCore::renameAccount() {
    //std::lock_guard<std::mutex> lock(accounts_mutex_);

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

    // ������� ����� ���� � ������������ ����������
    accounts.emplace(std::piecewise_construct,
        std::forward_as_tuple(newName),
        std::forward_as_tuple(newName));
    accounts.at(newName).move_transactions_from(std::move(*currentAccount));

    // ������� ������ ������ (���� ��� �� "�����" ����)
    if (currentAccount->get_name() != "�����") {
        accounts.erase(currentAccount->get_name());
    }

    currentAccount = &accounts.at(newName);
    std::cout << "���� ������������.\n";
}

/**
 * @brief ���� ���������� �������
 *
 * @details ���������:
 * - ��������� ����� �����
 * - ������� ������������
 * - ��������������� �����
 * - �������� �������� ����
 *
 * @throws std::invalid_argument ��� ������� ������� ��������� ����
 *
 * @par ������������:
 * - ��������� �������� ���� ������
 * - ��������� "�����" ���� ��� ���������
 */
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

//����������� ����

/**
 * @brief ���������� ������� ���� �������
 *
 * @details ���������:
 * - ����� � ������� ���������� �������
 * - ������ ��������� ��������
 * - ������������� ������� �����
 *
 * @post ������� ������� ����� ������������
 *
 * @note ���������� ������������������ ������:
 * - Windows API �� Win32
 * - ANSI-���� �� Unix-��������
 */
void FinanceCore::printMainMenu() const {

    // ������� ������� ���������
#ifdef _WIN32
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = { 0, 0 };
    DWORD count;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hStdOut, &csbi);
    FillConsoleOutputCharacter(hStdOut, ' ', csbi.dwSize.X * csbi.dwSize.Y, coord, &count);
    SetConsoleCursorPosition(hStdOut, coord);
#else
    std::cout << "\033[2J\033[1;1H"; // ANSI escape codes
#endif

    double total_balance = 0;
    double current_account_balance = 0;
    for (const auto& [name, account] : accounts) {
        total_balance += account.get_balance_in_currency(currency_converter_, base_currency_);
    }
    if (currentAccount) {
        current_account_balance = currentAccount->get_balance_in_currency(currency_converter_, base_currency_);
    }


    std::cout << "+-------------------------------+\n";
    std::cout << "|      ���������� ��������     |\n";
    std::cout << "+-------------------------------+\n";
    std::cout << "| �������� ������: " << std::left << std::setw(11) << base_currency_ << " |\n";
    std::cout << "| ����� ������: " << std::setw(14) << std::fixed << std::setprecision(2) << total_balance << " |\n";
    std::cout << "| ������� ����: " << std::setw(14) << currentAccount->get_name() << " |\n";
    std::cout << "| ������ �����:    " << std::setw(11) << current_account_balance << " |\n";
    std::cout << "+-------------------------------+\n";
    std::cout << "| 1. �������� ����������        |\n";
    std::cout << "| 2. ����������� �������        |\n";
    std::cout << "| 3. ����������                 |\n";
    std::cout << "| 4. ���������� �������         |\n";
    std::cout << "| 5. ������� ����������         |\n";
    std::cout << "| 6. �������� ����� �����       |\n";
    std::cout << "| 7. �������� �������� ������   |\n";
    std::cout << "| 8. ������ �� �������          |\n";
    std::cout << "| 9. ����� �� �����             |\n";
    std::cout << "| 0. �����                      |\n";
    std::cout << "+-------------------------------+\n";
    std::cout << "> �������� ��������: ";
}

/**
 * @brief ���� ������ ������
 *
 * @details ����������:
 * - ������ ��������� �����
 * - ������� ������� ������
 * - ������������� �����
 *
 * @note �������������� ������:
 * - RUB (�����)
 * - USD (�������)
 * - EUR (����)
 *
 * @see CurrencyConverter::is_currency_supported()
 */
void FinanceCore::showCurrencyMenu() {
    std::cout << "\n��������� ������:\n";
    int i = 1;
    std::vector<std::string> currencies;

    // �������� ������ �������������� �����
    currencies.push_back("RUB");
    currencies.push_back("USD");
    currencies.push_back("EUR");
    // ����� �������� ������ ������

    for (const auto& curr : currencies) {
        std::cout << i++ << ". " << curr << "\n";
    }

    std::cout << "�������� ������ (0 - ������): ";
    int choice;
    std::cin >> choice;

    if (choice > 0 && choice <= currencies.size()) {
        setBaseCurrency(currencies[choice - 1]);
        std::cout << "�������� ������ �������� �� " << currencies[choice - 1] << "\n";
    }
}

/**
 * @brief ���� ������ �� �����
 *
 * @details ���������:
 * - ����� ���������� �����
 * - ���������� ����������
 * - �������� �����������
 * - ���������� ������� �����
 *
 * @note ������������ ���������� ����� ��� �������������� ������: 5
 *
 * @par ������ ������:
 * @code
 * === ����� �� ����� ===
 * ��������� ����: [���] [�����������]
 * ��������� ����:
 * 1. ���������
 * 2. �����������
 * ...
 * 8. ������ �����
 * 9. �������� �����
 * 0. �����
 * @endcode
 */
void FinanceCore::runSearchMenu() {
    const auto& available_tags = Transaction::get_available_tags();
    std::vector<std::string> selected_tags;

    while (true) {
        clearConsole();
        std::cout << "\n=== ����� �� ����� ===";
        std::cout << "\n��������� ����: ";
        for (const auto& tag : selected_tags) std::cout << "[" << tag << "] ";

        std::cout << "\n\n��������� ����:\n";
        for (size_t i = 0; i < available_tags.size(); ++i) {
            std::cout << i + 1 << ". " << available_tags[i] << "\n";
        }

        std::cout << "\n" << available_tags.size() + 1 << ". ������ �����\n";
        std::cout << available_tags.size() + 2 << ". �������� �����\n";
        std::cout << "0. �����\n�������� ��������: ";

        int choice = getMenuChoice();

        if (choice == 0) {
            break;
        }
        else if (choice == available_tags.size() + 1) { // �����
            if (selected_tags.empty()) {
                std::cout << "�� ������� �� ������ ����!\n";
                std::cout << "������� Enter ��� �����������...";
                std::cin.ignore();
                std::cin.get();
            }
            else {
                searchByTags(selected_tags);
                // ����� ������ �������� � ���� ������ �����
            }
        }
        else if (choice == available_tags.size() + 2) { // �������
            selected_tags.clear();
        }
        else if (choice > 0 && choice <= available_tags.size()) {
            const std::string& selected_tag = available_tags[choice - 1];
            if (std::find(selected_tags.begin(), selected_tags.end(), selected_tag) == selected_tags.end()) {
                if (selected_tags.size() < Transaction::MAX_TAGS) {
                    selected_tags.push_back(selected_tag);
                }
                else {
                    std::cout << "��������� ����� ��������� ����� (" << Transaction::MAX_TAGS << ")\n";
                    std::cout << "������� Enter ��� �����������...";
                    std::cin.ignore();
                    std::cin.get();
                }
            }
        }
    }
}