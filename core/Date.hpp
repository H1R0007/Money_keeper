/**
 * @file Date.hpp
 * @brief ������ � ������������ ������ � ��������� 2000-2100 ��.
 *
 * @details ����� �������������:
 * - �������� � ��������� ���
 * - �������� ��������� ���
 * - �������������� � ������
 * - �������������� �������� � ������
 * - ��������� ���������� �����
 *
 * @section date_rules ������� ������ � ������
 * 1. ��� ���� ������ ���� ��������� (�������� � ������������)
 * 2. �������������� �������� 2000-2100 ��.
 * 3. ��������� ������������ ���������� ����
 * 4. ��������� ����������� ��� ����� ���������
 */


#pragma once
#include <iostream>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <ctime>

 /**
 * @class Date
 * @brief ����� ��� ������ � ������������ ������
 *
 * @invariant
 * 1. ��� ������ � ��������� 2000-2100
 * 2. ����� ������ 1-12
 * 3. ���� ������ ��������� ��� ������� ������/����
 */
class Date {
private:
	int year;   ///< ��� (2000-2100)
	int month;  ///< ����� (1-12)
	int day;    ///< ���� (1-31 � ����������� �� ������)

public:

	/**
	 * @brief ��������� ������������ ����
	 * @return true ���� ��� ����������
	 *
	 * @note ��������:
	 * - ��� ������� �� 4, �� �� �� 100 ���
	 * - ��� ������� �� 400
	 */
	bool is_leap_year() const
	{
		return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
	}

	/**
	 * @brief ���������� ���������� ���� � ������� ������
	 * @return ����� ���� (28-31)
	 *
	 * @note ��������� ��������� ���������� ���� ��� �������
	 */
	int day_in_month() const
	{
		const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		if (month == 2 && is_leap_year())
			return 29;
		return days[month - 1];
	}

	/**
	 * @brief ��������� ������������ ����
	 * @return true ���� ���� �������
	 *
	 * @details ���������:
	 * - ��� � ���������� ���������
	 * - ������������ ������
	 * - ������������ ��� ��� ������� ������/����
	 */
	bool is_valid() const {
		if (year < 2000 || year > 2100) return false;
		if (month < 1 || month > 12) return false;

		const int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		int max_days = days_in_month[month - 1];

		// ��������� ����������� ���� ��� �������
		if (month == 2 && is_leap_year()) {
			max_days = 29;
		}

		return (day >= 1 && day <= max_days);
	}

	/**
	 * @brief ����������� ���� � ������ ������� YYYY-MM-DD
	 * @return ����������������� ������
	 *
	 * @example
	 * Date(2023, 5, 20).to_string() => "2023-05-20"
	 */
	std::string to_string() const {
		std::ostringstream oss;
		oss << std::setw(4) << std::setfill('0') << year << "-"
			<< std::setw(2) << std::setfill('0') << month << "-"
			<< std::setw(2) << std::setfill('0') << day;
		return oss.str();
	}

	/**
	 * @brief ������� Date �� ������ ������� YYYY-MM-DD
	 * @param date_str ������ � �����
	 * @return ������ Date
	 *
	 * @throws std::invalid_argument ��� �������� �������
	 *
	 * @example
	 * Date::from_string("2023-05-20") => Date(2023, 5, 20)
	 */
	static Date from_string(const std::string& date_str) {
		// ������: "YYYY-MM-DD"
		int y, m, d;
		char dash;
		std::istringstream iss(date_str);
		iss >> y >> dash >> m >> dash >> d;
		return Date(y, m, d);
	}

	/**
	 * @brief ����������� � ��������� ����
	 * @param y ��� (2000-2100)
	 * @param m ����� (1-12)
	 * @param d ���� (1-31)
	 *
	 * @throws std::invalid_argument ��� ���������� ����
	 */
	Date(int y, int m, int d) : year(y), month(m), day(d)
	{
		if (!is_valid()) throw std::invalid_argument("Invalid date");
	}

	/**
	 * @brief ����������� �� ��������� (������� ����)
	 *
	 * @note ���������� ��������� �����
	 * @warning �� Windows ���������� localtime_s, �� Unix - localtime_r
	 */
	Date() {
		std::time_t time = std::time(nullptr);

		// ��� Windows
#ifdef _WIN32
		std::tm now;
		localtime_s(&now, &time);
		// ��� Linux/macOS
#else
		std::tm* now = std::localtime(&time);
#endif

		year = now.tm_year + 1900;
		month = now.tm_mon + 1;
		day = now.tm_mday;
	}

	/// @name �������
	/// @{
	int get_year() const { return year; }   ///< ���������� ���
	int get_month() const { return month; } ///< ���������� �����
	int get_day() const { return day; }     ///< ���������� ����
	/// @}

	/// @name �������
	/// @{
	/**
	 * @brief ������������� ���
	 * @param y ����� ��� (2000-2100)
	 * @throws std::invalid_argument ��� ���������� ����
	 */
	void seet_year(int y) {
		int temp = year;
		year = y;
		if (!is_valid()) {
			year = temp;
			throw std::invalid_argument("Invalid year");
		}
	}

	/**
	 * @brief ������������� �����
	 * @param m ����� ����� (1-12)
	 * @throws std::invalid_argument ��� ���������� ������
	 */
	void seet_month(int m) {
		int temp = month;
		month = m;
		if (!is_valid()) {
			month = temp;
			throw std::invalid_argument("Invalid month");
		}
	}

	/**
	 * @brief ������������� ����
	 * @param d ����� ����
	 * @throws std::invalid_argument ��� ���������� ���
	 */
	void seet_day(int d) {
		int temp = day;
		day = d;
		if (!is_valid()) {
			day = temp;
			throw std::invalid_argument("Invalid day");
		}
	}
	/// @}

	/// @name ��������� ���������
	/// @{
	/**
	 * @brief �������� "������"
	 * @param other ���� ��� ���������
	 * @return true ���� ������� ���� ������
	 */
	bool operator<(const Date& other) const
	{
		return (year < other.year) ||
			(year == other.year && month < other.month) ||
			(year == other.year && month == other.month && day < other.day);
	}

	/**
	 * @brief �������� ���������
	 * @param other ���� ��� ���������
	 * @return true ���� ���� ���������
	 */
	bool operator==(const Date& other) const
	{
		return year == other.year && month == other.month && month == other.month;
	}
	/// @}

	/// @name ��������� �����/������
	/// @{
	/**
	 * @brief �������� ������ � �����
	 * @param os �������� �����
	 * @param d ���� ��� ������
	 * @return ����� os
	 *
	 * @note ������: "YYYY MM DD"
	 */
	friend std::ostream& operator<<(std::ostream& os, const Date& d)
	{
		os << d.year << " " << d.month << " " << d.day;
		return os;
	}

	/**
	 * @brief �������� ����� �� ������
	 * @param is ������� �����
	 * @param d ���� ��� ����������
	 * @return ����� is
	 * @throws std::ios::failure ��� ���������� ����
	 */
	friend std::istream& operator>>(std::istream& is, Date& d)
	{
		is >> d.year >> d.month >> d.day;
		if (!d.is_valid())
		{
			is.setstate(std::ios::failbit);
		}
		return is;
	}
};
/// @}
