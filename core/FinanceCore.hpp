/**
 * @file FinanceCore.hpp
 * @brief Основной класс системы управления финансами
 *
 * @details Ядро системы, обеспечивающее:
 * - Управление счетами и транзакциями
 * - Финансовую аналитику и отчетность
 * - Сохранение/загрузку данных
 * - Валютные операции
 * - Пользовательские меню
 *
 * @section architecture_sec Архитектура
 * 1. Централизованное хранение счетов (std::map)
 * 2. Потокобезопасные операции (std::mutex)
 * 3. Разделение интерфейса и реализации
 * 4. Поддержка dependency injection для CurrencyConverter
 */

#pragma once
#include "Date.hpp"
#include "Time_Manager.hpp"
#include <vector>
#include <map>
#include <algorithm>
#include "Account.hpp"
#include "currency/CurrencyConverter.hpp"
#include <filesystem>
#include <future>

 /**
  * @class FinanceCore
  * @brief Центральный класс финансового менеджера
  *
  * @invariant
  * 1. Всегда существует минимум один счет ("Общий")
  * 2. currentAccount никогда не nullptr
  * 3. Балансы всегда синхронизированы с транзакциями
  */
class FinanceCore {
private:
    std::string base_currency_ = "RUB";       ///< Базовая валюта для отчетности
    std::string dataFile;                     ///< Путь к файлу данных
    std::map<std::string, Account> accounts;  ///< Коллекция счетов
    Account* currentAccount;                  ///< Текущий активный счет
    CurrencyConverter currency_converter_;    ///< Конвертер валют
    mutable std::mutex accounts_mutex_;      ///< Мьютекс для потокобезопасности

public:
    // Конструкторы/операторы
    FinanceCore(const FinanceCore&) = delete;
    FinanceCore& operator=(const FinanceCore&) = delete;
    FinanceCore(FinanceCore&&) = default;
    FinanceCore& operator=(FinanceCore&&) = default;

    /**
     * @brief Основной конструктор системы
     * @details Инициализирует:
     * 1. Стандартный счет "Общий"
     * 2. Пути к данным
     * 3. Загружает сохраненные данные
     * 4. Обновляет курсы валют
     *
     * @throws std::runtime_error При ошибках инициализации
     */
    FinanceCore();

    /// @name Управление данными
    /// @{
    /**
     * @brief Загружает данные из файла
     * @throws std::ios_base::failure При ошибках чтения
     * @post Восстанавливает все счета и транзакции
     */
    void loadData();

    /**
     * @brief Сохраняет все данные в файл
     * @throws std::ios_base::failure При ошибках записи
     * @note Формат файла: CSV с секциями по счетам
     */
    void saveData();
    /// @}

    /// @name Управление счетами
    /// @{
    /**
     * @brief Создает новый счет
     * @throws std::invalid_argument При дубликате имени
     * @post Добавляет счет в коллекцию accounts
     */
    void createAccount();

    /**
     * @brief Удаляет указанный счет
     * @pre В системе должен остаться минимум один счет
     * @throws std::logic_error При попытке удалить последний счет
     */
    void deleteAccount();

    /**
     * @brief Выбирает активный счет
     * @details Отображает интерактивное меню выбора
     * @post Устанавливает currentAccount
     */
    void selectAccount();

    /**
     * @brief Переименовывает текущий счет
     * @note Для "Общего" счета создает копию
     * @throws std::invalid_argument При дубликате имени
     */
    void renameAccount();

    /**
     * @brief Главное меню управления счетами
     * @details Объединяет create/delete/select/rename
     */
    void manageAccounts();
    /// @}

    /// @name Работа с транзакциями
    /// @{
    /**
     * @brief Добавляет новую транзакцию
     * @details Интерактивный многошаговый диалог:
     * 1. Выбор типа
     * 2. Ввод суммы
     * 3. Указание категории
     * 4. Установка даты
     * 5. Добавление описания
     * 6. Управление тегами
     */
    void addTransaction();

    /**
     * @brief Удаляет транзакцию по ID
     * @param id Уникальный идентификатор
     * @return true если транзакция найдена и удалена
     */
    void removeTransaction();

    /**
     * @brief Показывает все транзакции текущего счета
     * @details Форматированный табличный вывод
     */
    void viewAllTransactions() const;

    /**
     * @brief Показывает только доходные транзакции
     */
    void viewIncome() const;

    /**
     * @brief Показывает только расходные транзакции
     */
    void viewExpenses() const;
    /// @}

    /// @name Аналитика и отчеты
    /// @{
    /**
     * @brief Общий баланс по всем счетам
     */
    void showTotalBalance() const;

    /**
     * @brief Статистика по категориям
     * @details Группирует транзакции по категориям
     */
    void showByCategory() const;

    /**
     * @brief Помесячная статистика
     */
    void showByMonth() const;

    /**
     * @brief Статистика по текущему счету
     */
    void showCurrentAccountStats() const;

    /**
     * @brief Баланс в разрезе валют
     */
    void showBalanceByCurrency() const;

    /**
     * @brief Поиск транзакций по тегам
     * @param tags Список тегов для поиска
     */
    void searchByTags(const std::vector<std::string>& tags) const;
    /// @}

    /// @name Валютные операции
    /// @{
    /**
     * @brief Устанавливает базовую валюту
     * @param currency Код валюты (ISO 4217)
     * @throws std::invalid_argument Для неподдерживаемых валют
     */
    void setBaseCurrency(const std::string& currency);

    /**
     * @brief Возвращает текущую базовую валюту
     * @return Код валюты (например "RUB")
     */
    std::string getBaseCurrency() const;

    /**
     * @brief Конвертирует сумму между валютами
     * @param amount Исходная сумма
     * @param from Исходная валюта
     * @param to Целевая валюта (по умолчанию RUB)
     * @return Сконвертированная сумма
     * @throws std::runtime_error При ошибках конвертации
     */
    double convert_currency(double amount, const std::string& from,
        const std::string& to = "RUB") const;

    /**
     * @brief Обновляет курсы валют асинхронно
     * @param callback Функция обратного вызова
     */
    void update_currency_rates(std::function<void(bool success)> callback);
    /// @}

    /// @name Вспомогательные методы
    /// @{
    /**
     * @brief Возвращает текущий активный счет
     * @return Ссылка на Account
     * @warning Не возвращает nullptr (гарантирует валидность)
     */
    Account& getCurrentAccount();

    /**
     * @brief Гарантирует наличие счета по умолчанию
     * @post Создает "Общий" счет если accounts пуст
     */
    void ensureDefaultAccount();

    /**
     * @brief Фильтрует транзакции по типу
     * @param type Тип транзакции (INCOME/EXPENSE)
     * @return Вектор транзакций
     */
    std::vector<Transaction> getFilteredTransactions(Transaction::Type type) const;

    /**
     * @brief Выводит таблицу транзакций
     * @param transactions Список транзакций
     * @param title Заголовок таблицы
     */
    void printTransactionsTable(const std::vector<Transaction>& transactions,
        const std::string& title) const;

    /**
     * @brief Получает выбор пользователя из меню
     * @return Числовой выбор (1-N)
     * @note Зацикливается до получения корректного ввода
     */
    int getMenuChoice() const;

    /**
     * @brief Очищает консоль (кроссплатформенно)
     */
    void clearConsole() const;

    /**
     * @brief Очищает буфер ввода
     */
    void clearInputBuffer() const;
    /// @}

    /// @name Основные интерфейсы
    /// @{
    /**
     * @brief Главное меню приложения
     */
    void runMainMenu();

    /**
     * @brief Меню работы с транзакциями
     */
    void runTransactionMenu();

    /**
     * @brief Меню статистики
     */
    void runStatsMenu();

    /**
     * @brief Меню поиска
     */
    void runSearchMenu();
    /// @}

    /**
     * @brief Проверяет целостность данных
     * @return true если балансы всех счетов корректны
     * @note Используется при загрузке данных
     */
    bool validateData() const;

    /**
     * @brief Создает директорию для данных если отсутствует
     */
    void ensureDataDirectory();

    /**
     * @brief Возвращает полный путь к файлу данных
     * @param filename Имя файла
     * @return Абсолютный путь
     */
    std::string getDataPath(const std::string& filename);

    /**
     * @brief Меню выбора валюты
     */
    void showCurrencyMenu();
};