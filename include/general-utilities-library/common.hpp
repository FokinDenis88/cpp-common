#ifndef COMMON_HPP
#define COMMON_HPP

namespace common {
	template<typename T>
	class Cloneable {
	public:
		virtual std::unique_ptr<T> clone() const = 0;
	};


} // !namespace common

#endif // !COMMON_HPP
