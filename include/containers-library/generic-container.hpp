#ifndef GENERIC_CONTAINER_HPP
#define GENERIC_CONTAINER_HPP


#include <algorithm>	// remove_if
#include <execution>	// execution policies
#include <type_traits>	// is_same_v
#include <utility>		// forward

// Container Types
#include <forward_list>

#include <set>				// Find
#include <unordered_set>	// Find
#include <map>				// Find
#include <unordered_map>	// Find

// TODO: Too many container types. Too complex

// TODO: add specific functions for that containers, for which solution is not optimal.
// F.e. std::find is not optimal for std::unordered_set. Use specific container functions.


/** Generic container processing. One function for all container types. */
namespace generic { // Generic Container Element Modification

	/**
	* Add (emplace, push or insert) element from any type of container.
	*
	* Complexity: unordered_set, unordered_map = O(1).
	* set, map                                 = O(log n).
	* all other containers                     = O(n)
	*
	* Mutex: read
	*/
	template<typename ContainerT, typename ExecPolicyT>
	inline decltype(auto) Find(const ContainerT& container,
								const typename ContainerT::value_type& value,
								ExecPolicyT policy = std::execution::seq) {
		using value_type = typename ContainerT::value_type;

		if constexpr (
				std::is_same_v<std::remove_cvref_t<ContainerT>, std::unordered_set<value_type>>
				|| std::is_same_v<std::remove_cvref_t<ContainerT>, std::set<value_type>>
				|| std::is_same_v<std::remove_cvref_t<ContainerT>, std::unordered_map<value_type>>
				|| std::is_same_v<std::remove_cvref_t<ContainerT>, std::map<value_type>>
				//|| std::is_same_v<std::remove_cvref_t<ContainerT>, std::multiset<value_type>>
				//|| std::is_same_v<std::remove_cvref_t<ContainerT>, std::multimap<value_type>>
			)
		{ // associative containers have special find() method
			return container.find(value);
														//unordered_set, unordered_map			O(1)
														// set, map								O(log n)
		}
		//else if constexpr (std::is_same_v<std::remove_cvref_t<ContainerT>, std::vector<bool>>) {
		//	// Специальный случай для vector<bool>. Стандартные алгоритмы работают медленно.
		//	auto pos = static_cast<size_t>(container.size());
		//	for (size_t i = 0; i < pos; ++i) {
		//		if (container[i]) break;
		//	}
		//	return pos != container.size();

		//}
		else { // All other types of containers
			return std::find(policy, container.begin(), container.end(), value);				// O(n)
		}
	}


	/**
	* Add (emplace, push or insert) element from any type of container.
	*
	* Complexity: O(1)
	* Mutex: write
	*/
	template<typename ContainerT>
	inline void AddElement(ContainerT& container, typename ContainerT::value_type&& value) {
		using value_type = typename ContainerT::value_type;

		if constexpr (std::is_same_v<std::remove_cvref_t<ContainerT>,
						std::forward_list<value_type>>) { // forward_list
			container.emplace_front(std::forward<value_type>(value));			// O(1)
		} else { // All other types of containers, except forward_list
			container.emplace(std::forward<value_type>(value));					// O(1)
			// emplace_back is better for  vector, deque and list
		}
	}

	/**
	* Remove element from any type of container.
	*
	* Complexity: O(n)
	* Mutex: write
	*
	* @param predicate		for remove_if operation
	*/
	template<typename ContainerT, typename ExecPolicyT>
	inline void RemoveIf(ContainerT& container,
								auto predicate,
								ExecPolicyT policy = std::execution::seq) {
		if (container.empty()) { return; } // Precondition

		if constexpr (std::is_same_v<std:: remove_cvref_t<ContainerT>,
						std::forward_list<typename ContainerT::value_type>>) { // for Forward_list
			container.remove_if(predicate); // erase-remove idiom is not for forward_list		// O(n)
			// std::remove_if is not working with forward_list
		} else { // All other containers
			container.erase(std::remove_if(policy, container.begin(),
											container.end(), predicate), container.end());		// O(n)
		}
		// TODO: erase-remove idiom can't be used for set, map, unordered_set, unordered_map, cause of elements can't be moved.
		// Cause of structure of containers.
		//
		// Also can't be used for stack, queue.
	}


	/**
	* Remove value from any type of container.
	*
	* Complexity: O(n)
	* Mutex: write
	*
	* @param predicate for remove_if operation
	*/
	template<typename ContainerT, typename ExecPolicyT>
	inline void EraseFirst(ContainerT& container,
								const typename ContainerT::value_type& value,
								ExecPolicyT policy = std::execution::seq) {
		using value_type = typename ContainerT::value_type;
		if (container.empty()) { return; } // Precondition

		if constexpr (std::is_same_v<std::remove_cvref_t<ContainerT>,
									std::forward_list<value_type>>) { // for Forward_list
			container.remove(value);																	// O(n)
		} else { // All other containers
			container.erase(Find(container, value, policy));										// O(n)
		}
	}

	/**
	* Remove element of container by iterator.
	* All types of containers, except forward_list, cause it needs erase_after.
	*
	* Complexity: O(n)
	* Mutex: write
	*
	* @param it			iterator to erasable element
	* @return			iterator to the next element from erased element
	*/
	template<typename ContainerT>
	inline auto EraseIt(ContainerT& container,
						typename ContainerT::iterator it)
			-> decltype(container.end())
	{
		using value_type = typename ContainerT::value_type;

		if constexpr (!std::is_same_v<std::remove_cvref_t<ContainerT>,
									std::forward_list<value_type>>) { // all except forward_list
			if (it != container.end()) {
				return container.erase(it);
			}
		}
		return container.end();
	}

	/**
	* Remove element of forward_list after iterator.
	*
	* Complexity: O(n)
	* Mutex: write
	*
	* @param it			iterator to element previous to erasable
	* @return			iterator to the next element from erased element
	*/
	template<typename ValueT>
	inline auto EraseIt(std::forward_list<ValueT>& container,
						typename std::forward_list<ValueT>::iterator it)
			-> decltype(container.end())
	{
		if (it != container.before_begin()) {
			return container.erase_after(it);
		}
		return container.end();
	}

} // !namespace generic

#endif // !GENERIC_CONTAINER_HPP

/*
	1. Добавление элемента

		Ты уже реализовал этот метод, но для полноты картины можно добавить ещё пару нюансов, например, использование emplace_back() для контейнеров, поддерживающих вставку в конец(таких как vector, deque, list).

		template<typename ContainerT>
	inline void AddElement(ContainerT& container, typename ContainerT::value_type&& value) {
		using value_type = typename ContainerT::value_type;

		if constexpr (std::is_same_v<std::remove_cvref_t<ContainerT>, std::forward_list<value_type>>) {
			container.emplace_front(std::forward<value_type>(value)); // O(1)
		}
		else if constexpr (std::is_same_v<std::remove_cvref_t<ContainerT>, std::stack<value_type>> ||
			std::is_same_v<std::remove_cvref_t<ContainerT>, std::queue<value_type>>) {
			container.push(std::forward<value_type>(value)); // Специфичные push() для stack и queue
		}
		else {
			container.emplace_back(std::forward<value_type>(value)); // O(1) для большинства контейнеров
		}
	}

	2. Удаление одного элемента

		У тебя уже есть RemoveIf, теперь сделаем аналогичную функцию для удаления отдельного элемента :

	template<typename ContainerT>
	inline void RemoveElement(ContainerT& container, const typename ContainerT::value_type& value) {
		if constexpr (std::is_same_v<std::remove_cvref_t<ContainerT>, std::forward_list<typename ContainerT::value_type>>) {
			container.remove(value); // Оптимизированная версия для forward_list
		}
		else {
			auto pos = std::find(container.begin(), container.end(), value);
			if (pos != container.end()) {
				container.erase(pos); // Универсальное удаление
			}
		}
	}

	3. Получение первого и последнего элемента

		Для некоторых контейнеров(например, стека) доступен прямой доступ к первым и последним элементам, для других нужно обходить контейнер :

	template<typename ContainerT>
	inline auto GetFirstElement(const ContainerT& container) {
		static_assert(!std::is_same_v<std::remove_cvref_t<ContainerT>, std::stack<typename ContainerT::value_type>>,
			"Stack doesn't have a first element.");
		return *container.begin();
	}

	template<typename ContainerT>
	inline auto GetLastElement(const ContainerT& container) {
		static_assert(!std::is_same_v<std::remove_cvref_t<ContainerT>, std::stack<typename ContainerT::value_type>>,
			"Stack doesn't have a last element.");
		return *(--container.end());
	}

	4. Изменение порядка элементов

		Иногда полезно поменять порядок элементов в контейнере.Например, обратный порядок :

	template<typename ContainerT>
	inline void ReverseElements(ContainerT& container) {
		if constexpr (std::is_same_v<std::remove_cvref_t<ContainerT>, std::forward_list<typename ContainerT::value_type>>) {
			container.reverse(); // Прямая поддержка reverse()
		}
		else {
			std::reverse(container.begin(), container.end()); // Общий случай
		}
	}

	5. Копирование и перемещение элементов

		Поддержка копирования и перемещения элементов из одного контейнера в другой :

	// Перемещаем содержимое src в dst
	template<typename SrcContainer, typename DstContainer>
	inline void MoveElements(SrcContainer && src, DstContainer & dst) {
		using value_type = typename SrcContainer::value_type;

		if constexpr (std::is_same_v<std::remove_cvref_t<SrcContainer>, std::forward_list<value_type>>) {
			for (auto&& elem : src) {
				dst.emplace_front(std::move(elem)); // move для списка
			}
		}
		else {
			std::move(src.begin(), src.end(), std::back_inserter(dst)); // Общее перемещение
		}
	}

	6. Расширенный find

		Можно сделать обобщённую версию поиска, которая автоматически выбирает оптимальный способ нахождения элемента :

	template<typename ContainerT, typename ValueT>
	auto FindElement(const ContainerT& container, const ValueT& value) {
		if constexpr (std::is_base_of_v<std::associative_container_tag, typename ContainerT::container_category>) {
			return container.find(value); // Использование специализированного поиска для map/set/unordered_map
		}
		else {
			return std::find(container.begin(), container.end(), value); // Обычный find для других контейнеров
		}
	}

	7. Сортируемость контейнера

		Это важная операция, которую можно реализовать универсально :

	template<typename ContainerT>
	inline void SortContainer(ContainerT& container) {
		if constexpr (!std::is_same_v<std::remove_cvref_t<ContainerT>, std::forward_list<typename ContainerT::value_type>>) {
			std::sort(container.begin(), container.end()); // Работает для обычных контейнеров
		}
		else {
			container.sort(); // Forward_list сам сортирует себя
		}
	}

	8. Размер контейнера

		Реализуем безопасный способ определения размера :

	template<typename ContainerT>
	inline size_t SizeOfContainer(const ContainerT & container) {
		return container.size(); // Все контейнеры поддерживают size()
	}

	Итоговый набор полезных методов :

	Теперь у тебя есть базовый инструментарий для работы с любыми контейнерами STL, включая :

		добавление и удаление элементов,
		получение первых и последних элементов,
		изменение порядка элементов,
		сортировку и поиск,
		перенос содержимого между контейнерами.
		*/




		/*Компоненты Boost для построения обобщённого механизма работы с контейнерами :

		1. Boost.TTraits
		Позволяют анализировать типы контейнеров и их свойства.Используя метапрограммируемые техники(SFINAE, traits), можно определять характеристики контейнеров(например, является ли контейнер ассоциативным, поддерживает ли двунаправленную навигацию и т.д.).

		2. Boost.Utility
		Включает утилиты для удобства работы с типами и функциями.Например, классы enable_if и result_of помогают управлять компиляцией и специализациями шаблонов.

		3. Boost.ConceptCheck
		Предоставляет механизм проверок концептов(concept checking), позволяющий убедиться, что передаваемый контейнер соответствует необходимым требованиям(например, итерируемый, копируемый и т.д.).

		4. Boost.Move
		Реализует механизм перемещения(move semantics), полезный для эффективной работы с элементами контейнеров.

		5. Boost.Optional
		Используется для безопасного представления необязательного значения, которое может быть полезным при обработке пустых контейнеров или отсутствующих значений.

		6. Boost.Variant
		Помогает манипулировать разнотипными объектами в одном контейнере, предоставляя механизм безопасной диспетчеризации действий над ними.

		7. Boost.Functional / Hash
		Улучшенная реализация хэшей для ассоциативных контейнеров, позволяющая легко интегрировать собственные типы объектов в качестве ключей в hash - контейнерах.

		8. Boost.Algorithm
		Содержит широкий спектр специализированных алгоритмов для обработки последовательностей, многие из которых являются хорошими кандидатами для интеграции в обобщённое API для контейнеров.

		Пример общего подхода к созданию своего класса - обёртки для работы с любыми контейнерами :
		Допустим, ты хочешь собрать собственный класс - обёртку, обеспечивающий единый интерфейс для взаимодействия с любыми контейнерами :

		namespace MyLib {

			template<typename ContainerT>
			class GenericContainerProcessor {
			public:
				using value_type = typename ContainerT::value_type;

				explicit GenericContainerProcessor(ContainerT& cont) : m_container(cont) {}

				void addElement(value_type&& val) {
					AddElement(m_container, std::forward<value_type>(val));
				}

				void removeElement(const value_type& val) {
					RemoveElement(m_container, val);
				}

				auto begin() const noexcept { return m_container.begin(); }
				auto end() const noexcept { return m_container.end(); }

			private:
				ContainerT& m_container;
			};

		} // namespace MyLib


		Такой подход позволяет обернуть любой существующий контейнер в оболочку с единым интерфейсом, обеспечивая унификацию работы с ним вне зависимости от конкретной реализации.

		Заключение
		Хотя в Boost нет готового компонента уровня "GenericContainerProcessing", комбинируя различные части библиотеки, можно построить такой инструмент самому.Пользуясь средствами Boost для анализа типов, создания эффективных обработчиков контейнеров и предоставления удобных алгоритмов, ты сможешь достичь нужного результата.

		Таким образом, создав собственную систему поверх модулей Boost, можно добиться максимальной гибкости и эффективности при взаимодействии с самыми разнообразными контейнерами.

		*/
