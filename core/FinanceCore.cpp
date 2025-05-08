#pragma once
#include "FinanceCore.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <numeric>

void FinanceCore::saveData() {
    std::ofstream file(dataFile);
    for (const auto& [name, account] : accounts) {
        file << "[Account:" << name << "]\n";
        for (const auto& t : account.get_transactions()) {
            file << t << "\n";
        }
    }
}

void FinanceCore::loadData() {
    std::ifstream file(dataFile);
    std::string line;
    Account* current = nullptr;

    while (std::getline(file, line)) {
        if (line.find("[Account:") != std::string::npos) {
            std::string name = line.substr(9, line.size() - 10);
            accounts[name] = Account(name);
            current = &accounts[name];
        }
        else if (current) {
            std::istringstream iss(line);
            Transaction t;
            if (iss >> t) {
                current->addTransaction(t);
            }
        }
    }
}

void FinanceCore::viewIncome() const {
    std::cout << "\n=== Доходы ===\n";
    for (const auto& t : transactions) {
        if (t.get_type() == Transaction::Type::INCOME) {
            std::cout << t.get_summary() << "\n";
        }
    }
}

void FinanceCore::viewExpenses() const {
    std::cout << "\n=== Расходы ===\n";
    for (const auto& t : transactions) {
        if (t.get_type() == Transaction::Type::EXPENSE) {
            std::cout << t.get_summary() << "\n";
        }
    }
}

FinanceCore::FinanceCore() {
    accounts["Общий"] = Account("Общий");
    currentAccount = &accounts["Общий"];
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

void FinanceCore::showStatistics() const {
    int choice;
    do {
        std::cout << "\n=== Статистика ==="
            << "\n1. Общий баланс"
            << "\n2. По категориям"
            << "\n3. По месяцам"
            << "\n4. Назад"
            << "\nВыберите действие: ";

        std::cin >> choice;

        switch (choice) {
        case 1: showTotalBalance(); break;
        case 2: showByCategory(); break;
        case 3: showByMonth(); break;
        case 4: return;
        default: std::cout << "Неверный выбор!\n";
        }
    } while (true);
}

void FinanceCore::showTotalBalance() const {
    double balance = std::accumulate(transactions.begin(), transactions.end(), 0.0,
        [](double sum, const Transaction& t) {
            return sum + t.get_signed_amount();
        });

    std::cout << "\n=== Общий баланс ===\n";
    std::cout << "Текущий баланс: " << balance << " руб.\n";
}

void FinanceCore::showByCategory() const {
    std::map<std::string, double> categories;

    for (const auto& t : transactions) {
        categories[t.get_category()] += t.get_signed_amount();
    }

    std::cout << "\n=== По категориям ===\n";
    for (const auto& [category, amount] : categories) {
        std::cout << category << ": " << amount << " руб.\n";
    }
}

void FinanceCore::showByMonth() const {
    std::map<std::pair<int, int>, double> months; // <year, month>

    for (const auto& t : transactions) {
        auto key = std::make_pair(t.get_date().get_year(),
            t.get_date().get_month());
        months[key] += t.get_signed_amount();
    }

    std::cout << "\n=== По месяцам ===\n";
    for (const auto& [month, amount] : months) {
        std::cout << month.first << "-" << month.second << ": "
            << amount << " руб.\n";
    }
}


void FinanceCore::runMainMenu() {
    int choice;
    do {
        printMainMenu();
        std::cin >> choice;
        std::cin.ignore(); // Очистка буфера

        switch (choice) {
        case 1: addTransaction(); break;
        case 2: runTransactionMenu(); break;
        case 3: runStatsMenu(); break;
        case 4: saveData(); return;
        default: std::cout << "Неверный выбор!\n";
        }
    } while (true);
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
    std::cout << "\n=== Все транзакции ===\n";
    for (const auto& t : transactions) {
        std::cout << t.get_summary() << "\n";
    }
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

void FinanceCore::runStatsMenu() {
    int choice;
    do {
        printStatsMenu();
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
        case 1:
            showTotalBalance();
            break;
        case 2:
            showByCategory();
            break;
        case 3:
            showByMonth();
            break;
        case 4:
            showTopExpenses();
            break;
        case 5:
            return;
        default:
            std::cout << "Неверный выбор! Попробуйте снова.\n";
        }

        if (choice >= 1 && choice <= 4) {
            std::cout << "\nНажмите Enter для продолжения...";
            std::cin.get();
        }
    } while (true);
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
