/**
 * @file Menu_Handlers.cpp
 * @brief Реализация пользовательских меню и интерактивных диалогов
 *
 * @details Этот модуль содержит всю логику взаимодействия с пользователем:
 * - Главное меню системы
 * - Меню управления транзакциями
 * - Меню статистики и отчетов
 * - Меню управления счетами
 * - Вспомогательные диалоги
 *
 * @section ui_principles Принципы UI
 * 1. Консистентность: единый стиль всех меню
 * 2. Интерактивность: немедленная обратная связь
 * 3. Безопасность: валидация ввода
 * 4. Доступность: поддержка горячих клавиш
 */

#include "FinanceCore.hpp"
#ifdef _WIN32
#include <windows.h> // Для очистки консоли
#endif

 /**
  * @brief Главный цикл меню приложения
  *
  * @details Реализует основной workflow системы:
  * 1. Отображение главного меню
  * 2. Обработка выбора пользователя
  * 3. Вызов соответствующих функций
  * 4. Обновление данных
  *
  * @par Структура меню:
  * @code{.unparsed}
  * +-------------------------------+
  * |      ФИНАНСОВЫЙ МЕНЕДЖЕР     |
  * | Основная валюта: RUB          |
  * | Общий баланс: 1500.00         |
  * | Текущий счёт: Основной        |
  * | Баланс счета: 1000.00         |
  * +-------------------------------+
  * | 1. Добавить транзакцию        |
  * | 2. Просмотреть историю        |
  * | 3. Статистика                 |
  * | ...                           |
  * | 0. Выход                      |
  * +-------------------------------+
  * @endcode
  *
  * @note Автоматически сохраняет данные при выходе
  */

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
        case 9:
            runSearchMenu();
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
 * @brief Меню работы с историей транзакций
 *
 * @details Предоставляет:
 * - Полный список транзакций
 * - Фильтрацию по типам (доходы/расходы)
 * - Поиск по критериям
 * - Экспорт данных
 *
 * @par Пример использования:
 * @code
 * === История транзакций ===
 * 1. Все транзакции
 * 2. Только доходы
 * 3. Только расходы
 * 4. Назад
 * Выберите действие: 2
 * @endcode
 *
 * @see FinanceCore::viewAllTransactions()
 * @see FinanceCore::viewIncome()
 * @see FinanceCore::viewExpenses()
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
 * @brief Меню финансовой аналитики
 *
 * @details Включает все виды отчетов:
 * - Общий баланс
 * - Анализ по категориям
 * - Динамика по месяцам
 * - Статистика по счету
 *
 * @note Все отчеты используют текущую базовую валюту
 *
 * @warning Для некоторых отчетов требуется минимум 3 месяца данных
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
   // std::lock_guard<std::mutex> lock(accounts_mutex_);

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
   // std::lock_guard<std::mutex> lock(accounts_mutex_);

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
    //std::lock_guard<std::mutex> lock(accounts_mutex_);

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
 * @brief Меню управления счетами
 *
 * @details Позволяет:
 * - Создавать новые счета
 * - Удалять существующие
 * - Переименовывать счета
 * - Выбирать активный счет
 *
 * @throws std::invalid_argument При попытке удалить последний счет
 *
 * @par Безопасность:
 * - Запрещает удаление всех счетов
 * - Сохраняет "Общий" счет как резервный
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
 * @brief Отображает главное меню системы
 *
 * @details Формирует:
 * - Шапку с текущим состоянием системы
 * - Список доступных действий
 * - Интерактивный элемент ввода
 *
 * @post Очищает консоль перед отображением
 *
 * @note Использует кроссплатформенные методы:
 * - Windows API на Win32
 * - ANSI-коды на Unix-системах
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
    std::cout << "| 9. Поиск по тегам             |\n";
    std::cout << "| 0. Выход                      |\n";
    std::cout << "+-------------------------------+\n";
    std::cout << "> Выберите действие: ";
}

/**
 * @brief Меню выбора валюты
 *
 * @details Отображает:
 * - Список доступных валют
 * - Текущую базовую валюту
 * - Интерактивный выбор
 *
 * @note Поддерживаемые валюты:
 * - RUB (Рубли)
 * - USD (Доллары)
 * - EUR (Евро)
 *
 * @see CurrencyConverter::is_currency_supported()
 */
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

/**
 * @brief Меню поиска по тегам
 *
 * @details Реализует:
 * - Выбор нескольких тегов
 * - Фильтрацию транзакций
 * - Просмотр результатов
 * - Управление выбором тегов
 *
 * @note Максимальное количество тегов для одновременного поиска: 5
 *
 * @par Пример работы:
 * @code
 * === Поиск по тегам ===
 * Выбранные теги: [еда] [супермаркет]
 * Доступные теги:
 * 1. транспорт
 * 2. развлечения
 * ...
 * 8. Начать поиск
 * 9. Очистить выбор
 * 0. Назад
 * @endcode
 */
void FinanceCore::runSearchMenu() {
    const auto& available_tags = Transaction::get_available_tags();
    std::vector<std::string> selected_tags;

    while (true) {
        clearConsole();
        std::cout << "\n=== Поиск по тегам ===";
        std::cout << "\nВыбранные теги: ";
        for (const auto& tag : selected_tags) std::cout << "[" << tag << "] ";

        std::cout << "\n\nДоступные теги:\n";
        for (size_t i = 0; i < available_tags.size(); ++i) {
            std::cout << i + 1 << ". " << available_tags[i] << "\n";
        }

        std::cout << "\n" << available_tags.size() + 1 << ". Начать поиск\n";
        std::cout << available_tags.size() + 2 << ". Очистить выбор\n";
        std::cout << "0. Назад\nВыберите действие: ";

        int choice = getMenuChoice();

        if (choice == 0) {
            break;
        }
        else if (choice == available_tags.size() + 1) { // Поиск
            if (selected_tags.empty()) {
                std::cout << "Не выбрано ни одного тега!\n";
                std::cout << "Нажмите Enter для продолжения...";
                std::cin.ignore();
                std::cin.get();
            }
            else {
                searchByTags(selected_tags);
                // После поиска остаемся в меню выбора тегов
            }
        }
        else if (choice == available_tags.size() + 2) { // Очистка
            selected_tags.clear();
        }
        else if (choice > 0 && choice <= available_tags.size()) {
            const std::string& selected_tag = available_tags[choice - 1];
            if (std::find(selected_tags.begin(), selected_tags.end(), selected_tag) == selected_tags.end()) {
                if (selected_tags.size() < Transaction::MAX_TAGS) {
                    selected_tags.push_back(selected_tag);
                }
                else {
                    std::cout << "Достигнут лимит выбранных тегов (" << Transaction::MAX_TAGS << ")\n";
                    std::cout << "Нажмите Enter для продолжения...";
                    std::cin.ignore();
                    std::cin.get();
                }
            }
        }
    }
}