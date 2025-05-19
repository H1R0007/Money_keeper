
/**
 * @file Transactions.cpp
 * @brief Реализация методов работы с транзакциями
 */

#include "FinanceCore.hpp"
#include <Windows.h>
#include <iomanip>

 /**
  * @brief Добавляет новую транзакцию через интерактивный диалог
  * @details Пошаговый процесс:
  * 1. Выбор типа операции (доход/расход)
  * 2. Ввод суммы
  * 3. Указание категории
  * 4. Установка даты
  * 5. Добавление описания
  *
  * @throws std::invalid_argument При неверных входных данных
  * @post При успешном добавлении транзакция сохраняется в currentAccount
  *
  * @note Поддерживает:
  * - Отмену на любом этапе (ввод 0)
  * - Автоматическую установку текущей даты
  * - Валидацию всех полей
  *
  * @par Пример диалога:
  * @code{.unparsed}
  * === Новая транзакция ===
  * 1. Доход
  * 2. Расход
  * Выберите тип: 1
  * Введите сумму: 1000
  * Введите категорию: Зарплата
  * Дата (гггг-мм-дд): 2023-05-20
  * Описание: Аванс
  * Транзакция успешно добавлена!
  * @endcode
  */

// Добавление/удаление
void FinanceCore::addTransaction() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    Transaction newTrans;
    int step = 1;
    bool cancelled = false;

    while (step <= 6 && !cancelled) { // Увеличили количество шагов до 6
        try {
            switch (step) {

            case 1: { // Тип операции
                std::cout << "\n=== Новая транзакция ===\n";
                std::cout << "1. Доход\n2. Расход\n0. Отмена\nВыберите тип: ";
                int type = getMenuChoice();

                if (type == 0) {
                    cancelled = true;
                    std::cout << "Отменено.\n";
                    break;
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

                // Проверка поддержки валюты
                std::cout << "Введите валюту (RUB, USD, EUR): ";
                std::string currency;
                std::cin >> currency;

                if (!currency_converter_.is_currency_supported(currency)) {
                    std::cout << "Валюта не поддерживается. Используется RUB\n";
                    currency = "RUB";
                }

                newTrans.set_amount(amount);
                newTrans.set_currency(currency);
                step++;
                break;
            }

            case 3: { // Категория
                std::cout << "Введите категорию (0 для отмены): ";
                std::string category;
                clearInputBuffer();
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

            case 4: { // Дата
                std::cout << "Дата (гггг-мм-дд, enter для текущей, 0 для отмены): ";
                std::string date_str;
                std::cin.ignore(0);
                std::getline(std::cin, date_str);

                if (date_str == "0") {
                    cancelled = true;
                    std::cout << "Отменено.\n";
                    break;
                }

                if (date_str.empty()) {
                    step++; // Используем текущую дату по умолчанию
                    break;
                }

                try {
                    auto date = Date::from_string(date_str);
                    newTrans.set_date(date);
                    step++;
                }
                catch (...) {
                    throw std::invalid_argument("Неверный формат даты. Используйте ГГГГ-ММ-ДД");
                }
                break;
            }

            case 5: { // Описание
                std::cout << "Введите описание (enter чтобы пропустить, 0 для отмены): ";
                std::string desc;
                clearInputBuffer();
                std::getline(std::cin, desc);

                if (desc == "0") {
                    cancelled = true;
                    std::cout << "Отменено.\n";
                    break;
                }
                newTrans.set_description(desc);
                step++;
                break;
            }
            case 6: { // Шаг 6: Финализация и сохранение
                // Добавляем транзакцию
                currentAccount->addTransaction(newTrans);

                // Показываем сумму в рублях для информации
                double rubAmount = currency_converter_.convert(
                    newTrans.get_amount(),
                    newTrans.get_currency(),
                    "RUB"
                );

                std::cout << "Транзакция добавлена!\n"
                    << "Сумма: " << newTrans.get_amount() << " " << newTrans.get_currency()
                    << " (≈" << std::fixed << std::setprecision(2) << rubAmount << " RUB)\n";
                step++;
                break;
            }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Ошибка: " << e.what() << "\n";
            std::cin.clear();
            clearInputBuffer();
        }
    }
}
/**
 * @brief Удаляет транзакцию по ID
 * @details Процесс:
 * 1. Показывает список всех транзакций
 * 2. Запрашивает ID для удаления
 * 3. Удаляет транзакцию из currentAccount
 *
 * @pre В currentAccount должны быть транзакции
 * @throws std::out_of_range Если транзакция с указанным ID не найдена
 *
 * @note Поддерживает отмену операции (ввод 0)
 */
void FinanceCore::removeTransaction() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    //std::lock_guard<std::mutex> lock(accounts_mutex_);

    if (currentAccount->get_transactions().empty()) {
        std::cout << "Нет транзакций для удаления.\n";
        return;
    }

    viewAllTransactions();
    std::cout << "Введите ID транзакции для удаления (0 для отмены): ";
    int id = getMenuChoice();

    if (id == 0) return;

    if (currentAccount->removeTransaction(id)) {
        std::cout << "Транзакция удалена.\n";
    }
    else {
        std::cout << "Транзакция с ID " << id << " не найдена.\n";
    }
}


/**
 * @brief Выводит таблицу транзакций
 * @param transactions Список транзакций для отображения
 * @param title Заголовок таблицы
 *
 * @details Формат таблицы:
 * - ID (4 символа)
 * - Дата (YYYY-MM-DD)
 * - Сумма (10 символов)
 * - Категория (10 символов)
 * - Описание (12 символов)
 *
 * @note Автоматически обрезает длинные строки категории/описания
 */

//Просмотр
void FinanceCore::printTransactionsTable(const std::vector<Transaction>& transactions, const std::string& title) const {
    if (transactions.empty()) {
        std::cout << "\nНет транзакций (" << title << ") для отображения.\n";
        return;
    }

    std::cout << "\n=== " << title << " (" << transactions.size() << ") ===\n";
    std::cout << "+------+------------+----------+------------+------------+--------------+\n";
    std::cout << "|  ID  |    Дата    |   Тип    |   Сумма    |  Валюта    |  Категория   |\n";
    std::cout << "+------+------------+----------+------------+------------+--------------+\n";

    for (const auto& t : transactions) {
        std::cout << "| " << std::setw(4) << t.get_id() << " | "
            << t.get_date().to_string() << " | "
            << std::setw(8) << (t.get_type() == Transaction::Type::INCOME ? "Доход" : "Расход") << " | "
            << std::setw(10) << std::fixed << std::setprecision(2) << t.get_amount() << " | "
            << std::setw(10) << t.get_currency() << " | "
            << std::setw(12) << (t.get_category().empty() ? "-" : t.get_category().substr(0, 12)) << " |\n";
    }
    std::cout << "+------+------------+----------+------------+------------+--------------+\n" << std::flush;
}

/**
 * @brief Показывает только доходные транзакции
 * @see getFilteredTransactions()
 * @see printTransactionsTable()
 */
void FinanceCore::viewIncome() const {
    auto incomes = getFilteredTransactions(Transaction::Type::INCOME);
    printTransactionsTable(incomes, "Доходы");
}

/**
 * @brief Показывает только расходные транзакции
 * @see getFilteredTransactions()
 * @see printTransactionsTable()
 */
void FinanceCore::viewExpenses() const {
    auto expenses = getFilteredTransactions(Transaction::Type::EXPENSE);
    printTransactionsTable(expenses, "Расходы");
}

/**
 * @brief Фильтрует транзакции по типу
 * @param type Тип транзакции (INCOME/EXPENSE)
 * @return Вектор транзакций указанного типа
 *
 * @note Всегда возвращает копию транзакций (не ссылки)
 */

//Вспомогательные
std::vector<Transaction> FinanceCore::getFilteredTransactions(Transaction::Type type) const {
    std::vector<Transaction> result;
    for (const auto& t : currentAccount->get_transactions()) {
        if (t.get_type() == type) {
            result.push_back(t);
        }
    }
    return result;
}

/**
 * @brief Показывает все транзакции текущего счета
 * @details Формат вывода:
 * - ID | Дата | Тип | Сумма | Категория | Описание
 *
 * @post Очищает консоль перед выводом (clearConsole())
 *
 * @note Использует сокращение длинных текстовых полей (добавляет "...")
 */
void FinanceCore::viewAllTransactions() const {
    clearConsole();

    std::cout << "\n=== Все транзакции ===\n";
    std::cout << "+------+------------+----------+------------+------------+--------------+--------------+\n";
    std::cout << "|  ID  |    Дата    |   Тип    |   Сумма    |  Валюта    |  Категория   |  Описание    |\n";
    std::cout << "+------+------------+----------+------------+------------+--------------+--------------+\n";

    for (const auto& t : currentAccount->get_transactions()) {
        std::cout << "| " << std::setw(4) << t.get_id() << " | "
            << t.get_date().to_string() << " | "
            << std::setw(8) << (t.get_type() == Transaction::Type::INCOME ? "Доход" : "Расход") << " | "
            << std::setw(10) << std::fixed << std::setprecision(2) << t.get_amount() << " | "
            << std::setw(10) << t.get_currency() << " | "
            << std::setw(12) << (t.get_category().empty() ? "-" : t.get_category()) << " | "
            << std::setw(12) << (t.get_description().empty() ? "-" : t.get_description()) << " |\n";
    }
    std::cout << "+------+------------+----------+------------+------------+--------------+--------------+\n" << std::flush;
}