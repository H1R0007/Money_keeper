/**
 * @file Transactions.cpp
 * @brief Реализация операций с финансовыми транзакциями
 *
 * @details Этот модуль содержит реализацию методов для:
 * - Добавления новых транзакций через интерактивный интерфейс
 * - Удаления существующих транзакций
 * - Отображения транзакций в различных представлениях
 * - Работы с тегами транзакций
 *
 * @section transaction_flow_sec Рабочий процесс
 * 1. Создание транзакции через многошаговый диалог
 * 2. Валидация всех вводимых данных
 * 3. Конвертация валют при необходимости
 * 4. Добавление тегов
 * 5. Сохранение в текущий аккаунт
 *
 * @warning Все денежные значения хранятся в положительном формате,
 *          знак определяется типом операции (доход/расход)
 */

#include "FinanceCore.hpp"
#include <Windows.h>
#include <iomanip>

 /**
 * @brief Добавляет новую транзакцию через интерактивный диалог
 *
 * @details Реализует пошаговый процесс создания транзакции:
 * 1. Выбор типа операции (доход/расход)
 * 2. Ввод суммы и валюты
 * 3. Указание категории
 * 4. Установка даты (по умолчанию текущая)
 * 5. Добавление описания
 * 6. Управление тегами
 *
 * @throws std::invalid_argument При невалидных входных данных
 * @post При успешном выполнении транзакция добавляется в currentAccount
 *
 * @note Особенности:
 * - Поддержка отмены на любом этапе (ввод 0)
 * - Автоматическая валидация всех полей
 * - Визуализация суммы в рублях для небазовых валют
 * - Ограничение на количество тегов (MAX_TAGS)
 *
 * @par Пример диалога:
 * @code{.unparsed}
 * === Новая транзакция ===
 * 1. Доход | 2. Расход | 0. Отмена
 * > 1
 * Введите сумму: 100
 * Введите валюту: USD
 * Категория: Зарплата
 * Дата [YYYY-MM-DD]: 2023-05-20
 * Описание: Аванс за май
 * @endcode
 */

void FinanceCore::addTransaction() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    Transaction newTrans;
    int step = 1;
    bool cancelled = false;

    while (step <= 7 && !cancelled) { // Увеличили количество шагов до 6
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
            case 6: { // Шаг 6: Теги
                const auto& available_tags = Transaction::get_available_tags();
                bool tag_adding_finished = false;

                while (!tag_adding_finished) {
                    clearConsole();
                    std::cout << "\n=== Управление тегами ("
                        << newTrans.get_tags().size() << "/"
                        << Transaction::MAX_TAGS << ") ===\n";

                    // Вывод текущих тегов
                    if (!newTrans.get_tags().empty()) {
                        std::cout << "Текущие теги: ";
                        for (size_t i = 0; i < newTrans.get_tags().size(); ++i) {
                            std::cout << (i > 0 ? ", " : "") << "[" << newTrans.get_tags()[i] << "]";
                        }
                        std::cout << "\n\n";
                    }

                    // Вывод доступных тегов
                    std::cout << "Доступные теги:\n";
                    for (size_t i = 0; i < available_tags.size(); ++i) {
                        std::cout << i + 1 << ". " << available_tags[i] << "\n";
                    }

                    std::cout << "\n0. Завершить добавление тегов\n";
                    if (!newTrans.get_tags().empty()) {
                        std::cout << "99. Удалить тег\n";
                    }
                    std::cout << "Выберите действие: ";

                    int choice = getMenuChoice();

                    if (choice == 0) {
                        tag_adding_finished = true;
                        step++;
                    }
                    else if (choice == 99 && !newTrans.get_tags().empty()) {
                        // Удаление тега
                        std::cout << "Выберите тег для удаления:\n";
                        for (size_t i = 0; i < newTrans.get_tags().size(); ++i) {
                            std::cout << i + 1 << ". " << newTrans.get_tags()[i] << "\n";
                        }
                        std::cout << "0. Отмена\n> ";

                        int tag_choice = getMenuChoice();
                        if (tag_choice > 0 && tag_choice <= newTrans.get_tags().size()) {
                            newTrans.remove_tag(tag_choice - 1); // Используем новый метод
                        }
                    }
                    else if (choice > 0 && choice <= available_tags.size()) {
                        // Добавление тега
                        try {
                            const std::string& selected_tag = available_tags[choice - 1];
                            if (std::find(newTrans.get_tags().begin(), newTrans.get_tags().end(), selected_tag)
                                != newTrans.get_tags().end()) {
                                std::cout << "Этот тег уже добавлен!\n";
                            }
                            else if (newTrans.get_tags().size() >= Transaction::MAX_TAGS) {
                                std::cout << "Достигнут лимит тегов (" << Transaction::MAX_TAGS << ")\n";
                            }
                            else {
                                newTrans.add_tag(selected_tag); // Используем новый метод
                            }
                        }
                        catch (const std::exception& e) {
                            std::cerr << "Ошибка: " << e.what() << "\n";
                        }
                        std::cout << "Нажмите Enter для продолжения...";
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cin.get();
                    }
                }
                break;
            }

            case 7: { // Финализация
                currentAccount->addTransaction(newTrans);

                double rubAmount = currency_converter_.convert(
                    newTrans.get_amount(),
                    newTrans.get_currency(),
                    "RUB"
                );

                std::cout << "\nТранзакция добавлена!\n"
                    << "Сумма: " << newTrans.get_amount() << " " << newTrans.get_currency()
                    << " (?" << std::fixed << std::setprecision(2) << rubAmount << " RUB)\n";

                if (!newTrans.get_tags().empty()) {
                    std::cout << "Теги: ";
                    for (const auto& tag : newTrans.get_tags()) {
                        std::cout << "[" << tag << "] ";
                    }
                    std::cout << "\n";
                }

                step++;
                break;
            }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Ошибка: " << e.what() << "\n";
            std::cin.clear();
            clearInputBuffer();
            std::cout << "Нажмите Enter для продолжения...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
        }
    }
}

/**
 * @brief Удаляет транзакцию по ID
 *
 * @details Процесс удаления:
 * 1. Отображение списка всех транзакций
 * 2. Ввод ID транзакции для удаления
 * 3. Поиск и удаление из currentAccount
 *
 * @pre В currentAccount должны существовать транзакции
 * @throws std::out_of_range Если транзакция с указанным ID не найдена
 *
 * @note Поддерживает отмену операции (ввод 0)
 * @warning Удаленные транзакции невозможно восстановить
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
 * @brief Выводит табличное представление транзакций
 *
 * @param transactions Список транзакций для отображения
 * @param title Заголовок таблицы
 *
 * @details Формат таблицы:
 * +------+------------+----------+------------+------------+--------------+
 * |  ID  |    Дата    |   Тип    |   Сумма    |  Валюта    |  Категория   |
 * +------+------------+----------+------------+------------+--------------+
 *
 * @note Особенности:
 * - Автоматическое выравнивание столбцов
 * - Обрезка длинных текстовых полей
 * - Цветовое выделение типов операций (реализуется через WinAPI)
 * - Поддержка UTF-8 символов
 */

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
 *
 * @details Фильтрует транзакции currentAccount по типу INCOME
 * и выводит их в табличном формате через printTransactionsTable()
 *
 * @see printTransactionsTable()
 * @see getFilteredTransactions()
 */
void FinanceCore::viewIncome() const {
    auto incomes = getFilteredTransactions(Transaction::Type::INCOME);
    printTransactionsTable(incomes, "Доходы");
}

/**
 * @brief Показывает только расходные транзакции
 *
 * @details Фильтрует транзакции currentAccount по типу EXPENSE
 * и выводит их в табличном формате через printTransactionsTable()
 *
 * @see printTransactionsTable()
 * @see getFilteredTransactions()
 */
void FinanceCore::viewExpenses() const {
    auto expenses = getFilteredTransactions(Transaction::Type::EXPENSE);
    printTransactionsTable(expenses, "Расходы");
}

/**
 * @brief Фильтрует транзакции по типу
 *
 * @param type Тип транзакции (INCOME/EXPENSE)
 * @return Вектор транзакций указанного типа
 *
 * @note Возвращает копии транзакций (не ссылки)
 * @complexity O(n), где n - количество транзакций
 */

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
 *
 * @details Формат вывода:
 * +------+------------+----------+------------+------------+--------------+--------------+
 * |  ID  |    Дата    |   Тип    |   Сумма    |  Валюта    |  Категория   |  Описание    |
 * +------+------------+----------+------------+------------+--------------+--------------+
 *
 * @post Очищает консоль перед выводом
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

/**
 * @brief Возвращает список доступных тегов
 *
 * @return Константная ссылка на вектор предопределенных тегов
 *
 * @details Стандартные теги включают:
 * - Основные категории расходов (еда, транспорт)
 * - Категории доходов (зарплата, инвестиции)
 * - Специальные метки (важно, налоговый вычет)
 *
 * @note Теги локализованы на русский язык
 */
const std::vector<std::string>& Transaction::get_available_tags() {
    static const std::vector<std::string> TAGS = {
        "продукты", "супермаркет", "ресторан",
        "транспорт", "такси", "метро", "бензин",
        "развлечения", "кино", "концерт", "хобби",
        "здоровье", "аптека", "врач", "спортзал",
        "образование", "книги", "курсы",
        "коммуналка", "электричество", "интернет",
        "аренда", "ремонт", "одежда",
        "подарки", "путешествия", "другое"
    };
    return TAGS;
}