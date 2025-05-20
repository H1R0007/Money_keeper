
/**
 * @file Statistics.cpp
 * @brief Реализация методов финансовой статистики
 */


#include "FinanceCore.hpp"
#include <iomanip>

 /**
  * @brief Показывает общий баланс по всем счетам
  * @details Вычисляет:
  * - Суммарные доходы
  * - Суммарные расходы
  * - Итоговый баланс
  *
  * @par Пример вывода:
  * @code
  * === Общая статистика ===
  * Доходы: 1500.00 руб.
  * Расходы: 750.50 руб.
  * Баланс: 749.50 руб.
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
 * @details Для каждой категории выводит:
 * - Сумму доходов
 * - Сумму расходов
 * - Итоговый баланс
 *
 * @note Использует std::map для группировки по категориям
 *
 * @par Формат вывода:
 * @code
 * Еда: +500.00 / -1200.50 (итого: -700.50)
 * Транспорт: +0.00 / -350.00 (итого: -350.00)
 * @endcode
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
 * @brief Показывает помесячную статистику
 * @details Формирует таблицу с разбивкой по:
 * - Году и месяцу (YYYY-MM)
 * - Доходам
 * - Расходам
 * - Балансу
 *
 * @post Очищает консоль перед выводом (clearConsole())
 *
 * @par Пример таблицы:
 * @code
 * +------------+------------+------------+------------+
 * |   Месяц    |   Доходы   |  Расходы   |  Баланс    |
 * +------------+------------+------------+------------+
 * | 2023-05    |    1500.00 |     750.50 |     749.50 |
 * +------------+------------+------------+------------+
 * @endcode
 */

 /**
  * @brief Анализирует транзакции по месяцам
  * @details Алгоритм работы:
  * 1. Группирует транзакции по ключу (год, месяц)
  * 2. Суммирует доходы/расходы для каждого периода
  * 3. Форматирует результат в таблицу
  *
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
 * @brief Показывает статистику по текущему счету
 * @details Включает:
 * - Количество транзакций
 * - Суммарные доходы
 * - Суммарные расходы
 * - Текущий баланс
 *
 * @warning Использует currentAccount (может быть nullptr)
 *
 * @par Пример вывода:
 * @code
 * === Статистика (Основной) ===
 * Транзакций: 5
 * Доходы: 2000.00
 * Расходы: 1500.00
 * Баланс: 500.00
 * @endcode
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
