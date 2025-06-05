#ifndef INTERFACE_MACROS_HPP
#define INTERFACE_MACROS_HPP

/** Macros for quick and correct Defining of abstract stateless interface. C++ Core Guidelines: C.67 C.21 */
#define INTERFACE_DEFINE(ClassName)								\
	protected:													\
		ClassName() = default;									\
		ClassName(const ClassName&) = delete;	/* C.67	C.21 Polymorphic suppress Copy & Move */	\
		ClassName& operator=(const ClassName&) = delete;		\
		ClassName(ClassName&&) noexcept = delete;				\
		ClassName& operator=(ClassName&&) noexcept = delete;	\
	public:														\
		virtual ~ClassName() = default;



#endif // !INTERFACE_MACROS_HPP
