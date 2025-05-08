#pragma once
#include "FinanceCore.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <numeric>

Account& FinanceCore::getCurrentAccount() {
    if (currentAccount == nullptr) {
        currentAccount = &accounts["Общий"];
    }
    return *currentAccount;
}

void FinanceCore::ensureDefaultAccount() {
    if (accounts.empty()) {
        accounts["Общий"] = Account("Общий");
    }
    if (currentAccount == nullptr) {
        currentAccount = &accounts["Общий"];
    }
}

void FinanceCore::saveData() {
    ensureDefaultAccount(); // Гарантируем наличие счета перед сохранением

    std::ofstream file(dataFile);
    if (!file.is_open()) {
        throw std::runtime_error("Не удалось открыть файл для сохранения");
    }

    for (const auto& [name, account] : accounts) {
        file << "[Account:" << name << "]\n";
        for (const auto& t : account.get_transactions()) {
            file << t << "\n";
        }
    }
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


FinanceCore::FinanceCore() : dataFile("transactions.dat") {
    accounts["Общий"] = Account("Общий");
    currentAccount = &accounts["Общий"];
    ensureDefaultAccount();
    loadData();
}

void FinanceCore::printMainMenu() const {
    std::cout << "\n=== Главное меню ==="
        << "\nТекущий счет: " << currentAccount->get_name()
        << " (Баланс: " << currentAccount->get_balance() << ")"
        << "\n1. Добавить транзакцию"
        << "\n2. История операций"
        << "\n3. Статистика"
        << "\n4. Управление счетами"
        << "\n5. Удалить транзакцию"
        << "\n6. Выход"
        << "\nВыберите действие: ";
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

        std::cin >> choice;

        switch (choice) {
        case 1: showTotalBalance(); break;
        case 2: showByCategory(); break;
        case 3: showByMonth(); break;
        case 4: showCurrentAccountStats(); break;
        case 5: return;
        default: std::cout << "Неверный выбор!\n";
        }
    } while (true);
}

void FinanceCore::showCurrentAccountStats() const {
    double income = 0;
    double expenses = 0;

    for (const auto& t : currentAccount->get_transactions()) {
        if (t.get_type() == Transaction::Type::INCOME) {
            income += t.get_amount();
        }
        else {
            expenses += t.get_amount();
        }
    }

    std::cout << "\n=== Статистика (" << currentAccount->get_name() << ") ===\n"
        << "Транзакций: " << currentAccount->get_transactions().size() << "\n"
        << "Доходы: " << income << "\n"
        << "Расходы: " << expenses << "\n"
        << "Баланс: " << currentAccount->get_balance() << "\n";
}

void FinanceCore::showTotalBalance() const {
    double totalIncome = 0;
    double totalExpenses = 0;

    for (const auto& [name, account] : accounts) {
        for (const auto& t : account.get_transactions()) {
            if (t.get_type() == Transaction::Type::INCOME) {
                totalIncome += t.get_amount();
            }
            else {
                totalExpenses += t.get_amount();
            }
        }
    }

    std::cout << "\n=== Общая статистика ===\n"
        << "Доходы: " << totalIncome << " руб.\n"
        << "Расходы: " << totalExpenses << " руб.\n"
        << "Баланс: " << (totalIncome - totalExpenses) << " руб.\n";
}

void FinanceCore::showByCategory() const {
    std::map<std::string, std::pair<double, double>> categories; // категория -> <доходы, расходы>

    for (const auto& [name, account] : accounts) {
        for (const auto& t : account.get_transactions()) {
            if (t.get_type() == Transaction::Type::INCOME) {
                categories[t.get_category()].first += t.get_amount();
            }
            else {
                categories[t.get_category()].second += t.get_amount();
            }
        }
    }

    std::cout << "\n=== По категориям ===\n";
    for (const auto& [category, amounts] : categories) {
        std::cout << category << ": "
            << "+" << amounts.first << " / -" << amounts.second
            << " (итого: " << (amounts.first - amounts.second) << ")\n";
    }
}

void FinanceCore::showByMonth() const {
    std::map<std::pair<int, int>, std::pair<double, double>> months; // <год, месяц> -> <доходы, расходы>

    for (const auto& [name, account] : accounts) {
        for (const auto& t : account.get_transactions()) {
            auto monthKey = std::make_pair(
                t.get_date().get_year(),
                t.get_date().get_month()
            );

            if (t.get_type() == Transaction::Type::INCOME) {
                months[monthKey].first += t.get_amount();
            }
            else {
                months[monthKey].second += t.get_amount();
            }
        }
    }

    std::cout << "\n=== По месяцам ===\n";
    for (const auto& [month, amounts] : months) {
        std::cout << month.first << "-" << std::setfill('0') << std::setw(2) << month.second << ": "
            << "+" << amounts.first << " / -" << amounts.second
            << " (баланс: " << (amounts.first - amounts.second) << ")\n";
    }
}


void FinanceCore::runMainMenu() {
    int choice;
    bool exitRequested = false;

    while (!exitRequested) {
        try {
            // Очистка экрана (кросс-платформенный способ)
#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif

            std::cout << "╔══════════════════════════════╗\n";
            std::cout << "║    ФИНАНСОВЫЙ МЕНЕДЖЕР      ║\n";
            std::cout << "╠══════════════════════════════╣\n";
            std::cout << "║ Текущий счёт: " << std::left << std::setw(15)
                << currentAccount->get_name().substr(0, 15)
                << " ║ Баланс: " << std::setw(8) << currentAccount->get_balance() << " ║\n";
            std::cout << "╠══════════════════════════════╣\n";
            std::cout << "║ 1. Добавить транзакцию       ║\n";
            std::cout << "║ 2. Просмотреть историю       ║\n";
            std::cout << "║ 3. Статистика                ║\n";
            std::cout << "║ 4. Управление счетами        ║\n";
            std::cout << "║ 5. Удалить транзакцию        ║\n";
            std::cout << "║ 0. Выход                     ║\n";
            std::cout << "╚══════════════════════════════╝\n";
            std::cout << "➤ Выберите действие: ";

            // Проверка ввода
            if (!(std::cin >> choice)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                throw std::runtime_error("Ошибка: введите число от 0 до 5");
            }

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
                std::cout << "╔══════════════════════════════╗\n";
                std::cout << "║ Данные сохранены. До свидания!║\n";
                std::cout << "╚══════════════════════════════╝\n";
                exitRequested = true;
                break;
            default:
                std::cout << "⚠ Ошибка: неверный пункт меню!\n";
                std::cout << "Нажмите Enter для продолжения...";
                std::cin.ignore();
                std::cin.get();
            }

        }
        catch (const std::exception& e) {
            std::cerr << "\n⚠ Ошибка: " << e.what() << "\n";
            std::cout << "Нажмите Enter для продолжения...";
            std::cin.ignore();
            std::cin.get();
        }
    }
}

void FinanceCore::addTransaction() {
    Transaction newTrans;
    int step = 1;
    bool cancelled = false;

    while (step <= 5 && !cancelled) {
        try {
            switch (step) {
            case 1: { // Шаг 1: Тип операции
                std::cout << "\n=== Новая транзакция ===\n";
                std::cout << "1. Доход\n2. Расход\n0. Отмена\nВыберите тип: ";
                int type;
                std::cin >> type;

                if (type == 0) {
                    cancelled = true;
                    std::cout << "Отменено.\n";
                    break;
                }

                if (type != 1 && type != 2) {
                    throw std::invalid_argument("Неверный тип операции");
                }

                newTrans.set_type(type == 1 ? Transaction::Type::INCOME : Transaction::Type::EXPENSE);
                step++;
                break;
            }

            case 2: { // Шаг 2: Сумма
                std::cout << "Введите сумму (0 для отмены): ";
                double amount;
                std::cin >> amount;

                if (amount == 0) {
                    cancelled = true;
                    std::cout << "Отменено.\n";
                    break;
                }

                newTrans.set_amount(amount);
                step++;
                break;
            }

            case 3: { // Шаг 3: Категория
                std::cout << "Введите категорию (0 для отмены): ";
                std::string category;
                std::cin.ignore();
                std::getline(std::cin, category);

                if (category == "0") {
                    cancelled = true;
                    std::cout << "Отменено.\n";
                    break;
                }

                newTrans.set_category(category);
                step++;
                break;
            }

            case 4: { // Шаг 4: Дата
                std::cout << "Дата (гггг мм дд, 0 0 0 для текущей, -1 для отмены): ";
                int y, m, d;
                std::cin >> y >> m >> d;

                if (y == -1) {
                    cancelled = true;
                    std::cout << "Отменено.\n";
                    break;
                }

                if (y == 0 && m == 0 && d == 0) {
                    // Используем текущую дату (по умолчанию)
                    step++;
                    break;
                }

                Date customDate(y, m, d);
                newTrans.set_date(customDate);
                step++;
                break;
            }

            case 5: { // Шаг 5: Описание
                std::cout << "Введите описание (опционально, 0 для отмены): ";
                std::string desc;
                std::cin.ignore();
                std::getline(std::cin, desc);

                if (desc == "0") {
                    cancelled = true;
                    std::cout << "Отменено.\n";
                    break;
                }

                newTrans.set_description(desc);

                // Финализация
                currentAccount->addTransaction(newTrans);
                std::cout << "Транзакция успешно добавлена!\n";
                step++;
                break;
            }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Ошибка: " << e.what() << "\nПопробуйте снова.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

void FinanceCore::validateData() {
    for (auto& [name, account] : accounts) {
        double calculatedBalance = 0;
        for (const auto& t : account.get_transactions()) {
            calculatedBalance += t.get_signed_amount();
        }

        if (std::abs(account.get_balance() - calculatedBalance) > 0.01) {
            std::cerr << "Внимание: несоответствие баланса для счета " << name
                << "! Автоматическая коррекция...\n";
            account.recalculateBalance();
        }
    }
}

bool FinanceCore::validateData() const {
    if (accounts.empty()) return false;
    for (const auto& [name, account] : accounts) {
        double balance = 0;
        for (const auto& t : account.get_transactions()) {
            balance += t.get_signed_amount();
        }
        if (std::abs(balance - account.get_balance()) > 0.01) {
            return false;
        }
    }
    return true;
}

void FinanceCore::removeTransaction() {
    if (currentAccount->get_transactions().empty()) {
        std::cout << "Нет транзакций для удаления.\n";
        return;
    }

    viewAllTransactions();
    std::cout << "Введите ID транзакции для удаления (0 для отмены): ";
    int id;
    std::cin >> id;

    if (id == 0) return;

    currentAccount->removeTransaction(id);
    std::cout << "Транзакция удалена.\n";
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

void FinanceCore::viewAllTransactions() const {
    if (currentAccount->get_transactions().empty()) {
        std::cout << "Нет транзакций для отображения.\n";
        return;
    }

    std::cout << "\n=== Все транзакции (" << currentAccount->get_transactions().size() << ") ===\n";
    std::cout << "┌──────┬──────────┬──────────┬────────────┬────────────┬──────────────┐\n";
    std::cout << "│  ID  │   Дата   │   Тип    │   Сумма    │ Категория  │  Описание    │\n";
    std::cout << "├──────┼──────────┼──────────┼────────────┼────────────┼──────────────┤\n";

    for (const auto& t : currentAccount->get_transactions()) {
        std::cout << "│ " << std::setw(4) << t.get_id() << " │ "
            << t.get_date().to_string() << " │ "
            << std::setw(8) << (t.get_type() == Transaction::Type::INCOME ? "Доход" : "Расход") << " │ "
            << std::setw(10) << t.get_amount() << " │ "
            << std::setw(10) << t.get_category().substr(0, 10) << " │ "
            << std::setw(12) << t.get_description().substr(0, 12) << " │\n";
    }
    std::cout << "└──────┴──────────┴──────────┴────────────┴────────────┴──────────────┘\n";
}


void FinanceCore::viewIncome() const {
    auto incomes = getFilteredTransactions(Transaction::Type::INCOME);
    printTransactionsTable(incomes, "Доходы");
}

void FinanceCore::viewExpenses() const {
    auto expenses = getFilteredTransactions(Transaction::Type::EXPENSE);
    printTransactionsTable(expenses, "Расходы");
}

std::vector<Transaction> FinanceCore::getFilteredTransactions(Transaction::Type type) const {
    std::vector<Transaction> result;
    for (const auto& t : currentAccount->get_transactions()) {
        if (t.get_type() == type) {
            result.push_back(t);
        }
    }
    return result;
}

void FinanceCore::printTransactionsTable(const std::vector<Transaction>& transactions, const std::string& title) const {
    if (transactions.empty()) {
        std::cout << "\nНет транзакций (" << title << ") для отображения.\n";
        return;
    }

    std::cout << "\n=== " << title << " (" << transactions.size() << ") ===\n";
    std::cout << "┌──────┬──────────┬────────────┬────────────┬──────────────┐\n";
    std::cout << "│  ID  │   Дата   │   Сумма    │ Категория  │  Описание    │\n";
    std::cout << "├──────┼──────────┼────────────┼────────────┼──────────────┤\n";

    for (const auto& t : transactions) {
        std::cout << "│ " << std::setw(4) << t.get_id() << " │ "
            << t.get_date().to_string() << " │ "
            << std::setw(10) << t.get_amount() << " │ "
            << std::setw(10) << t.get_category().substr(0, 10) << " │ "
            << std::setw(12) << t.get_description().substr(0, 12) << " │\n";
    }
    std::cout << "└──────┴──────────┴────────────┴────────────┴──────────────┘\n";
}

void FinanceCore::printTransactionMenu() const {
    std::cout << "\n=== Меню транзакций ==="
        << "\n1. Показать все транзакции"
        << "\n2. Показать доходы"
        << "\n3. Показать расходы"
        << "\n4. Сортировать по дате (новые сначала)"
        << "\n5. Сортировать по сумме"
        << "\n6. Назад"
        << "\nВыберите действие: ";
}

void FinanceCore::printStatsMenu() const {
    std::cout << "\n=== Меню статистики ==="
        << "\n1. Общий баланс"
        << "\n2. Статистика по категориям"
        << "\n3. Статистика по месяцам"
        << "\n4. Топ-5 самых крупных расходов"
        << "\n5. Назад"
        << "\nВыберите действие: ";
}

void FinanceCore::showTopExpenses() const {
    std::vector<Transaction> expenses;

    // Собираем все расходы
    std::copy_if(transactions.begin(), transactions.end(),
        std::back_inserter(expenses),
        [](const Transaction& t) {
            return t.get_type() == Transaction::Type::EXPENSE;
        });

    // Сортируем по убыванию суммы
    std::sort(expenses.begin(), expenses.end(),
        [](const Transaction& a, const Transaction& b) {
            return a.get_amount() > b.get_amount();
        });

    // Выводим топ-5 или меньше, если расходов меньше 5
    std::cout << "\n=== Топ-5 самых крупных расходов ===\n";
    int count = std::min(5, static_cast<int>(expenses.size()));
    for (int i = 0; i < count; ++i) {
        std::cout << i + 1 << ". " << expenses[i].get_summary() << "\n";
    }

    if (expenses.empty()) {
        std::cout << "Нет данных о расходах.\n";
    }
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
