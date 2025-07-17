#ifndef TYPE_TRAITS_HPP
#define TYPE_TRAITS_HPP

namespace util {

//================Check of args count===============================================

	//template<typename FnT>
	//struct args_count {
	//private:
	//	using Functor = std::decay_t<FnT>;
	//	using Signature = std::function<Functor>;

	//	template<typename... Ts>
	//	static constexpr std::size_t count_impl(Signature(Ts...)) { return sizeof...(Ts); }

	//public:
	//	static constexpr std::size_t value = count_impl(Functor());
	//};

	//// Проверка корректности кортежа
	//template<typename MemFn, typename Tuple>
	//inline constexpr bool is_valid_tuple_for_memfn_v = []() {
	//	using FnT = std::decay_t<MemFn>;
	//	constexpr size_t expected_args_count = args_count<FnT>::value;
	//	constexpr size_t actual_args_count = std::tuple_size_v<std::decay_t<Tuple>>;
	//	return expected_args_count == actual_args_count;
	//	}();



	/*template<class PtrT>
	concept SmartPointer = requires(PtrT p) {
		requires std::same_as<std::remove_cvref_t<PtrT>, std::unique_ptr<typename PtrT::element_type>> ||
	std::same_as<std::remove_cvref_t<PtrT>, std::shared_ptr<typename PtrT::element_type>> ||
		std::same_as<std::remove_cvref_t<PtrT>, std::weak_ptr<typename PtrT::element_type>>;
	};*/

	//// Проверяем, принадлежит ли аргумент одному из указанных умных указателей
	//template<class PtrT>
	//constexpr bool is_smart_pointer() noexcept {
	//	return std::is_same_v<std::remove_cvref_t<PtrT>, std::unique_ptr<typename PtrT::element_type>> ||
	//		std::is_same_v<std::remove_cvref_t<PtrT>, std::shared_ptr<typename PtrT::element_type>> ||
	//		std::is_same_v<std::remove_cvref_t<PtrT>, std::weak_ptr<typename PtrT::element_type>>;
	//}

	//// Обобщённая функция с ограничением на типы
	//template<typename SomeT>
	//auto process(TTi<SomeT>& ptr) -> std::enable_if_t<is_smart_pointer<TTi<SomeT>>::value, void> {
	//	// Тело вашей функции
	//	std::cout << "Processing pointer of type: " << typeid(ptr).name() << "\\n";
	//}


	template<typename T>
	concept Dereferencable = requires(T t) {
		{ *t };            // Требует наличия оператора *
		{ t.operator->() }; // Требует наличия оператора ->
	};


	/**
	* Pointer to not const member function.
	* Every member function has first implicit parameter this pointer to object.
	*
	* @tparam ReturnT		The return type of member function.
	* @tparam ObjectT		T of object, for which member function will be called.
	* @tparam ArgsTs			Arguments for member function call, all except this pointer.
	*/
	template<typename ReturnT, typename ObjectT, typename... ArgsTs>
	using MemberFnPtr = ReturnT(ObjectT::*)(ArgsTs...);

	/**
	* Pointer to const member function.
	* Every member function has first implicit parameter this pointer to object.
	*
	* @tparam ReturnT		The return type of member function.
	* @tparam ObjectT		T of object, for which member function will be called.
	* @tparam ArgsTs			Arguments for member function call, all except this pointer.
	*/
	template<typename ReturnT, typename ObjectT, typename... ArgsTs>
	using ConstMemberFnPtr = ReturnT(ObjectT::*)(ArgsTs...) const;

	// TODO: Concept IsSmartPointer or IsDereferencable



	// Helper structure to simplify logic extraction of element type from different pointer types
	template<class PtrT>
	struct GetPointerTraits {
		using ElementT = typename std::remove_cvref_t<PtrT>::element_type;
	};

	// Specialization for raw pointers
	template<class T>
	struct GetPointerTraits<T*> {
		using ElementT = T;
	};

	template<typename MemberFnPtrT>
	struct MemFnPtrTrait {
		static_assert(false, "MemberFnPtrT must be pointer to member function.");
		using ReturnT = void;
		using ObjectT = void;
		using ArgsT = std::tuple<>;
	};
	// There were some problems with template parameter deduction.
	// If MemberFnPtrT is pointer to member function, then real deduction thinks, that it is pointer to global function.
	//
	// template<typename MemFnPtrT, typename ClassT>
	// void Func1(MemFnPtrT ClassT::* mem_fn) {
	// MemFnPtrT - type is pointer to global function. decltype(mem_fn) - type is pointer to member function.

	/** Traits (ReturnT, ObjectT, ArgsTs) for pointer to member function. */
	template<typename ReturnTrait, typename ObjectTrait, typename... ArgsTraits>
	struct MemFnPtrTrait<ReturnTrait(ObjectTrait::*)(ArgsTraits...)> {
		using ReturnT = ReturnTrait;
		using ObjectT = ObjectTrait;
		using ArgsT = std::tuple<ArgsTraits...>;
	};

	/** Traits (ReturnT, ObjectT, ArgsTs) for pointer to member function. */
	template<typename ReturnTrait, typename ObjectTrait, typename... ArgsTraits>
	struct MemFnPtrTrait<ReturnTrait(ObjectTrait::*)(ArgsTraits...) const> {
		using ReturnT = ReturnTrait;
		using ObjectT = ObjectTrait;
		using ArgsT = std::tuple<ArgsTraits...>;
	};


	/** Base template, set value = false */
	template<typename FnPtr>
	struct HasMemberFn : public std::false_type {};

	/** Specialization of non const member function */
	template<typename Ret, typename Class, typename... Args>
	struct HasMemberFn<Ret(Class::*)(Args...)> : std::integral_constant<bool,
		std::is_same_v<Ret, decltype((((Class*) nullptr)->*(static_cast<Ret(Class::*)(Args...)>(nullptr)))(
			std::declval<Args>()...))
		>> {
	};

	/** Specialization of const member function */
	template<typename Ret, typename Class, typename... Args>
	struct HasMemberFn<Ret(Class::*)(Args...) const> : std::integral_constant<bool,
		std::is_same_v<Ret, decltype((((const Class*) nullptr)->*(static_cast<Ret(Class::*)(Args...) const>(nullptr)))(
			std::declval<Args>()...))
		>> {
	};

	template<typename FnPtr>
	inline constexpr bool HasMemberFn_v = HasMemberFn<FnPtr>::value;


} // !namespace util



namespace help {

	/*
	* Чтобы вывести типы ArgsT... и Traits::ArgsT, мы можем воспользоваться возможностями метапрограммирования C++. Для начала нам потребуется шаблонная структура, которая сможет выводить типы аргументов шаблона. Например, такая простая структура позволит напечатать список типов через запятую:
	*/

	//// Вспомогательная структура для вывода типа
	//template<class T>
	//struct TypePrinter {
	//	static void print() { std::cout << typeid(T).name(); }
	//};

	//// Специализация для вывода списка типов через запятую
	//template<class Head, class... Tail>
	//struct TypeListPrinter {
	//	static void print() {
	//		TypePrinter<Head>::print();
	//		if constexpr (sizeof...(Tail)) {
	//			std::cout << ", ";
	//			TypeListPrinter<Tail...>::print();
	//		}
	//	}
	//};

	//// Завершение рекурсии
	//template<class Last>
	//struct TypeListPrinter<Last> {
	//	static void print() {
	//		TypePrinter<Last>::print();
	//	}
	//};

	//// Структура для распечатывания одного типа
	//template<class T>
	//void PrintType(const char* prefix = "") {
	//	std::cout << prefix << ": ";
	//	TypePrinter<T>::print();
	//	std::cout << '\\n';
	//}

	//// Функция для печати набора типов
	//template<class... Ts>
	//void PrintTypes(const char* listName = "") {
	//	std::cout << listName << ": ";
	//	TypeListPrinter<Ts...>::print();
	//	std::cout << "\\n";
	//}

}

#endif // !TYPE_TRAITS_HPP
