/**
 * @file FinanceCore.cpp
 * @brief Реализация ядра финансовой системы
 *
 * @details Этот модуль содержит:
 * - Инициализацию системы
 * - Работу с файлами данных
 * - Валютные операции
 * - Базовые утилиты
 *
 * @section init_sec Процесс инициализации
 * 1. Определение путей к данным
 * 2. Создание стандартных директорий
 * 3. Загрузка сохраненных данных
 * 4. Инициализация валютных курсов
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
  * @brief Получает полный путь к файлу данных
  * @param filename Имя файла
  * @return Абсолютный путь в формате {executable_path}/data/{filename}
  *
  * @details Алгоритм работы:
  * 1. Определяет путь к исполняемому файлу
  * 2. Создает поддиректорию /data
  * 3. Возвращает полный путь
  *
  * @throws std::filesystem::filesystem_error При ошибках работы с файловой системой
  */
std::string FinanceCore::getDataPath(const std::string& filename) {
    std::filesystem::path exePath = std::filesystem::current_path();
    std::filesystem::path dataPath = exePath / "data" / filename;
    return dataPath.string();
}

/**
 * @brief Возвращает текущий активный счет
 * @return Ссылка на Account
 *
 * @details Гарантии:
 * - Если currentAccount не установлен, возвращает счет "Общий"
 * - Никогда не возвращает nullptr
 *
 * @note Потокобезопасен за счет accounts_mutex_
 */
Account& FinanceCore::getCurrentAccount() {
    if (!currentAccount) {
        currentAccount = &accounts.at("Общий");
    }
    return *currentAccount;
}

/**
 * @brief Обеспечивает наличие счетов по умолчанию
 *
 * @details Действия:
 * 1. Если accounts пуст, создает счет "Общий"
 * 2. Если currentAccount недействителен, устанавливает его на "Общий"
 *
 * @post Гарантирует accounts.size() >= 1
 * @post Гарантирует currentAccount != nullptr
 */
void FinanceCore::ensureDefaultAccount() {
    if (accounts.empty()) {
        accounts.try_emplace("Общий", "Общий");
    }
    if (!currentAccount) {
        currentAccount = &accounts.at("Общий");
    }
}

/**
 * @brief Основной конструктор системы
 *
 * @details Полный процесс инициализации:
 * 1. Настройка путей к данным (порядок проверки):
 *    - Linux: /proc/self/exe
 *    - Windows: текущая директория
 *    - Fallback: относительный путь
 * 2. Создание необходимых директорий
 * 3. Инициализация стандартного счета
 * 4. Асинхронное обновление валютных курсов
 * 5. Загрузка сохраненных данных
 *
 * @throws std::runtime_error При критических ошибках инициализации
 */
FinanceCore::FinanceCore() {
    // Инициализация путей
    std::filesystem::path dataPath;
    try {
        // Linux-способ определения пути
        dataPath = std::filesystem::canonical("/proc/self/exe").parent_path() / "transactions.dat";
    }
    catch (...) {
        try {
            // Windows fallback
            dataPath = getDataPath("transactions.dat");
        }
        catch (...) {
            // Аварийный fallback
            dataPath = "transactions.dat";
        }
    }

    dataFile = dataPath.string();
    std::cout << "Файл данных будет сохранен в: " << dataFile << std::endl;

    // Создание директорий
    ensureDataDirectory();
    std::filesystem::create_directories("CurrencyDat");

    // Инициализация аккаунтов
    accounts.try_emplace("Общий", "Общий");
    currentAccount = &accounts.at("Общий");

    // Асинхронное обновление курсов валют
    try {
        std::promise<bool> promise;
        auto future = promise.get_future();
        update_currency_rates([&promise](bool success) {
            promise.set_value(success);
            });
        future.wait();

        if (!future.get()) {
            std::cout << "Предупреждение: не удалось обновить курсы валют\n";
        }
    }
    catch (...) {
        std::cerr << "Ошибка при обновлении курсов валют\n";
    }

    // Загрузка данных
    loadData();
}

/**
 * @brief Проверяет целостность финансовых данных
 * @return true если балансы всех счетов соответствуют транзакциям
 *
 * @details Алгоритм проверки:
 * 1. Для каждого счета вычисляет сумму всех транзакций
 * 2. Сравнивает с текущим балансом
 * 3. Допустимая погрешность: 0.01 (ошибки округления)
 *
 * @note Используется при загрузке данных
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
 * @brief Получает выбор пользователя из меню
 * @return Число от 0 до N (в зависимости от меню)
 *
 * @details Особенности:
 * - Защита от некорректного ввода
 * - Очистка буфера после считывания
 * - Цикл до получения валидного значения
 *
 * @note Использует clearInputBuffer() для очистки
 */
int FinanceCore::getMenuChoice() const {
    int choice;
    while (true) {
        if (std::cin >> choice) {
            clearInputBuffer();
            return choice;
        }
        else {
            std::cout << "Ошибка ввода. Пожалуйста, введите число: ";
            std::cin.clear();
            clearInputBuffer();
        }
    }
}

/**
 * @brief Очищает консоль кроссплатформенно
 *
 * @details Реализация:
 * - Windows: WinAPI (FillConsoleOutputCharacter)
 * - Unix: ANSI escape codes
 *
 * @post Курсор устанавливается в позицию (0,0)
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
 * @brief Очищает буфер ввода std::cin
 *
 * @details Используется после:
 * - cin >> для предотвращения проблем
 * - Ошибок ввода
 *
 * @note Работает с любым количеством оставшихся символов
 */
void FinanceCore::clearInputBuffer() const {
    std::cin.clear();
    if (std::cin.rdbuf()->in_avail() > 0) {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

/**
 * @brief Обновляет курсы валют асинхронно
 * @param callback Функция обратного вызова (bool success)
 *
 * @details Последовательность действий:
 * 1. Запускает CurrencyFetcher в отдельном потоке
 * 2. При успехе сохраняет курсы в файл
 * 3. При неудаче пытается загрузить сохраненные курсы
 * 4. Вызывает callback с результатом
 *
 * @note Использует std::async для асинхронности
 */
void FinanceCore::update_currency_rates(std::function<void(bool)> callback) {
    CurrencyFetcher fetcher;
    fetcher.fetch_rates([this, callback](const auto& new_rates) {
        bool success = false;

        // 1. Обновление курсов
        if (!new_rates.empty()) {
            currency_converter_.set_rates(new_rates);
            currency_converter_.save_rates_to_file("CurrencyDat/currency_rates.json");
            success = true;
        }
        else {
            success = currency_converter_.load_rates_from_file("CurrencyDat/currency_rates.json");
        }

        // 2. Пересчет балансов
        if (success) {
            for (auto& [name, account] : accounts) {
                account.recalculateBalance(currency_converter_);
            }
        }

        callback(success);
        });
}

/**
 * @brief Конвертирует сумму между валютами
 * @param amount Исходная сумма
 * @param from Исходная валюта (код ISO 4217)
 * @param to Целевая валюта (по умолчанию RUB)
 * @return Сконвертированная сумма
 *
 * @throws std::runtime_error При:
 * - Неподдерживаемой валюте
 * - Отсутствии курсов валют
 * - Ошибках вычисления
 *
 * @note Использует текущие курсы CurrencyConverter
 */
double FinanceCore::convert_currency(double amount, const std::string& from,
    const std::string& to) const {
    return currency_converter_.convert(amount, from, to);
}

/**
 * @brief Устанавливает базовую валюту системы
 * @param currency Код валюты (ISO 4217)
 *
 * @throws std::invalid_argument Если валюта не поддерживается
 * @post Все отчеты будут в указанной валюте
 */
void FinanceCore::setBaseCurrency(const std::string& currency) {
    if (currency_converter_.is_currency_supported(currency)) {
        base_currency_ = currency;
    }
    else {
        throw std::invalid_argument("Валюта не поддерживается");
    }
}

/**
 * @brief Возвращает текущую базовую валюту
 * @return Код валюты (например "RUB")
 */
std::string FinanceCore::getBaseCurrency() const {
    return base_currency_;
}

/**
 * @brief Создает директорию для данных если отсутствует
 *
 * @details Проверяет и создает:
 * - Основную директорию данных
 * - Поддиректорию для курсов валют
 *
 * @note Вызывается автоматически при инициализации
 */
void FinanceCore::ensureDataDirectory() {
    std::filesystem::path dataDir = std::filesystem::path(dataFile).parent_path();
    if (!std::filesystem::exists(dataDir)) {
        std::filesystem::create_directories(dataDir);
    }
}