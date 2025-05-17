/**
 * @file Date.hpp
 * @brief Заголовочный файл класса Date для работы с датами
 * @author Сонин Михаил/Эксузян Давид
 * @version 1.0
 * @date 2025-05-16
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
  * @brief Класс для работы с датами в диапазоне 2000-2100 годов
  *
  * Класс предоставляет функционал для:
  * - Хранения и валидации дат
  * - Определения високосных годов
  * - Сравнения дат
  * - Сериализации/десериализации
  */
class Date {
private:
	int year; ///< Год (2000-2100)
	int month; ///< Месяц (1-12)
	int day; ///< День (1-31 в зависимости от месяца)

public:

	/**
	 * @brief Проверяет, является ли год високосным
	 * @return true если год високосный
	 *
	 * @note Год считается високосным, если он делится на 4, но не на 100,
	 *       или делится на 400
	 */
	bool is_leap_year() const
	{
		return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
	}

	/**
	 * @brief Возвращает количество дней в текущем месяце
	 * @return Число дней в месяце с учетом високосных годов
	 */
	int day_in_month() const
	{
		const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		if (month == 2 && is_leap_year())
			return 29;
		return days[month - 1];
	}

	/**
	 * @brief Проверяет корректность текущей даты
	 * @return true если дата валидна (2000-2100, правильные месяц/день)
	 */
	bool is_valid() const {
		if (year < 2000 || year > 2100) return false;
		if (month < 1 || month > 12) return false;

		const int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		int max_days = days_in_month[month - 1];

		// Обработка високосного года для февраля
		if (month == 2 && is_leap_year()) {
			max_days = 29;
		}

		return (day >= 1 && day <= max_days);
	}

	/**
	 * @brief Преобразует дату в строку формата YYYY-MM-DD
	 * @return Отформатированная строка с датой
	 */
	std::string to_string() const {
		std::ostringstream oss;
		oss << std::setw(4) << std::setfill('0') << year << "-"
			<< std::setw(2) << std::setfill('0') << month << "-"
			<< std::setw(2) << std::setfill('0') << day;
		return oss.str();
	}

	/**
	 * @brief Конструктор с указанием даты
	 * @param y Год (2000-2100)
	 * @param m Месяц (1-12)
	 * @param d День (1-31 в зависимости от месяца)
	 * @throws std::invalid_argument Если дата некорректна
	 */
	Date(int y, int m, int d) : year(y), month(m), day(d)
	{
		if (!is_valid()) throw std::invalid_argument("Invalid date");
	}

	/**
	 * @brief Конструктор по умолчанию (устанавливает текущую дату)
	 * @note Использует системное время, кросс-платформенная реализация
	 */
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
	int get_year() const { return year; } ///< @return Текущий год
	int get_month() const { return month; } ///< @return Текущий месяц
	int get_day() const { return day; } ///< @return Текущий день

	/**
	 * @brief Устанавливает год
	 * @param y Новый год (2000-2100)
	 * @throws std::invalid_argument Если год некорректен
	 */
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

	/**
	 * @brief Устанавливает месяц
	 * @param m Новый месяц (1-12)
	 * @throws std::invalid_argument Если месяц некорректен
	 */
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

	/**
	 * @brief Устанавливает день
	 * @param d Новый день
	 * @throws std::invalid_argument Если день некорректен для текущего месяца/года
	 */
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

	/**
	 * @brief Оператор сравнения "меньше"
	 * @param other Дата для сравнения
	 * @return true если текущая дата раньше other
	 */
	bool operator<(const Date& other) const
	{
		return (year < other.year) ||
			(year == other.year && month < other.month) ||
			(year == other.year && month == other.month && day < other.day);
	}

	/**
	* @brief Оператор сравнения на равенство
	* @param other Дата для сравнения
	* @return true если даты идентичны
	*/
	bool operator==(const Date& other) const
	{
		return year == other.year && month == other.month && month == other.month;
	}

	/**
	 * @brief Оператор вывода в поток
	 * @param os Выходной поток
	 * @param d Дата для вывода
	 * @return Поток os
	 *
	 * @note Формат вывода: "YYYY MM DD"
	 */
	friend std::ostream& operator<<(std::ostream& os, const Date& d)
	{
		os << d.year << " " << d.month << " " << d.day;
		return os;
	}

	/**
	 * @brief Оператор ввода из потока
	 * @param is Входной поток
	 * @param d Дата для заполнения
	 * @return Поток is
	 * @throws std::ios::failure Если введена некорректная дата
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
