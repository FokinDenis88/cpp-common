#ifndef TUPLE_HPP
#define TUPLE_HPP

#include <tuple>
#include <concepts>

/** C++ General Support Library */
namespace common {

	/** Implementation of ApplyTuple. */
	template<typename FuncType, typename... TupleArgsTypes, size_t... index>
	//template<typename FuncType, size_t... index, typename... TupleElemTypes>
	requires std::is_invocable_v<FuncType, TupleArgsTypes...>
	inline void ApplyTupleElement(FuncType&& func_obj,
								  std::tuple<TupleArgsTypes&&...>& tuple_p,
								  std::index_sequence<index...>) {
		func_obj(std::get<index>(tuple_p)...);
	}

	/** Invoke callable object with tuple args. Needs C++ 11. */
	template<typename FuncType, typename... TupleArgsTypes>
	requires std::is_invocable_v<FuncType, TupleArgsTypes...>
	inline void ApplyTuple(FuncType&& func_obj, std::tuple<TupleArgsTypes&&...>& tuple_p) {
		// Can't make const args tuple
		ApplyTupleElement(std::forward<FuncType>(func_obj),
					tuple_p,
					std::make_index_sequence<std::tuple_size_v<std::tuple<TupleArgsTypes&&...>>>{});
	}




	/** Variadic Function Call */
	// Convert string data to data of field in data table
	/*template<const size_t Index, typename... ColumnT>
	inline void ConvertDataOnIndexTuple(std::tuple<ColumnT...>& buffer_tuple, std::string(&buffer_str)[]) {
		using TableRow = std::tuple<ColumnT...>;
		if constexpr (std::is_same_v<std::tuple_element_t<Index, TableRow>, std::string>) {
			std::get<Index>(buffer_tuple) = buffer_str[Index];
		}
		else if constexpr (std::is_same_v<std::tuple_element_t<Index, TableRow>, int>) {
			std::get<Index>(buffer_tuple) = std::stoi(buffer_str[Index]);
		}
		else if constexpr (std::is_same_v<std::tuple_element_t<Index, TableRow>, float>) {
			std::get<Index>(buffer_tuple) = std::stof(ReplaceComma(buffer_str[Index]));
		}
		else if constexpr (std::is_same_v<std::tuple_element_t<Index, TableRow>, double>) {
			std::get<Index>(buffer_tuple) = std::stod(ReplaceComma(buffer_str[Index]));
		}
	}

	template<typename... ColumnT, size_t... Indexes>
	inline void ConvertDataTuple(std::tuple<ColumnT...>& buffer_tuple, std::string(&buffer_str)[], std::index_sequence<Indexes...>) {
		(ConvertDataOnIndexTuple<Indexes>(buffer_tuple, buffer_str), ...);
	}*/

} // !namespace common

#endif // !TUPLE_HPP
