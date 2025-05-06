#include <string>
#include <iostream>
#include <stdexcept>
#include "Date.h"


// Инициализация статической переменной
int Transaction::next_id = 1;


class Transaction {
private:
    static int next_id; //Переменная для создания уникальных ID
    int id;
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

public:
    enum class Type {income, expense};

    //Умолчательные конструкторы
    Transaction(double amt, const std::string& cat, Type t, const Date& d, const std::string& desc = "")
        : id(next_id++), amount(amt), category(cat), type(t), date(d), description(desc) {
        validate();
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

    //Метод для получения суммы с учетом типа операции
    double get_signed_amount() const {
        return (type == Type::INCOME) ? amount : -amount;
    }

    //Форматированный вывод 
    std::string get_summary() const {
        std::string typeSymbol = (type == Type::INCOME) ? "[+] " : "[-] ";
        return date.toString() + " " + typeSymbol + std::to_string(amount) +
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
};

