/**
 * @file Time_Manager.hpp
 * @brief Класс для работы с финансовыми транзакциями
 * @author Сонин Михаил/Эксузян Давид
 * @version 1.0
 * @date 2025-05-16
 */

#pragma once
#include <string>
#include <iostream>
#include <stdexcept>
#include "Date.hpp"
#include "currency/CurrencyConverter.hpp"

 /**
  * @class Transaction
  * @brief Класс финансовой транзакции (доход/расход)
  *
  * @details Обеспечивает:
  * - Хранение информации о транзакции
  * - Валидацию данных
  * - Сериализацию/десериализацию
  * - Форматированный вывод
  *
  * @note Автоматически генерирует уникальные ID для транзакций
  */
class Transaction {
    friend class FinanceCore;
public:
    /**
     * @enum Type
     * @brief Тип транзакции
     */
    enum class Type { 
        INCOME, ///< Доходная операция
        EXPENSE ///< Расходная операция
    };

    /**
    * @brief Конструктор по умолчанию
    * @details Инициализирует:
    * - ID (автоинкремент)
    * - Текущую дату
    * - Пустую категорию
    * - Тип EXPENSE
    */
    Transaction()
        : id(next_id++),
        amount(0.0),
        category("Uncategorized"),
        type(Type::EXPENSE),
        date(),  // Текущая дата
        description("")
    {
    }

    Transaction(const Transaction& other)
        : id(other.id), amount(other.amount), category(other.category),
        type(other.type), date(other.date), description(other.description),
        currency_(other.currency_), tags_(other.tags_) {
    }

    /**
    * @brief Основной конструктор
    * @param amt Сумма (должна быть > 0)
    * @param cat Категория (не пустая)
    * @param t Тип транзакции
    * @param d Дата (должна быть валидной)
    * @param desc Описание (опционально)
    * @throws std::invalid_argument При невалидных данных
    */
    Transaction(double amt, const std::string& cat, Type t, const Date& d, const std::string& desc = "")
        : id(next_id++), amount(amt), category(cat), type(t), date(d), description(desc) {
        validation();
    }

    //Геттеры
    int get_id() const { return id; }                   ///< @return Уникальный ID транзакции
    double get_amount() const { return amount; }        ///< @return Сумма транзакции
    std::string get_category() const { return category; } ///< @return Категория
    Type get_type() const { return type; }             ///< @return Тип (INCOME/EXPENSE)
    Date get_date() const { return date; }             ///< @return Дата транзакции
    std::string get_description() const { return description; } ///< @return Описание

    /**
    * @brief Устанавливает сумму транзакции
    * @param amt Новая сумма (> 0)
    * @throws std::invalid_argument Если сумма <= 0
    */
    void set_amount(double amt) {
        if (amt <= 0) throw std::invalid_argument("Amount must be positive");
        amount = amt;
    }

    /**
     * @brief Устанавливает категорию
     * @param cat Новая категория (не пустая)
     * @throws std::invalid_argument Если категория пуста
     */
    void set_category(const std::string& cat) {
        if (cat.empty()) throw std::invalid_argument("Category cannot be empty");
        category = cat;
    }

    /**
     * @brief Устанавливает дату
     * @param d Новая дата (должна быть валидной)
     * @throws std::invalid_argument Если дата невалидна
     */
    void set_date(const Date& d) {
        if (!d.is_valid()) throw std::invalid_argument("Invalid date");
        date = d;
    }


    void set_type(Type t) { type = t; }                ///< Устанавливает тип транзакции
    void set_description(const std::string& desc) {
        description = desc.empty() ? "--" : desc;
    } ///< Устанавливает описание
    void set_id(int new_id) { id = new_id; }           ///< Устанавливает ID (для загрузки из файла)

    /**
     * @brief Возвращает сумму с учетом типа
     * @return Для INCOME - положительная сумма, для EXPENSE - отрицательная
     */
    double get_signed_amount() const {
        return (type == Type::INCOME) ? amount : -amount;
    }

    /**
     * @brief Форматированное строковое представление
     * @return Строка вида "YYYY-MM-DD [+/-] сумма (категория) описание"
     */
    std::string get_summary() const {
        return date.to_string() + " " +
            (type == Type::INCOME ? "[+] " : "[-] ") +
            std::to_string(amount) + " (" + category + ") " + description;
    }

    /**
     * @brief Оператор вывода в поток
     * @param os Выходной поток
     * @param t Транзакция для вывода
     * @return Поток os
     *
     * @note Формат: "id,amount,type,category,yyyy mm dd,description"
     */
    friend std::ostream& operator<<(std::ostream& os, const Transaction& t) {
        os << t.id << ","
            << t.amount << ","
            << static_cast<int>(t.type) << ","
            << t.category << ","
            << t.date << ","
            << t.currency_ << ","  // Добавлено сохранение валюты
            << t.description;
        return os;
    }

    /**
    * @brief Оператор ввода из потока
    * @param is Входной поток
    * @param t Транзакция для заполнения
    * @return Поток is
    * @throws std::runtime_error При ошибках формата
    * @throws std::ios_base::failure При невалидных данных
    */
    friend std::istream& operator>>(std::istream& is, Transaction& t) {
        std::string line;
        if (!std::getline(is, line)) return is;

        std::istringstream iss(line);
        char delim;
        int type;

        try {
            if (!(iss >> t.id >> delim >> t.amount >> delim >> type >> delim)) {
                throw std::runtime_error("Ошибка чтения формата транзакции");
            }
            t.type = static_cast<Transaction::Type>(type);

            // Чтение категории (до запятой)
            std::getline(iss, t.category, ',');

            // Чтение даты
            int y, m, d;
            if (!(iss >> y >> m >> d >> delim)) {
                throw std::runtime_error("Ошибка чтения даты");
            }
            t.date = Date(y, m, d);
            
            // Остаток строки - описание
            std::getline(iss, t.currency_, ',');
            std::getline(iss, t.description);
        }
        catch (const std::exception& e) {
            std::cerr << "Ошибка чтения транзакции: " << e.what() << "\n";
            is.setstate(std::ios::failbit);
        }

        return is;
    }

    void set_currency(const std::string& currency) {
        currency_ = currency;
    }

    double get_amount_in_rub(const CurrencyConverter& converter) const {
        return converter.convert(amount, currency_, "RUB");
    }


    // Геттеры
    const std::string& get_currency() const { return currency_; }

    void Transaction::add_tag(const std::string& tag) {
        if (tags_.size() >= MAX_TAGS) {
            throw std::runtime_error("Достигнут лимит тегов (" + std::to_string(MAX_TAGS) + ")");
        }
        if (std::find(tags_.begin(), tags_.end(), tag) != tags_.end()) {
            throw std::runtime_error("Тег '" + tag + "' уже добавлен");
        }
        tags_.push_back(tag);
    }

    const std::vector<std::string>& get_tags() const { return tags_; }
    static const std::vector<std::string>& get_available_tags(); // Предопределенные теги

    void remove_tag(size_t index) {
        if (index < tags_.size()) {
            tags_.erase(tags_.begin() + index);
        }
    }


private:

    std::vector<std::string> tags_; // Новое поле
    static constexpr size_t MAX_TAGS = 5; // Макс. количество тегов

    static inline int next_id = 1; ///< Счетчик для автоинкремента ID
    int id;             ///< Уникальный идентификатор
    double amount;      ///< Сумма (> 0)
    std::string category; ///< Категория (не пустая)
    Type type;          ///< Тип (доход/расход)
    Date date;          ///< Дата транзакции
    std::string description; ///< Описание (опционально)
    std::string currency_ = "RUB";

    /**
     * @brief Валидация данных транзакции
     * @throws std::invalid_argument Если:
     * - amount <= 0
     * - category пуста
     * - date невалидна
     */
    void validation() const {
        if (amount <= 0) throw std::invalid_argument("Amount can't be negative");
        if (category.empty()) throw std::invalid_argument("You should choose category");
        if (!date.is_valid()) throw std::invalid_argument("Invalid date format");
    }
};


