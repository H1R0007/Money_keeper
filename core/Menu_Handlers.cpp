#include "FinanceCore.hpp"
#ifdef _WIN32
#include <windows.h> // Для очистки консоли
#endif

//Основные меню
void FinanceCore::runMainMenu() {
    int choice;
    bool exitRequested = false;

    while (!exitRequested) {
        printMainMenu();
        choice = getMenuChoice();


        // Обработка выбора
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
        case 0:
            saveData();
            std::cout << "+----------------------------+\n";
            std::cout << "| Данные сохранены. До свидания! |\n";
            std::cout << "+----------------------------+\n";
            exitRequested = true;
            break;
        default:
            std::cout << " Ошибка: неверный пункт меню!\n";
            std::cout << "Нажмите Enter для продолжения...";
            std::cin.ignore();
            std::cin.get();
        }
    }
}

void FinanceCore::runTransactionMenu() {
    int choice;
    do {
        std::cout << "\n=== История транзакций ==="
            << "\n1. Все транзакции"
            << "\n2. Только доходы"
            << "\n3. Только расходы"
            << "\n4. Назад"
            << "\nВыберите действие: ";

        std::cin >> choice;

        switch (choice) {
        case 1: viewAllTransactions(); break;
        case 2: viewIncome(); break;
        case 3: viewExpenses(); break;
        case 4: return;
        default: std::cout << "Неверный выбор!\n";
        }
    } while (true);
}

void FinanceCore::runStatsMenu() {
    int choice;
    do {
        std::cout << "\n=== Статистика ==="
            << "\n1. Общий баланс"
            << "\n2. По категориям"
            << "\n3. По месяцам"
            << "\n4. По текущему счету"
            << "\n5. Назад"
            << "\nВыберите: ";

        choice = getMenuChoice(); // Используем новую функцию

        switch (choice) {
        case 1: showTotalBalance(); break;
        case 2: showByCategory(); break;
        case 3: showByMonth(); break;
        case 4: showCurrentAccountStats(); break;
        case 5: return; // Выход с одного нажатия
        default: std::cout << "Неверный выбор!\n";
        }
    } while (true);
}

//Управление счетами
void FinanceCore::createAccount() {
    std::string name;
    std::cout << "Введите название счета: ";
    std::cin.ignore();
    std::getline(std::cin, name);

    if (accounts.find(name) != accounts.end()) {
        std::cout << "Счет с таким именем уже существует!\n";
        return;
    }

    accounts[name] = Account(name);
    std::cout << "Счет создан!\n";
}

void FinanceCore::selectAccount() {
    if (accounts.empty()) {
        std::cout << "Нет доступных счетов.\n";
        return;
    }

    std::cout << "\n=== Выбор счета ===\n";
    int index = 1;
    for (const auto& [name, account] : accounts) {
        std::cout << index++ << ". " << name
            << " (Баланс: " << account.get_balance() << ")\n";
    }
    std::cout << "0. Отмена\n";

    std::cout << "Выберите счет: ";
    int choice;
    std::cin >> choice;

    if (choice == 0) {
        std::cout << "Отменено.\n";
        return;
    }

    if (choice < 1 || choice > accounts.size()) {
        std::cout << "Неверный выбор!\n";
        return;
    }

    auto it = accounts.begin();
    std::advance(it, choice - 1);
    currentAccount = &(it->second);
    std::cout << "Выбран счет: " << it->first << "\n";
}

void FinanceCore::deleteAccount() {
    if (accounts.size() <= 1) { // Нельзя удалить последний счет
        std::cout << "Должен остаться хотя бы один счет!\n";
        return;
    }

    std::cout << "\n=== Удаление счета ===\n";
    int index = 1;
    for (const auto& [name, account] : accounts) {
        std::cout << index++ << ". " << name << "\n";
    }
    std::cout << "0. Отмена\n";

    std::cout << "Выберите счет для удаления: ";
    int choice;
    std::cin >> choice;

    if (choice == 0) {
        std::cout << "Отменено.\n";
        return;
    }

    if (choice < 1 || choice > accounts.size()) {
        std::cout << "Неверный выбор!\n";
        return;
    }

    auto it = accounts.begin();
    std::advance(it, choice - 1);

    // Если удаляем текущий счет, переключаемся на "Общий"
    if (currentAccount == &(it->second)) {
        currentAccount = &accounts["Общий"];
    }

    accounts.erase(it);
    std::cout << "Счет удален.\n";
}

void FinanceCore::renameAccount() {
    std::cout << "\n=== Переименование счета ===\n";
    std::cout << "Текущее имя: " << currentAccount->get_name() << "\n";
    std::cout << "Новое имя (или 0 для отмены): ";

    std::string newName;
    std::cin.ignore();
    std::getline(std::cin, newName);

    if (newName == "0") {
        std::cout << "Отменено.\n";
        return;
    }

    if (accounts.find(newName) != accounts.end()) {
        std::cout << "Счет с таким именем уже существует!\n";
        return;
    }

    // Создаем новый счет с перенесенными данными
    accounts[newName] = std::move(*currentAccount);
    accounts[newName].set_name(newName);

    // Удаляем старую запись (если это не "Общий" счет)
    if (currentAccount->get_name() != "Общий") {
        accounts.erase(currentAccount->get_name());
    }

    currentAccount = &accounts[newName];
    std::cout << "Счет переименован.\n";
}

void FinanceCore::manageAccounts() {
    int choice;
    do {
        std::cout << "\n=== Управление счетами ==="
            << "\n1. Создать счет"
            << "\n2. Удалить счет"
            << "\n3. Выбрать счет"
            << "\n4. Назад"
            << "\nВыберите действие: ";

        std::cin >> choice;

        switch (choice) {
        case 1: createAccount(); break;
        case 2: deleteAccount(); break;
        case 3: selectAccount(); break;
        case 4: return;
        default: std::cout << "Неверный выбор!\n";
        }
    } while (true);
}

//Отображение меню
void FinanceCore::printMainMenu() const {

    // Очищаем консоль правильно
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

    std::cout << "+----------------------------+\n";
    std::cout << "|     ФИНАНСОВЫЙ МЕНЕДЖЕР     |\n";
    std::cout << "+----------------------------+\n";

    // Форматируем вывод с фиксированной длиной
    std::string accountName = currentAccount->get_name();
    if (accountName.length() > 15) accountName = accountName.substr(0, 12) + "...";

    std::cout << "| Текущий счёт: " << std::left << std::setw(12) << accountName
        << " | Баланс: " << std::setw(8) << std::fixed << std::setprecision(2)
        << currentAccount->get_balance() << " |\n";
    std::cout << "+----------------------------+\n";
    std::cout << "| 1. Добавить транзакцию      |\n";
    std::cout << "| 2. Просмотреть историю      |\n";
    std::cout << "| 3. Статистика               |\n";
    std::cout << "| 4. Управление счетами       |\n";
    std::cout << "| 5. Удалить транзакцию       |\n";
    std::cout << "| 0. Выход                    |\n";
    std::cout << "+----------------------------+\n";
    std::cout << "> Выберите действие: ";
}

void FinanceCore::printTransactionMenu() const {
    std::cout << "\n--- История транзакций ---\n"
        << "1. Все транзакции\n"
        << "2. Только доходы\n"
        << "3. Только расходы\n"
        << "4. Сортировать по дате\n"
        << "5. Сортировать по сумме\n"
        << "6. Назад\n"
        << "Выберите действие: ";

}

void FinanceCore::printStatsMenu() const {
    std::cout << "\n--- Меню статистики ---\n"
        << "1. Общий баланс\n"
        << "2. По категориям\n"
        << "3. По месяцам\n"
        << "4. Топ-5 расходов\n"
        << "5. Назад\n"
        << "Выберите действие: ";

}