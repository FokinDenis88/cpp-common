#ifndef UTILITY_HPP
#define UTILITY_HPP

namespace util {
	template<typename T>
	class Cloneable {
	public:
		virtual std::unique_ptr<T> clone() const = 0;
	};


} // !namespace util

#endif // !UTILITY_HPP
