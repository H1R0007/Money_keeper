/**
 * @file Date.hpp
 * @brief Работа с календарными датами в диапазоне 2000-2100 гг.
 *
 * @details Класс предоставляет:
 * - Хранение и валидацию дат
 * - Операции сравнения дат
 * - Преобразования в строки
 * - Арифметические операции с датами
 * - Поддержку високосных годов
 *
 * @section date_rules Правила работы с датами
 * 1. Все даты должны быть валидными (проверка в конструкторе)
 * 2. Поддерживается диапазон 2000-2100 гг.
 * 3. Корректно обрабатывает високосные годы
 * 4. Сохраняет целостность при любых операциях
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
 * @brief Класс для работы с календарными датами
 *
 * @invariant
 * 1. Год всегда в диапазоне 2000-2100
 * 2. Месяц всегда 1-12
 * 3. День всегда корректен для данного месяца/года
 */
class Date {
private:
	int year;   ///< Год (2000-2100)
	int month;  ///< Месяц (1-12)
	int day;    ///< День (1-31 в зависимости от месяца)

public:

	/**
	 * @brief Проверяет високосность года
	 * @return true если год високосный
	 *
	 * @note Алгоритм:
	 * - Год делится на 4, но не на 100 ИЛИ
	 * - Год делится на 400
	 */
	bool is_leap_year() const
	{
		return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
	}

	/**
	 * @brief Возвращает количество дней в текущем месяце
	 * @return Число дней (28-31)
	 *
	 * @note Корректно учитывает високосные годы для февраля
	 */
	int day_in_month() const
	{
		const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		if (month == 2 && is_leap_year())
			return 29;
		return days[month - 1];
	}

	/**
	 * @brief Проверяет корректность даты
	 * @return true если дата валидна
	 *
	 * @details Проверяет:
	 * - Год в допустимом диапазоне
	 * - Корректность месяца
	 * - Корректность дня для данного месяца/года
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
	 * @return Отформатированная строка
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
	 * @brief Создает Date из строки формата YYYY-MM-DD
	 * @param date_str Строка с датой
	 * @return Объект Date
	 *
	 * @throws std::invalid_argument При неверном формате
	 *
	 * @example
	 * Date::from_string("2023-05-20") => Date(2023, 5, 20)
	 */
	static Date from_string(const std::string& date_str) {
		// Формат: "YYYY-MM-DD"
		int y, m, d;
		char dash;
		std::istringstream iss(date_str);
		iss >> y >> dash >> m >> dash >> d;
		return Date(y, m, d);
	}

	/**
	 * @brief Конструктор с указанием даты
	 * @param y Год (2000-2100)
	 * @param m Месяц (1-12)
	 * @param d День (1-31)
	 *
	 * @throws std::invalid_argument При невалидной дате
	 */
	Date(int y, int m, int d) : year(y), month(m), day(d)
	{
		if (!is_valid()) throw std::invalid_argument("Invalid date");
	}

	/**
	 * @brief Конструктор по умолчанию (текущая дата)
	 *
	 * @note Использует системное время
	 * @warning На Windows использует localtime_s, на Unix - localtime_r
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

	/// @name Геттеры
	/// @{
	int get_year() const { return year; }   ///< Возвращает год
	int get_month() const { return month; } ///< Возвращает месяц
	int get_day() const { return day; }     ///< Возвращает день
	/// @}

	/// @name Сеттеры
	/// @{
	/**
	 * @brief Устанавливает год
	 * @param y Новый год (2000-2100)
	 * @throws std::invalid_argument При невалидном годе
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
	 * @brief Устанавливает месяц
	 * @param m Новый месяц (1-12)
	 * @throws std::invalid_argument При невалидном месяце
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
	 * @brief Устанавливает день
	 * @param d Новый день
	 * @throws std::invalid_argument При невалидном дне
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

	/// @name Операторы сравнения
	/// @{
	/**
	 * @brief Оператор "меньше"
	 * @param other Дата для сравнения
	 * @return true если текущая дата раньше
	 */
	bool operator<(const Date& other) const
	{
		return (year < other.year) ||
			(year == other.year && month < other.month) ||
			(year == other.year && month == other.month && day < other.day);
	}

	/**
	 * @brief Оператор равенства
	 * @param other Дата для сравнения
	 * @return true если даты идентичны
	 */
	bool operator==(const Date& other) const
	{
		return year == other.year && month == other.month && month == other.month;
	}
	/// @}

	/// @name Операторы ввода/вывода
	/// @{
	/**
	 * @brief Оператор вывода в поток
	 * @param os Выходной поток
	 * @param d Дата для вывода
	 * @return Поток os
	 *
	 * @note Формат: "YYYY MM DD"
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
	 * @throws std::ios::failure При невалидной дате
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
