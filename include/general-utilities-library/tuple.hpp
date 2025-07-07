#ifndef TUPLE_HPP
#define TUPLE_HPP

//#include <concepts> // for requires
#include <tuple>
//#include <memory>	// weak_ptr

//#include "metaprogramming-library/type-traits/type-traits.hpp"


/** C++ General Support Library */
namespace util {


} // !namespace util




namespace help{
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

}

#endif // !TUPLE_HPP
