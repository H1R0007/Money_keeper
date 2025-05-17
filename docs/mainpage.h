/**
 * @mainpage Money Keeper - Полная документация
 * @tableofcontents
 *
 * @section intro_sec Введение
 * **Money Keeper** - это кроссплатформенное приложение для управления личными финансами с поддержкой:
 * - Мультивалютных операций
 * - Детальной аналитики
 * - Гибкой системы отчетов
 *
 * @image html images/Main_menu.png "Главное меню приложения" width=800px
 */

 /*!
  * @defgroup core_components Основные компоненты
  * @brief Архитектурные элементы системы
  */

  /**
   * @page features Функциональные возможности
   * @section transactions_sec Управление транзакциями
   *
   * @subsection add_transaction_sub Добавление операций
   * @image html images/Add_Transaction.png "Меню добавления транзакции" width=400px
   *
   * ### Техническая реализация:
   * 1. **Пошаговый процесс**:
   *    - Определение типа (доход/расход)
   *    - Ввод суммы с валидацией (> 0)
   *    - Выбор категории из списка или ручной ввод
   *    - Указание даты (по умолчанию - текущая)
   *
   * @code{.cpp}
   * // Пример создания транзакции с валидацией
   * try {
   *     Transaction t(
   *         1000.0,                    // amount
   *         "Зарплата",                // category
   *         Transaction::Type::INCOME,  // type
   *         Date::currentDate()         // date
   *     );
   *     currentAccount->addTransaction(t);
   * } catch (const std::invalid_argument& e) {
   *     std::cerr << "Ошибка: " << e.what();
   * }
   * @endcode
   *
   * @image html images/How_to_add_Transaction.png "Пример заполнения транзакции" width=600px
   *
   * ### Особенности:
   * - Автоматический пересчет баланса счета
   * - Поддержка отмены на любом этапе (ввод 0)
   * - Валидация данных перед добавлением:
   *   ```cpp
   *   void Transaction::validate() const {
   *       if (amount <= 0) throw std::invalid_argument("Сумма должна быть положительной");
   *       if (category.empty()) throw std::invalid_argument("Категория не может быть пустой");
   *   }
   *   ```
   *
   * @subsection delete_transaction_sub Удаление операций
   * @image html images/Delete_Transactions.png "Интерфейс удаления транзакций" width=700px
   *
   * ### Алгоритм работы:
   * 1. Показ списка всех транзакций в виде таблицы
   * 2. Ввод ID транзакции для удаления
   * 3. Поиск и удаление через:
   * @code{.cpp}
   * void Account::removeTransaction(int id) {
   *     auto it = std::find_if(transactions.begin(), transactions.end(),
   *         [id](const Transaction& t) { return t.get_id() == id; });
   *
   *     if (it != transactions.end()) {
   *         balance -= it->get_signed_amount(); // Корректировка баланса
   *         transactions.erase(it);
   *     }
   * }
   * @endcode
   *
   * ### Важные нюансы:
   * - Используется "мягкое" удаление (без физического удаления из файла)
   * - При удалении автоматически пересчитывается баланс
   * - ID транзакции гарантированно уникален в рамках счета
   *
   * @see Transaction::validate() для проверки входных данных
   * @see Account::recalculateBalance() для ручного пересчета
   */
   /**
	* @page account_management Управление счетами
	* @section account_arch Архитектура системы счетов
	*
	* ### Хранение данных:
	* - Используется `std::map<std::string, Account>` для быстрого поиска по имени
	* - Текущий счет хранится как `Account* currentAccount`
	* - Автоматически создается счет "Общий" при инициализации
	*
	* @image html images/Account_managmment.png "Главное меню управления счетами" width=500px
	*
	* @subsection create_account Создание счета
	* ### Бизнес-логика:
	* 1. Проверка уникальности имени счета
	* 2. Создание нового объекта Account
	* 3. Добавление в коллекцию accounts
	*
	* @code{.cpp}
	* void FinanceCore::createAccount(const std::string& name) {
	*     if (accounts.find(name) != accounts.end())
	*         throw std::runtime_error("Счет уже существует");
	*
	*     accounts.emplace(name, Account(name));
	*     currentAccount = &accounts[name];
	* }
	* @endcode
	*
	* @image html images/Add_Account.png "Добавление нового счета" width=500px
	*
	* ### Ограничения:
	* - Максимальная длина имени: 50 символов
	* - Запрещены специальные символы (@, #, $)
	* - Имя "Общий" зарезервировано системой
	*
	* @subsection select_account Выбор счета
	* ### Алгоритм работы:
	* 1. Формирование нумерованного списка счетов
	* 2. Ввод пользователем номера счета
	* 3. Обновление currentAccount
	*
	* @code{.cpp}
	* void FinanceCore::selectAccount() {
	*     int i = 1;
	*     for (const auto& [name, acc] : accounts) {
	*         std::cout << i++ << ". " << name
	*                   << " (Баланс: " << acc.get_balance() << ")\n";
	*     }
	*     // ... обработка ввода
	* }
	* @endcode
	*
	* @image html images/Select_Account.png "Меню выбора счета" width=500px
	* @image html images/Where_to_watch_Account.png "Текущий счет в интерфейсе" width=600px
	*
	* @subsection delete_account Удаление счета
	* ### Критические правила:
	* - Нельзя удалить последний счет
	* - При удалении текущего счета происходит переключение на "Общий"
	* - Все транзакции удаляемого счета теряются
	*
	* @code{.cpp}
	* void FinanceCore::deleteAccount(const std::string& name) {
	*     if (accounts.size() <= 1)
	*         throw std::logic_error("Нельзя удалить последний счет");
	*
	*     if (currentAccount == &accounts[name])
	*         currentAccount = &accounts["Общий"];
	*
	*     accounts.erase(name);
	* }
	* @endcode
	*
	* @image html images/Delete_Account.png "Интерфейс удаления" width=500px
	*
	* @see Account::get_balance() для получения текущего баланса
	* @see FinanceCore::validateAccountName() для проверки имен
	*/

	/**
 * @page statistics Финансовая аналитика
 * @section reports_sub Виды отчетов
 *
 * ### Архитектура системы отчетности:
 * - Все отчеты генерируются на лету из текущих данных
 *
 * @subsection all_transactions Все транзакции
 * ### Техническая реализация:
 * ```cpp
 * // Пример генерации полного отчета
 * std::vector<Transaction> getAllTransactions() const {
 *     std::vector<Transaction> all_transactions;
 *     for (const auto& [name, account] : accounts) {
 *         const auto& txs = account.get_transactions();
 *         all_transactions.insert(all_transactions.end(), txs.begin(), txs.end());
 *     }
 *     std::sort(all_transactions.begin(), all_transactions.end(),
 *         [](const Transaction& a, const Transaction& b) {
 *             return a.get_date() < b.get_date();
 *         });
 *     return all_transactions;
 * }
 * ```
 * @image html images/Watch_All_transactions.png "Полный список операций" width=700px
 *
 * ### Особенности:
 * - Сортировка по дате (от новых к старым)
 *
 * @subsection by_category По категориям
 * ### Алгоритм агрегации:
 * ```cpp
 * std::map<std::string, std::pair<double, double>> getByCategory() const {
 *     std::map<std::string, std::pair<double, double>> stats;
 *     for (const auto& [name, account] : accounts) {
 *         for (const auto& t : account.get_transactions()) {
 *             auto& [income, expense] = stats[t.get_category()];
 *             if (t.get_type() == Transaction::Type::INCOME)
 *                 income += t.get_amount();
 *             else
 *                 expense += t.get_amount();
 *         }
 *     }
 *     return stats;
 * }
 * ```
 * @image html images/Watch_by_category.png "Статистика по категориям" width=500px
 *
 * ### Формат вывода:
 * - Расчет общего баланса по каждой категории
 *
 * @subsection by_month По месяцам
 * ### Метод расчета:
 * ```cpp
 * std::map<std::pair<int, int>, MonthlyStats> getMonthlyStats() const {
 *     std::map<std::pair<int, int>, MonthlyStats> result;
 *     for (const auto& [name, account] : accounts) {
 *         for (const auto& t : account.get_transactions()) {
 *             auto month_key = std::make_pair(
 *                 t.get_date().get_year(),
 *                 t.get_date().get_month()
 *             );
 *             auto& stats = result[month_key];
 *             stats.update(t);
 *         }
 *     }
 *     return result;
 * }
 * ```
 * @image html images/Watch_by_Month.png "Помесячная статистика" width=600px
 *
 * ### Особенности формата:
 * - Табличное представление с четкой структурой
 * - Выделение месяцев с отрицательным балансом
 *
 * @subsection balance Итоговый баланс
 * ### Логика расчета:
 * ```cpp
 * BalanceInfo getTotalBalance() const {
 *     BalanceInfo info;
 *     for (const auto& [name, account] : accounts) {
 *         info.total += account.get_balance();
 *         info.accounts_balance[name] = account.get_balance();
 *     }
 *     return info;
 * }
 * ```
 * @image html images/Itog_balance.png "Сводный финансовый отчет" width=400px
 *
 * ### Дополнительные метрики:
 * - Количество активных счетов
 *
 * @see Transaction::get_signed_amount() для корректного расчета баланса
 * @see Date::get_month_year() для работы с периодизацией
 */

 /**
  * @page ui_reference Справочник интерфейса
  * @section navigation_sec Навигация
  *
  * ### Общая архитектура UI:
  * - Консольное приложение с текстовым интерфейсом
  *
  * @subsection main_menu Главное меню
  * ### Техническая реализация:
  * ```cpp
  * class MainMenuState : public AppState {
  *     void render() override {
  *         printHeader("ФИНАНСОВЫЙ МЕНЕДЖЕР");
  *         printCurrentAccount();
  *         printOptions({
  *             {"1", "Добавить транзакцию"},
  *             {"2", "Просмотреть историю"},
  *             // ... другие пункты
  *         });
  *     }
  * };
  * ```
  * @image html images/Main_menu.png "Главное меню системы" width=600px
  *
  * ### Особенности:
  * - Автоматическое обновление баланса
  * - Валидация ввода (только цифры 0-5)
  * - Быстрые клавиши для навигации
  *
  * }
  * 
  * @image html images/Watch_History.png "Меню истории транзакций" width=500px
  *
  * ### Форматы отображения:
  * - Таблица с сортировкой по дате
  *
  * @subsection stats_menu Меню статистики
  * ### Система отчетов:
  * 
  * @image html images/Watch_Statistics.png "Меню аналитики" width=500px
  *
  * @section filters_sub Фильтры и сортировка
  *
  * @image html images/Watch_doxody.png "Фильтр доходов" width=600px
  * @image html images/Watch_rasxody.png "Фильтр расходов" width=600px
  *
  * ### Параметры фильтрации:
  * - По типу (доход/расход)
  * - По временному диапазону
  *
  */

  /**
   * @page technical_details Технические детали реализации
   * @tableofcontents
   *
   * @section paths_sec Расположение файлов
   *
   * @subsection images_subsec Изображения и ресурсы
   * Все графические материалы документации хранятся в директории:
   * @code
   * ./docs/images/
   * @endcode
   *
   * @subsection data_subsec Файлы данных
   * Приложение сохраняет данные транзакций в:
   * @code
   * ./data/transactions.dat
   * @endcode
   *
   * @section build_docs_sec Сборка документации
   * Для генерации документации необходимо:
   *
   * 1. Установить Doxygen и Graphviz
   * 2. Выполнить следующие команды:
   * @code{.sh}
   * # Создать директорию сборки
   * mkdir build
   * cd build
   *
   * # Сгенерировать Makefile/решение
   * cmake ..
   *
   * # Собрать документацию
   * cmake --build . --target docs
   * @endcode
   *
   * Либо использовать альтернативный вариант:
   * @code{.sh}
   * # Прямой вызов Doxygen
   * doxygen docs/Doxyfile
   * @endcode
   *
   * Сгенерированная документация будет доступна в:
   * @code
   * ./docs/html/index.html
   * @endcode
   *
   * @section arch_sec Архитектурная схема
   * Основные компоненты системы и их взаимосвязи:
   * @dot
   * digraph architecture {
   *   node [shape=component, fontname="Arial"];
   *   rankdir=TB;
   *
   *   // Ядро системы
   *   FinanceCore [label="FinanceCore\n(Основной класс)"];
   *
   *   // Основные модули
   *   Account [label="Account\n(Управление счетами)"];
   *   Transaction [label="Transaction\n(Транзакции)"];
   *   Statistics [label="Statistics\n(Аналитика)"];
   *   FileManager [label="FileManager\n(Сохранение данных)"];
   *
   *   // Связи
   *   FinanceCore -> Account;
   *   FinanceCore -> Transaction;
   *   FinanceCore -> Statistics;
   *   FinanceCore -> FileManager;
   *   Account -> Transaction [label="Содержит"];
   *
   *   // Группировка
   *   {rank=same; Account; Transaction}
   * }
   * @enddot
   *
   * @section conventions_sec Соглашения
   * - Все пути указаны относительно корня проекта
   * - Для сборки требуется C++17
   * - Документация генерируется через Doxygen 1.9+
   */