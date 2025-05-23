/**
 * @file Time_Manager.hpp
 * @brief ���������� ����������� ������������ � �� ���������� ����������������
 *
 * @details ���� ������ ������������� ���������� ��� ������ � ����������� ����������,
 *          ������� �� ��������, ���������, ������������ � ������ ��������� �����.
 *          ����������� ��������� ����������������, ����� � ������� ��������� ��������.
 *
 * @section features_sec �������� �����������
 * - �������� ���������� � �������������� ���������� ID
 * - ��������� �������/��������
 * - ��������� ���� ����� ����������
 * - ������ � ������ � ���������� ���������
 * - ����������� �����
 * - ������� ����� ��� �������������
 *
 * @section design_sec ������������� �������
 * 1. ������������ ������� "Value Object" ��� ����������
 * 2. ���������������� ����������
 * 3. ��������� ������������/��������������
 * 4. ������� ��������� ������
 *
 * @warning ��� �������� � ������ ���������� �������� 2000-2100 ��.
 * @note ��� ������ � �������� ��������� ������������ CurrencyConverter
 */

#pragma once
#include <string>
#include <iostream>
#include <stdexcept>
#include "Date.hpp"
#include "currency/CurrencyConverter.hpp"

 /**
 * @class Transaction
 * @brief ����� ���������� ����������
 *
 * @details ������������� ��� ������ ������ � ����������� ����������:
 * - �������� �����, ���������, ����, ���� � ��������
 * - ��������� ��������� �����
 * - ������� ����� ��� �������������� �������������
 * - ���������� ��������� ���������
 *
 * @invariant
 * 1. ID ������ �������� � ������������ �������������
 * 2. ����� ������ ������������
 * 3. ���� ������ ������� (2000-2100)
 * 4. ��������� �� �����
 */
class Transaction {
    friend class FinanceCore;
public:
    /**
     * @enum Type
     * @brief ��� ���������� ��������
     */
    enum class Type { 
        INCOME, ///< �������� ���������� (�����)
        EXPENSE ///< �������� �������� (������)
    };

    /**
     * @brief ����������� �� ���������
     * @post ������� ���������� �:
     * - ������������� ��������������� ID
     * - ������� �����
     * - ����� EXPENSE
     * - ������� ������� (RUB)
     * - ������ ���������
     */
    Transaction()
        : id(next_id++),
        amount(0.0),
        category("Uncategorized"),
        type(Type::EXPENSE),
        date(),  // ������� ����
        description("")
    {
    }

    /**
     * @brief ����������� �����������
     * @param other ���������� ��� �����������
     * @post ������� ������ ����� ����������, ������� ����
     */
    Transaction(const Transaction& other)
        : id(other.id), amount(other.amount), category(other.category),
        type(other.type), date(other.date), description(other.description),
        currency_(other.currency_), tags_(other.tags_) {
    }

    /**
     * @brief �������� ����������������� �����������
     * @param amt ����� (> 0)
     * @param cat ��������� (�� ������)
     * @param t ��� ��������
     * @param d ���� ��������
     * @param desc �������� (�����������)
     * @throws std::invalid_argument ��� ���������� ������
     *
     * @par ������:
     * @code
     * Transaction t(100.0, "Food", Type::EXPENSE, Date(2023, 5, 20), "Supermarket");
     * @endcode
     */
    Transaction(double amt, const std::string& cat, Type t, const Date& d, const std::string& desc = "")
        : id(next_id++), amount(amt), category(cat), type(t), date(d), description(desc) {
        validation();
    }

    /// @name �������
    /// @{
    int get_id() const { return id; }                   ///< @return ���������� ID ����������
    double get_amount() const { return amount; }        ///< @return ����� ����������
    std::string get_category() const { return category; } ///< @return ���������
    Type get_type() const { return type; }             ///< @return ��� (INCOME/EXPENSE)
    Date get_date() const { return date; }             ///< @return ���� ����������
    std::string get_description() const { return description; } ///< @return ��������

    /**
     * @brief ���������� ����� � ������ ���� ��������
     * @return ��� ������� - ������������� ��������, ��� �������� - �������������
     */
    double get_signed_amount() const {
        return (type == Type::INCOME) ? amount : -amount;
    }

    /**
     * @brief ��������������� ��������� �������������
     * @return ������ �������: "YYYY-MM-DD [+/-]����� (���������) ��������"
     */
    std::string get_summary() const {
        return date.to_string() + " " +
            (type == Type::INCOME ? "[+] " : "[-] ") +
            std::to_string(amount) + " (" + category + ") " + description;
    }
    /// @}

    /// @name �������
    /// @{
    /**
     * @brief ������������� ����� ����������
     * @param amt ����� ����� (> 0)
     * @throws std::invalid_argument ���� ����� �� ������������
     */
    void set_amount(double amt) {
        if (amt <= 0) throw std::invalid_argument("Amount must be positive");
        amount = amt;
    }

    /**
     * @brief ������������� ���������
     * @param cat ����� ��������� (�� ������)
     * @throws std::invalid_argument ���� ��������� �����
     */
    void set_category(const std::string& cat) {
        if (cat.empty()) throw std::invalid_argument("Category cannot be empty");
        category = cat;
    }

    /**
     * @brief ������������� ���� ��������
     * @param d ����� ����
     * @throws std::invalid_argument ���� ���� ���������
     */
    void set_date(const Date& d) {
        if (!d.is_valid()) throw std::invalid_argument("Invalid date");
        date = d;
    }


    void set_type(Type t) { type = t; }                ///< ������������� ��� ��������
    void set_description(const std::string& desc) {    
        description = desc.empty() ? "--" : desc;
    } ///< ������������� ��������
    void set_id(int new_id) { id = new_id; }           ///< ������������� ID (��� �������� �� �����)
    /// @}

    /**
     * @brief �������� ������ � �����
     * @param os �������� �����
     * @param t ���������� ��� ������
     * @return ����� os
     *
     * @note ������ ������������:
     * id,amount,type,category,yyyy mm dd,currency,description,tags
     */
    friend std::ostream& operator<<(std::ostream& os, const Transaction& t) {
        os << t.id << ","
            << t.amount << ","
            << static_cast<int>(t.type) << ","
            << t.category << ","
            << t.date << ","
            << t.currency_ << ","  // ��������� ���������� ������
            << t.description;
        return os;
    }

    /**
     * @brief �������� ����� �� ������
     * @param is ������� �����
     * @param t ���������� ��� ����������
     * @return ����� is
     * @throws std::runtime_error ��� ������� �������
     * @throws std::ios_base::failure ��� ���������� ������
     */
    friend std::istream& operator>>(std::istream& is, Transaction& t) {
        std::string line;
        if (!std::getline(is, line)) return is;

        std::istringstream iss(line);
        char delim;
        int type;

        try {
            if (!(iss >> t.id >> delim >> t.amount >> delim >> type >> delim)) {
                throw std::runtime_error("������ ������ ������� ����������");
            }
            t.type = static_cast<Transaction::Type>(type);

            // ������ ��������� (�� �������)
            std::getline(iss, t.category, ',');

            // ������ ����
            int y, m, d;
            if (!(iss >> y >> m >> d >> delim)) {
                throw std::runtime_error("������ ������ ����");
            }
            t.date = Date(y, m, d);
            
            // ������� ������ - ��������
            std::getline(iss, t.currency_, ',');
            std::getline(iss, t.description);
        }
        catch (const std::exception& e) {
            std::cerr << "������ ������ ����������: " << e.what() << "\n";
            is.setstate(std::ios::failbit);
        }

        return is;
    }

    /// @name ������ � ��������
    /// @{
    /**
     * @brief ������������� ������ ����������
     * @param currency ��� ������ (�������� "USD")
     */
    void set_currency(const std::string& currency) {
        currency_ = currency;
    }

    /**
     * @brief ������������ ����� � �����
     * @param converter ��������� �����
     * @return ����� � ������
     * @throws std::runtime_error ���� ���� ������ ����������
     */
    double get_amount_in_rub(const CurrencyConverter& converter) const {
        return converter.convert(amount, currency_, "RUB");
    }

    const std::string& get_currency() const { return currency_; } ///< ���������� ��� ������
    /// @}

    /// @name ������ � ������
    /// @{
    /**
     * @brief ��������� ��� � ����������
     * @param tag ��� ��� ����������
     * @throws std::runtime_error ��� ���������� ������ ����� ��� ���������
     */
    void Transaction::add_tag(const std::string& tag) {
        if (tags_.size() >= MAX_TAGS) {
            throw std::runtime_error("��������� ����� ����� (" + std::to_string(MAX_TAGS) + ")");
        }
        if (std::find(tags_.begin(), tags_.end(), tag) != tags_.end()) {
            throw std::runtime_error("��� '" + tag + "' ��� ��������");
        }
        tags_.push_back(tag);
    }

    const std::vector<std::string>& get_tags() const { return tags_; } ///< ���������� ������ �����

    /**
     * @brief ���������� ������ ��������� ���������������� �����
     * @return ������ ����������� �����
     */
    static const std::vector<std::string>& get_available_tags(); 

    /**
     * @brief ������� ��� �� �������
     * @param index ������� ���� ��� ��������
     */
    void remove_tag(size_t index) {
        if (index < tags_.size()) {
            tags_.erase(tags_.begin() + index);
        }
    }
    /// @}

private:

    std::vector<std::string> tags_; ///< ������ �����
    static constexpr size_t MAX_TAGS = 5; ///< ������������ ���������� �����

    static inline int next_id = 1; ///< ������� ��� ��������� ID
    int id;             ///< ���������� �������������
    double amount;      ///< ����� �������� (> 0)
    std::string category; ///< ��������� ��������
    Type type;          ///< ��� (�����/������)
    Date date;          ///< ���� ����������
    std::string description; ///< �������� (�����������)
    std::string currency_ = "RUB"; ///< ��� ������ (ISO 4217)

    /**
     * @brief ��������� ������ ����������
     * @throws std::invalid_argument ����:
     * - ����� <= 0
     * - ��������� �����
     * - ���� ���������
     */
    void validation() const {
        if (amount <= 0) throw std::invalid_argument("Amount can't be negative");
        if (category.empty()) throw std::invalid_argument("You should choose category");
        if (!date.is_valid()) throw std::invalid_argument("Invalid date format");
    }
};


