#pragma once
#include <iostream>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <ctime>


class Date {
private:
	int year;
	int month;
	int day;

public:

	//Проверка на високосный год
	bool is_leap_year() const
	{
		return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
	}

	//Проверка на кол-во дней в месяце
	int day_in_month() const
	{
		const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		if (month == 2 && is_leap_year())
			return 29;
		return days[month - 1];
	}

	//Проверка корректности даты
	bool is_valid() const
	{
		return (year >= 2000) &&
			(month >= 1 && month <= 12) &&
			(day >= 1 && day <= day_in_month());
	}

	//Умолчательный конструктор 
	Date(int y, int m, int d) : year(y), month(m), day(d)
	{
		if (!is_valid()) throw std::invalid_argument("Invalid date");
	}

	//Установка текущей даты по умолчанию
	Date() {
		std::time_t time = std::time(nullptr);

		// Для Windows
#ifdef _WIN32
		std::tm now;
		localtime_s(&now, &time);
		// Для Linux/macOS
#else
		std::tm* now = std::localtime(&time);
#endif

		year = now.tm_year + 1900;
		month = now.tm_mon + 1;
		day = now.tm_mday;
	}

	//Геттеры
	int get_year() const { return year; }
	int get_month() const { return month; }
	int get_day() const { return day; }

	//Сеттеры + проверка корректности 
	void seet_year(int y)
	{
		int temp = year;
		year = y;
		if (!is_valid())
		{
			year = temp;
			throw std::invalid_argument("Invalid year!");
		}
	}

	void seet_month(int m)
	{
		int temp = month;
		month = m;
		if (!is_valid())
		{
			month = temp;
			throw std::invalid_argument("Invalid month!");
		}
	}

	void seet_day(int d)
	{
		int temp = day;
		day = d;
		if (!is_valid())
		{
			day = temp;
			throw std::invalid_argument("Invalid day!");
		}
	}

	//Форматированный вывод
	std::string to_string() const
	{
		std::ostringstream ostrm;
		ostrm << std::setw(4) << std::setfill('0') << year << "."
			<< std::setw(2) << std::setfill('0') << month << "."
			<< std::setw(2) << std::setfill('0') << day;
		return ostrm.str();
	}

	//Перегружаем операторы сравнения
	bool operator<(const Date& other) const
	{
		return (year < other.year) ||
			(year == other.year && month < other.month) ||
			(year == other.year && month == other.month && day < other.day);
	}

	bool operator==(const Date& other) const
	{
		return year == other.year && month == other.month && month == other.month;
	}

	//Для работы с файлами(перегрузка ввода/вывода)
	friend std::ostream& operator<<(std::ostream& os, const Date& d)
	{
		os << d.year << " " << d.month << " " << d.day;
		return os;
	}

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