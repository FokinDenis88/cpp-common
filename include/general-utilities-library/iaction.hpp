#ifndef IACTION_HPP
#define IACTION_HPP

namespace util {

	/**
	* Abstract. Callable object, that invoke some action - function returning void.
	* It may be lambda, std::function, std::bind, functor, free functions, functions members.
	*/
	class IAction {
	protected:
		IAction() = default;
		IAction(const IAction&) = delete; // polymorphic guard. class suppress copy/move C.67
		IAction& operator=(const IAction&) = delete;
		IAction(IAction&&) noexcept = delete;
		IAction& operator=(IAction&&) noexcept = delete;
	public:
		virtual ~IAction() = default;

		/** Call action without return value. */
		virtual void operator()() = 0;
	};

} // !namespace util

#endif // !IACTION_HPP
