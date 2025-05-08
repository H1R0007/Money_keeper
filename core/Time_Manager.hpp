#pragma once
#include <string>
#include <iostream>
#include <stdexcept>
#include "Date.hpp"

class Transaction {

public:
    enum class Type { INCOME, EXPENSE };

    Transaction()
        : id(next_id++),
        amount(0.0),
        category("Uncategorized"),
        type(Type::EXPENSE),
        date(),  // Текущая дата
        description("")
    {
    }

    //Умолчательные конструкторы
    Transaction(double amt, const std::string& cat, Type t, const Date& d, const std::string& desc = "")
        : id(next_id++), amount(amt), category(cat), type(t), date(d), description(desc) {
        validation();
    }

    //Геттеры
    int get_id() const { return id; }
    double get_amount() const { return amount; }
    std::string get_category() const { return category; }
    Type get_type() const { return type; }
    Date get_date() const { return date; }
    std::string get_description() const { return description; }

    //Корректирующие сеттеры
    void set_amount(double amt) {
        if (amt <= 0) throw std::invalid_argument("Amount must be positive");
        amount = amt;
    }

    void set_category(const std::string& cat) {
        if (cat.empty()) throw std::invalid_argument("Category cannot be empty");
        category = cat;
    }

    void set_date(const Date& d) {
        if (!d.is_valid()) throw std::invalid_argument("Invalid date");
        date = d;
    }


    void set_type(Type t) { type = t; }

    void set_description(const std::string& desc) { description = desc; }

    void set_id(int new_id) { id = new_id; }


    //Метод для получения суммы с учетом типа операции
    double get_signed_amount() const {
        return (type == Type::INCOME) ? amount : -amount;
    }

    //Форматированный вывод 
    std::string get_summary() const {
        std::string typeSymbol = (type == Type::INCOME) ? "[+] " : "[-] ";
        return date.to_string() + " " + typeSymbol + std::to_string(amount) +
            " (" + category + ") " + description;
    }

    //Перевод данных для вывода ... вывод
    friend std::ostream& operator<<(std::ostream& os, const Transaction& t) {
        os << t.id << ","
            << t.amount << ","
            << static_cast<int>(t.type) << ","
            << t.category << ","
            << t.date << ","
            << t.description;
        return os;
    }

    //Перегрузка оператора ввода
    friend std::istream& operator>>(std::istream& is, Transaction& t) {
        char delimiter;
        int type;
        is >> t.id >> delimiter
            >> t.amount >> delimiter
            >> type >> delimiter;
        t.type = static_cast<Type>(type);
        std::getline(is, t.category, ',');
        is >> t.date >> delimiter;
        std::getline(is, t.description);
        return is;
    }

private:
    static inline int next_id = 1;
    int id;
    double amount;
    std::string category;
    Type type;
    Date date;
    std::string description;

    //Корректировка данных
    void validation() const
    {
        if (amount <= 0)
        {
            throw std::invalid_argument("Amount can`t be negative");
        }
        if (category.empty())
        {
            throw std::invalid_argument("You should choose category");
        }
        if (!date.is_valid())
        {
            throw std::invalid_argument("Invalid format");
        }
    }
};


