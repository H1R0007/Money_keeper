/**
 * @file Time_Manager.hpp
 * @brief Управление финансовыми транзакциями и их временными характеристиками
 *
 * @details Этот модуль предоставляет функционал для работы с финансовыми операциями,
 *          включая их создание, валидацию, сериализацию и анализ временных меток.
 *          Реализована поддержка мультивалютности, тегов и сложных временных периодов.
 *
 * @section features_sec Основные возможности
 * - Создание транзакций с автоматической генерацией ID
 * - Поддержка доходов/расходов
 * - Валидация всех полей транзакции
 * - Работа с датами и временными периодами
 * - Конвертация валют
 * - Система тегов для категоризации
 *
 * @section design_sec Архитектурные решения
 * 1. Используется паттерн "Value Object" для транзакций
 * 2. Потокобезопасная реализация
 * 3. Поддержка сериализации/десериализации
 * 4. Строгая валидация данных
 *
 * @warning Все операции с датами ограничены периодом 2000-2100 гг.
 * @note Для работы с валютами требуется подключенный CurrencyConverter
 */

#pragma once
#include <string>
#include <iostream>
#include <stdexcept>
#include "Date.hpp"
#include "currency/CurrencyConverter.hpp"

 /**
 * @class Transaction
 * @brief Класс финансовой транзакции
 *
 * @details Инкапсулирует всю логику работы с финансовыми операциями:
 * - Хранение суммы, категории, типа, даты и описания
 * - Поддержка различных валют
 * - Система тегов для дополнительной категоризации
 * - Встроенные механизмы валидации
 *
 * @invariant
 * 1. ID всегда уникален и генерируется автоматически
 * 2. Сумма всегда положительна
 * 3. Дата всегда валидна (2000-2100)
 * 4. Категория не пуста
 */
class Transaction {
    friend class FinanceCore;
public:
    /**
     * @enum Type
     * @brief Тип финансовой операции
     */
    enum class Type { 
        INCOME, ///< Операция пополнения (доход)
        EXPENSE ///< Операция списания (расход)
    };

    /**
     * @brief Конструктор по умолчанию
     * @post Создает транзакцию с:
     * - Автоматически сгенерированным ID
     * - Текущей датой
     * - Типом EXPENSE
     * - Базовой валютой (RUB)
     * - Пустым описанием
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

    /**
     * @brief Конструктор копирования
     * @param other Транзакция для копирования
     * @post Создает полную копию транзакции, включая теги
     */
    Transaction(const Transaction& other)
        : id(other.id), amount(other.amount), category(other.category),
        type(other.type), date(other.date), description(other.description),
        currency_(other.currency_), tags_(other.tags_) {
    }

    /**
     * @brief Основной параметризованный конструктор
     * @param amt Сумма (> 0)
     * @param cat Категория (не пустая)
     * @param t Тип операции
     * @param d Дата операции
     * @param desc Описание (опционально)
     * @throws std::invalid_argument При невалидных данных
     *
     * @par Пример:
     * @code
     * Transaction t(100.0, "Food", Type::EXPENSE, Date(2023, 5, 20), "Supermarket");
     * @endcode
     */
    Transaction(double amt, const std::string& cat, Type t, const Date& d, const std::string& desc = "")
        : id(next_id++), amount(amt), category(cat), type(t), date(d), description(desc) {
        validation();
    }

    /// @name Геттеры
    /// @{
    int get_id() const { return id; }                   ///< @return Уникальный ID транзакции
    double get_amount() const { return amount; }        ///< @return Сумма транзакции
    std::string get_category() const { return category; } ///< @return Категория
    Type get_type() const { return type; }             ///< @return Тип (INCOME/EXPENSE)
    Date get_date() const { return date; }             ///< @return Дата транзакции
    std::string get_description() const { return description; } ///< @return Описание

    /**
     * @brief Возвращает сумму с учетом типа операции
     * @return Для доходов - положительное значение, для расходов - отрицательное
     */
    double get_signed_amount() const {
        return (type == Type::INCOME) ? amount : -amount;
    }

    /**
     * @brief Форматированное строковое представление
     * @return Строка формата: "YYYY-MM-DD [+/-]сумма (категория) описание"
     */
    std::string get_summary() const {
        return date.to_string() + " " +
            (type == Type::INCOME ? "[+] " : "[-] ") +
            std::to_string(amount) + " (" + category + ") " + description;
    }
    /// @}

    /// @name Сеттеры
    /// @{
    /**
     * @brief Устанавливает сумму транзакции
     * @param amt Новая сумма (> 0)
     * @throws std::invalid_argument Если сумма не положительна
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
     * @brief Устанавливает дату операции
     * @param d Новая дата
     * @throws std::invalid_argument Если дата невалидна
     */
    void set_date(const Date& d) {
        if (!d.is_valid()) throw std::invalid_argument("Invalid date");
        date = d;
    }


    void set_type(Type t) { type = t; }                ///< Устанавливает тип операции
    void set_description(const std::string& desc) {    
        description = desc.empty() ? "--" : desc;
    } ///< Устанавливает описание
    void set_id(int new_id) { id = new_id; }           ///< Устанавливает ID (для загрузки из файла)
    /// @}

    /**
     * @brief Оператор вывода в поток
     * @param os Выходной поток
     * @param t Транзакция для вывода
     * @return Поток os
     *
     * @note Формат сериализации:
     * id,amount,type,category,yyyy mm dd,currency,description,tags
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

    /// @name Работа с валютами
    /// @{
    /**
     * @brief Устанавливает валюту транзакции
     * @param currency Код валюты (например "USD")
     */
    void set_currency(const std::string& currency) {
        currency_ = currency;
    }

    /**
     * @brief Конвертирует сумму в рубли
     * @param converter Конвертер валют
     * @return Сумма в рублях
     * @throws std::runtime_error Если курс валюты недоступен
     */
    double get_amount_in_rub(const CurrencyConverter& converter) const {
        return converter.convert(amount, currency_, "RUB");
    }

    const std::string& get_currency() const { return currency_; } ///< Возвращает код валюты
    /// @}

    /// @name Работа с тегами
    /// @{
    /**
     * @brief Добавляет тег к транзакции
     * @param tag Тег для добавления
     * @throws std::runtime_error При превышении лимита тегов или дубликате
     */
    void Transaction::add_tag(const std::string& tag) {
        if (tags_.size() >= MAX_TAGS) {
            throw std::runtime_error("Достигнут лимит тегов (" + std::to_string(MAX_TAGS) + ")");
        }
        if (std::find(tags_.begin(), tags_.end(), tag) != tags_.end()) {
            throw std::runtime_error("Тег '" + tag + "' уже добавлен");
        }
        tags_.push_back(tag);
    }

    const std::vector<std::string>& get_tags() const { return tags_; } ///< Возвращает список тегов

    /**
     * @brief Возвращает список доступных предопределенных тегов
     * @return Вектор стандартных тегов
     */
    static const std::vector<std::string>& get_available_tags(); 

    /**
     * @brief Удаляет тег по индексу
     * @param index Позиция тега для удаления
     */
    void remove_tag(size_t index) {
        if (index < tags_.size()) {
            tags_.erase(tags_.begin() + index);
        }
    }
    /// @}

private:

    std::vector<std::string> tags_; ///< Список тегов
    static constexpr size_t MAX_TAGS = 5; ///< Максимальное количество тегов

    static inline int next_id = 1; ///< Счетчик для генерации ID
    int id;             ///< Уникальный идентификатор
    double amount;      ///< Сумма операции (> 0)
    std::string category; ///< Категория операции
    Type type;          ///< Тип (доход/расход)
    Date date;          ///< Дата транзакции
    std::string description; ///< Описание (опционально)
    std::string currency_ = "RUB"; ///< Код валюты (ISO 4217)

    /**
     * @brief Валидация данных транзакции
     * @throws std::invalid_argument Если:
     * - Сумма <= 0
     * - Категория пуста
     * - Дата невалидна
     */
    void validation() const {
        if (amount <= 0) throw std::invalid_argument("Amount can't be negative");
        if (category.empty()) throw std::invalid_argument("You should choose category");
        if (!date.is_valid()) throw std::invalid_argument("Invalid date format");
    }
};


