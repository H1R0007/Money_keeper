/**
 * @file Menu_Handlers.cpp
 * @brief Реализация пользовательского интерфейса FinanceCore
 */


#include "FinanceCore.hpp"
#ifdef _WIN32
#include <windows.h> // Для очистки консоли
#endif

 /**
   * @brief Главный цикл меню приложения
   * @details Обрабатывает:
   * - Добавление/удаление транзакций
   * - Просмотр статистики
   * - Управление счетами
   * - Автоматическое сохранение при выходе
   *
   * @par Логика работы:
   * @code
   * while (не запрошен выход) {
   *   1. Отображает главное меню
   *   2. Получает выбор пользователя
   *   3. Выполняет соответствующее действие
   * }
   * @endcode
   */

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
        case 6: {
            update_currency_rates([](bool success) {
                std::cout << (success ? "Курсы обновлены!\n" : "Ошибка обновления!\n");
                });
            break;
        }
        case 7:
            showCurrencyMenu();
            break;
        case 8:
            showBalanceByCurrency();
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

/**
 * @brief Меню работы с транзакциями
 * @details Позволяет:
 * - Просматривать все транзакции
 * - Фильтровать по доходам/расходам
 *
 * @note Использует getMenuChoice() для обработки ввода
 */
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

/**
 * @brief Меню финансовой статистики
 * @details Доступные отчеты:
 * - Общий баланс
 * - Анализ по категориям
 * - Помесячная статистика
 * - Статистика по текущему счету
 */
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

/**
 * @brief Создает новый счет
 * @throws std::invalid_argument Если имя счета уже существует
 * @post Добавляет новый счет в accounts
 *
 * @par Пример:
 * @code
 * Введите название счета: Сбережения
 * Счет создан!
 * @endcode
 */

 /**
  * @details Для переименования:
  * 1. Создается новый счет с перенесенными данными
  * 2. Старый счет удаляется (кроме "Общего")
  * 3. Обновляется currentAccount
  */
void FinanceCore::createAccount() {
    std::string name;
    std::cout << "Введите название счета: ";
    std::cin.ignore();
    std::getline(std::cin, name);

    //std::lock_guard<std::mutex> lock(accounts_mutex_); // Добавляем блокировку

    if (accounts.find(name) != accounts.end()) {
        std::cout << "Счет с таким именем уже существует!\n";
        return;
    }

    accounts.emplace(std::piecewise_construct,
        std::forward_as_tuple(name),
        std::forward_as_tuple(name));
    std::cout << "Счет создан!\n";
}

/**
 * @brief Выбирает активный счет
 * @details Отображает список всех счетов с балансами
 * @post Устанавливает currentAccount на выбранный
 *
 * @warning При отмене currentAccount не изменяется
 */
void FinanceCore::selectAccount() {
   // std::lock_guard<std::mutex> lock(accounts_mutex_); // Блокировка

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
    currentAccount = &accounts.at(it->first);
    std::cout << "Выбран счет: " << it->first << "\n";
}

/**
 * @brief Удаляет указанный счет
 * @pre В системе должен остаться хотя бы один счет
 * @post Если удалялся текущий счет, переключается на "Общий"
 *
 * @throws std::logic_error При попытке удалить последний счет
 */
void FinanceCore::deleteAccount() {
   // std::lock_guard<std::mutex> lock(accounts_mutex_); // Блокировка

    if (accounts.size() <= 1) {
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
        currentAccount = &accounts.at("Общий");
    }

    accounts.erase(it);
    std::cout << "Счет удален.\n";
}


/**
 * @brief Переименовывает текущий счет
 * @param[in] newName Новое имя счета
 * @throws std::invalid_argument Если имя уже занято
 *
 * @note Для "Общего" счета создает копию, а не переименовывает
 */
void FinanceCore::renameAccount() {
    //std::lock_guard<std::mutex> lock(accounts_mutex_); // Блокировка

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

    // Создаем новый счет с перемещением транзакций
    accounts.emplace(std::piecewise_construct,
        std::forward_as_tuple(newName),
        std::forward_as_tuple(newName));
    accounts.at(newName).move_transactions_from(std::move(*currentAccount));

    // Удаляем старую запись (если это не "Общий" счет)
    if (currentAccount->get_name() != "Общий") {
        accounts.erase(currentAccount->get_name());
    }

    currentAccount = &accounts.at(newName);
    std::cout << "Счет переименован.\n";
}

/**
 * @brief Управление счетами (главное меню)
 * @details Объединяет:
 * - createAccount()
 * - deleteAccount()
 * - selectAccount()
 *
 * @see createAccount()
 * @see deleteAccount()
 * @see selectAccount()
 */
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

/**
 * @brief Отображает главное меню
 * @details Формат вывода:
 * - Заголовок приложения
 * - Инфо о текущем счете
 * - Доступные действия
 *
 * @note Кроссплатформенная реализация очистки консоли
 */
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

    double total_balance = 0;
    double current_account_balance = 0;
    for (const auto& [name, account] : accounts) {
        total_balance += account.get_balance_in_currency(currency_converter_, base_currency_);
    }
    if (currentAccount) {
        current_account_balance = currentAccount->get_balance_in_currency(currency_converter_, base_currency_);
    }


    std::cout << "+-------------------------------+\n";
    std::cout << "|      ФИНАНСОВЫЙ МЕНЕДЖЕР     |\n";
    std::cout << "+-------------------------------+\n";
    std::cout << "| Основная валюта: " << std::left << std::setw(11) << base_currency_ << " |\n";
    std::cout << "| Общий баланс: " << std::setw(14) << std::fixed << std::setprecision(2) << total_balance << " |\n";
    std::cout << "| Текущий счёт: " << std::setw(14) << currentAccount->get_name() << " |\n";
    std::cout << "| Баланс счета:    " << std::setw(11) << current_account_balance << " |\n";
    std::cout << "+-------------------------------+\n";
    std::cout << "| 1. Добавить транзакцию        |\n";
    std::cout << "| 2. Просмотреть историю        |\n";
    std::cout << "| 3. Статистика                 |\n";
    std::cout << "| 4. Управление счетами         |\n";
    std::cout << "| 5. Удалить транзакцию         |\n";
    std::cout << "| 6. Обновить курсы валют       |\n";
    std::cout << "| 7. Изменить основную валюту   |\n";
    std::cout << "| 8. Баланс по валютам          |\n";
    std::cout << "| 0. Выход                      |\n";
    std::cout << "+-------------------------------+\n";
    std::cout << "> Выберите действие: ";
}

void FinanceCore::showCurrencyMenu() {
    std::cout << "\nДоступные валюты:\n";
    int i = 1;
    std::vector<std::string> currencies;

    // Получаем список поддерживаемых валют
    currencies.push_back("RUB");
    currencies.push_back("USD");
    currencies.push_back("EUR");
    // Можно добавить другие валюты

    for (const auto& curr : currencies) {
        std::cout << i++ << ". " << curr << "\n";
    }

    std::cout << "Выберите валюту (0 - отмена): ";
    int choice;
    std::cin >> choice;

    if (choice > 0 && choice <= currencies.size()) {
        setBaseCurrency(currencies[choice - 1]);
        std::cout << "Основная валюта изменена на " << currencies[choice - 1] << "\n";
    }
}