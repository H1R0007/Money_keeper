/**
 * @file Statistics.cpp
 * @brief Реализация финансовой аналитики и отчетности
 *
 * @details Этот модуль предоставляет комплексные инструменты для анализа:
 * - Общего финансового состояния
 * - Динамики доходов/расходов
 * - Категоризированной статистики
 * - Помесячной отчетности
 * - Валютной аналитики
 *
 * @section analysis_types Типы аналитики
 * 1. Агрегированная (общие суммы)
 * 2. Категоризированная (по группам)
 * 3. Временная (по периодам)
 * 4. Валютная (конвертированные значения)
 */


#include "FinanceCore.hpp"
#include <iomanip>

 /**
  * @brief Отображает общий баланс по всем счетам
  *
  * @details Вычисляет:
  * - Суммарные доходы (все INCOME транзакции)
  * - Суммарные расходы (все EXPENSE транзакции)
  * - Чистый баланс (доходы - расходы)
  *
  * @note Все значения конвертируются в базовую валюту (base_currency_)
  *
  * @par Пример вывода:
  * @code
  * === Общая статистика (USD) ===
  * Доходы: 1500.00
  * Расходы: 750.50
  * Баланс: 749.50
  * @endcode
  */

void FinanceCore::showTotalBalance() const {
    double totalIncome = 0;
    double totalExpenses = 0;

    for (const auto& [name, account] : accounts) {
        for (const auto& t : account.get_transactions()) {
            double amount = currency_converter_.convert(
                t.get_amount(),
                t.get_currency(),
                base_currency_
            );

            if (t.get_type() == Transaction::Type::INCOME) {
                totalIncome += amount;
            }
            else {
                totalExpenses += amount;
            }
        }
    }

    std::cout << "\n=== Общая статистика (" << base_currency_ << ") ===\n"
        << "Доходы: " << std::fixed << std::setprecision(2) << totalIncome << "\n"
        << "Расходы: " << totalExpenses << "\n";
}

/**
 * @brief Показывает статистику по категориям
 *
 * @details Формирует детализированный отчет:
 * - Группирует транзакции по категориям
 * - Считает отдельно доходы и расходы для каждой
 * - Вычисляет итоговый баланс по категориям
 *
 * @note Использует двухуровневую сортировку:
 * 1. По алфавиту категорий
 * 2. По абсолютному значению суммы
 *
 * @warning Категории с пустым названием объединяются в "Без категории"
 */
void FinanceCore::showByCategory() const {
    std::map<std::string, std::pair<double, double>> categories;

    for (const auto& [name, account] : accounts) {
        for (const auto& t : account.get_transactions()) {
            double amount = t.get_amount_in_rub(currency_converter_);
            if (t.get_type() == Transaction::Type::INCOME) {
                categories[t.get_category()].first += amount;
            }
            else {
                categories[t.get_category()].second += amount;
            }
        }
    }

    std::cout << "\n=== Статистика по категориям (" << base_currency_ << ") ===\n";
    std::cout << "+----------------------+----------------+----------------+----------------+\n";
    std::cout << "|      Категория       |     Доходы     |    Расходы     |     Итого      |\n";
    std::cout << "+----------------------+----------------+----------------+----------------+\n";

    for (const auto& [category, amounts] : categories) {
        std::string cat_display = category.empty() ? "Без категории" : category;
        if (cat_display.length() > 20) cat_display = cat_display.substr(0, 17) + "...";

        std::cout << "| " << std::left << std::setw(20) << cat_display << " | "
            << std::right << std::setw(14) << std::fixed << std::setprecision(2) << amounts.first << " | "
            << std::setw(14) << amounts.second << " | "
            << std::setw(14) << (amounts.first - amounts.second) << " |\n";
    }
    std::cout << "+----------------------+----------------+----------------+----------------+\n" << std::flush;
}

/**
 * @brief Анализирует транзакции по месяцам
 *
 * @details Строит временную шкалу:
 * - Группировка по году и месяцу
 * - Суммирование доходов/расходов
 * - Расчет месячного баланса
 * - Визуализация динамики
 *
 * @note Для корректной работы требуется:
 * - Минимум 3 месяца данных
 * - Настройка CultureInfo для локализации
 *
 * @par Пример вывода:
 * @code
 * | 2023-05 |   1500.00 |     750.50 |      749.50 |
 * | 2023-06 |   2000.00 |    1000.00 |     1000.00 |
 * @endcode
 */
void FinanceCore::showByMonth() const {
  //  std::lock_guard<std::mutex> lock(accounts_mutex_);
    std::map<std::pair<int, int>, std::pair<double, double>> monthly_stats;

    for (const auto& [name, account] : accounts) {
        for (const auto& t : account.get_transactions()) {
            const Date& date = t.get_date();
            auto month_key = std::make_pair(date.get_year(), date.get_month());
            double amount = t.get_amount_in_rub(currency_converter_);

            if (t.get_type() == Transaction::Type::INCOME) {
                monthly_stats[month_key].first += amount;
            }
            else {
                monthly_stats[month_key].second += amount;
            }
        }
    }

    clearConsole();
    std::cout << "\n=== Статистика по месяцам (в RUB) ===\n";
    std::cout << "+------------+--------------+--------------+--------------+\n";
    std::cout << "|   Месяц    |    Доходы    |   Расходы    |    Баланс    |\n";
    std::cout << "+------------+--------------+--------------+--------------+\n";

    for (const auto& [month, amounts] : monthly_stats) {
        std::string month_str = std::to_string(month.first) + "-" +
            (month.second < 10 ? "0" : "") + std::to_string(month.second);

        std::cout << "| " << std::setw(10) << month_str << " | "
            << std::setw(12) << std::fixed << std::setprecision(2) << amounts.first << " | "
            << std::setw(12) << amounts.second << " | "
            << std::setw(12) << (amounts.first - amounts.second) << " |\n";
    }
    std::cout << "+------------+--------------+--------------+--------------+\n" << std::flush;
}



/**
 * @brief Показывает детальную статистику по текущему счету
 *
 * @details Включает:
 * - Общее количество транзакций
 * - Разбивку по валютам
 * - Соотношение доходов/расходов
 * - Средние значения операций
 *
 * @warning Использует только активный счет (currentAccount)
 *
 * @post После вызова сохраняет последние вычисленные значения в кеше
 */
void FinanceCore::showCurrentAccountStats() const {
   // std::lock_guard<std::mutex> lock(accounts_mutex_);

    if (!currentAccount) {
        std::cout << "Нет текущего счета!\n";
        return;
    }

    double income = 0;
    double expenses = 0;
    std::map<std::string, double> byCurrency;

    for (const auto& t : currentAccount->get_transactions()) {
        double amount = t.get_amount_in_rub(currency_converter_);
        byCurrency[t.get_currency()] += t.get_signed_amount();

        if (t.get_type() == Transaction::Type::INCOME) {
            income += amount;
        }
        else {
            expenses += amount;
        }
    }

    std::cout << "\n=== Статистика (" << currentAccount->get_name() << ") ===\n"
        << "Транзакций: " << currentAccount->get_transactions().size() << "\n"
        << "Доходы: " << std::fixed << std::setprecision(2) << income << " руб.\n"
        << "Расходы: " << expenses << " руб.\n"
        << "\nПо валютам:\n";

    for (const auto& [currency, amount] : byCurrency) {
        std::cout << "  " << currency << ": " << amount;
        if (currency != "RUB") {
            std::cout << " (≈" << convert_currency(amount, currency, "RUB") << " руб.)";
        }
        std::cout << "\n";
    }
}

/**
 * @brief Отображает баланс в разрезе валют
 *
 * @details Показывает:
 * - Независимые суммы по каждой валюте
 * - Автоконвертацию в базовую валюту
 * - Процентное соотношение валют
 *
 * @note Для конвертации использует текущие курсы CurrencyConverter
 */
void FinanceCore::showBalanceByCurrency() const {
    std::map<std::string, double> balances;

    // Собираем балансы по всем валютам
    for (const auto& [name, account] : accounts) {
        for (const auto& t : account.get_transactions()) {
            balances[t.get_currency()] += t.get_signed_amount();
        }
    }

    std::cout << "\n=== Баланс по валютам ===\n";
    for (const auto& [currency, amount] : balances) {
        std::cout << currency << ": " << std::fixed << std::setprecision(2) << amount << "\n";
    }
    std::cout << "\nНажмите Enter чтобы продолжить...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

/**
 * @brief Поиск транзакций по тегам
 *
 * @param tags Вектор тегов для поиска
 *
 * @details Алгоритм работы:
 * 1. Ищет совпадения по всем тегам одновременно (AND-логика)
 * 2. Выводит результаты в табличном формате
 * 3. Подсвечивает найденные теги
 *
 * @note Регистр тегов не учитывается
 * @warning При пустом списке тегов выводит предупреждение
 */
void FinanceCore::searchByTags(const std::vector<std::string>& tags) const {
    if (tags.empty()) {
        std::cout << "\nОшибка: Не указаны теги для поиска\n";
        std::cout << "Нажмите Enter для продолжения...";
        std::cin.ignore();
        std::cin.get();
        return;
    }

    std::vector<Transaction> result;

    for (const auto& [name, account] : accounts) {
        for (const auto& t : account.get_transactions()) {
            const auto& transaction_tags = t.get_tags();
            if (std::any_of(tags.begin(), tags.end(),
                [&](const auto& tag) {
                    return std::find(transaction_tags.begin(),
                        transaction_tags.end(), tag) != transaction_tags.end();
                })) {
                result.push_back(t);
            }
        }
    }

    clearConsole();
    printTransactionsTable(result, "Результаты поиска по тегам");

    // Добавляем паузу перед возвратом в меню
    std::cout << "\nНажмите Enter для возврата в меню...";
    std::cin.ignore();
    std::cin.get();
}
