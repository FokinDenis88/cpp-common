#ifndef FUNCTIONAL_HPP
#define FUNCTIONAL_HPP


#include <memory>
#include <type_traits> // conditional_t
#include <tuple>
#include <utility> // pair, forward

#include "metaprogramming-library/type-traits/type-traits.hpp"


namespace util {
	/*
	* ApplyMemFn
	* Summary:
	* This code provides flexible utilities to invoke member functions via pointers or weak pointers with arguments supplied as tuples.
	*
	* It handles various cases including:
	* Member functions returning void or non-void types.
	* Null or expired pointers.
	* Type safety through static assertions.
	* Efficient argument unpacking via index sequences.
	* This pattern is useful in callback systems, reflection-like mechanisms, or generic frameworks where functions are
	*		invoked dynamically with parameters stored in tuples.
	*/



	/** Return false value for Invoke Method function. */
	template<typename MemFnPtrT>
	inline auto InvokeMethodFalseReturn() noexcept
			->	std::conditional_t< !std::is_void_v<typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool, typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool> > {
		using ReturnT = MemFnPtrTrait<MemFnPtrT>::ReturnT;
		if constexpr (!std::is_void_v<ReturnT>) { return std::make_tuple(false, ReturnT{}); }
		else { return std::make_tuple(false); }
	}

	/** Invoke member function without any checks. */
	template<typename MemFnPtrT, typename ObjectT, typename... ArgsT>
	decltype(auto) InvokeMethodImpl(MemFnPtrT mem_fn, ObjectT&& object, ArgsT&&... args) {
		return (std::forward<ObjectT>(object).*mem_fn)(std::forward<ArgsT>(args)...);
	}

	/**
	* Invokes member function on concrete object using std::apply and tuple arguments.
	* Be careful with types of args. They must be equal and not implicitly converted to proper.
	* Or just disable types checking static assert.
	*
	* Preconditions:	object must exists.
	*					mem_fn mustn't be nullptr. mem_fn must point to exists member function of object.
	*					Number of arguments must be equal to number of arguments of member function.
	*					Ts of arguments must be proper.
	*
	* @param mem_fn			pointer to member function
	* @param object			object, on which member function will be called
	* @param args			arguments for member function call
	* @return				tuple(success flag, return value of member function call. Or Nothing = void return )
	*/
	template<typename MemFnPtrT, typename ObjectT, typename... ArgsT>
	inline auto InvokeMethod(MemFnPtrT mem_fn,
							ObjectT&& object,
							ArgsT&&... args)
			->	std::conditional_t< !std::is_void_v<typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool, typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool> >
	{
		using Traits = MemFnPtrTrait<MemFnPtrT>;
		using ReturnT = typename Traits::ReturnT;

		static_assert(std::is_same_v<std::remove_cvref_t<typename Traits::ObjectT>,
						std::remove_cvref_t<ObjectT> >,
						"ClassT of Pointer to member function must be equal to ObjectT.");
		static_assert(std::is_object_v< std::remove_reference_t<ObjectT> >,	// reference to object is not object
						"object: object must be object type. F.e. class, struct, union.");
		static_assert(std::is_member_function_pointer_v< std::remove_reference_t<MemFnPtrT> >,
						"mem_fn: Expected member function pointer.");
		static_assert(HasMemberFn_v<MemFnPtrT>,
						"The specified method does not exist in the given class.");
		static_assert(std::is_convertible_v<	std::tuple<ArgsT...>,
						std::remove_cvref_t<typename Traits::ArgsT> >,
						"Arguments types of member function pointer, and input arguments must be equal.");
		/*decltype(auto) a{ std::tuple<ArgsT...>() }; // To see types in debug
		typename Traits::ArgsT b{ };*/


		if (!mem_fn) { // precondition // TODO: check args count and types
			return InvokeMethodFalseReturn<MemFnPtrT>();
		}

		if constexpr (!std::is_void_v<ReturnT>) { // member function returns some type
			return std::make_tuple(true, InvokeMethodImpl(mem_fn, std::forward<ObjectT>(object),
															std::forward<ArgsT>(args)...));
		} else {
			InvokeMethodImpl(mem_fn, std::forward<ObjectT>(object), std::forward<ArgsT>(args)...);
			return std::make_tuple(true); // member function returns void type
		}
	}
	/*
	* TODO: ApplyMemFn can fail, so return std::optional or pair.
	*	But if return of mem_fn is void, then make_pair give error, cause it waits && value.
	*/


//====================InvokeMethodByPtr=============================================================

	/**
	* Invokes member function on raw pointer to object using std::apply and tuple arguments.
	*
	* @param mem_fn			pointer to member function
	* @param object_ptr		pointer to object, on which member function will be called
	* @param args			arguments for member function call
	* @return				tuple(success flag, return value of member function call. Or Nothing = void return )
	*/
	template<typename MemFnPtrT, typename ObjectT, typename... ArgsT>
	inline auto InvokeMethodByPtr(MemFnPtrT mem_fn,
									ObjectT* object_ptr,
									ArgsT&&... args)
			->	std::conditional_t< !std::is_void_v<typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool, typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool> >
	{ // can be used, cause dereference * operation
		if (!object_ptr) { // precondition // TODO: C++17 replace to optional
			return InvokeMethodFalseReturn<MemFnPtrT>();
		}

		return InvokeMethod(mem_fn, *object_ptr, std::forward<ArgsT>(args)...);
	}

	/**
	* Invokes member function on unique_ptr pointer to object using std::apply and tuple arguments.
	*
	* @param mem_fn			pointer to member function
	* @param object_ptr		pointer to object, on which member function will be called
	* @param args			arguments for member function call
	* @return				tuple(success flag, return value of member function call. Or Nothing = void return )
	*/
	template<typename MemFnPtrT, typename ObjectT, typename... ArgsT>
	inline auto InvokeMethodByPtr(MemFnPtrT mem_fn,
									const std::unique_ptr<ObjectT>& object_ptr,
									ArgsT&&... args)
			->	std::conditional_t< !std::is_void_v<typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool, typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool> >
	{ // can be used, cause dereference * operation
		return InvokeMethodByPtr(mem_fn, object_ptr.get(), std::forward<ArgsT>(args)...);
	}

	/**
	* Invokes member function on shared_ptr pointer to object using std::apply and tuple arguments.
	*
	* @param mem_fn			pointer to member function
	* @param object_ptr		pointer to object, on which member function will be called
	* @param args			arguments for member function call
	* @return				tuple(success flag, return value of member function call. Or Nothing = void return )
	*/
	template<typename MemFnPtrT, typename ObjectT, typename... ArgsT>
	inline auto InvokeMethodByPtr(MemFnPtrT mem_fn,
								std::shared_ptr<ObjectT> object_ptr,
								ArgsT&&... args)
			->	std::conditional_t< !std::is_void_v<typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool, typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool> >
	{ // can be used, cause dereference * operation
		return InvokeMethodByPtr(mem_fn, object_ptr.get(), std::forward<ArgsT>(args)...);
	}

	/**
	* Invokes member function on weak_ptr pointer to object using std::apply and tuple arguments.
	*
	* @param mem_fn			pointer to member function
	* @param object_ptr		pointer to object, on which member function will be called
	* @param args			arguments for member function call
	* @return				tuple(success flag, return value of member function call. Or Nothing = void return )
	*/
	template<typename MemFnPtrT, typename ObjectT, typename... ArgsT>
	inline auto InvokeMethodByPtr(MemFnPtrT mem_fn,
									std::weak_ptr<ObjectT> object_ptr,
									ArgsT&&... args)
			->	std::conditional_t< !std::is_void_v<typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool, typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool> >
	{ // can be used, cause dereference * operation
		if (auto object_shared_ptr = object_ptr.lock()) {
			return InvokeMethodByPtr(mem_fn, object_shared_ptr.get(), std::forward<ArgsT>(args)...);
		} // if expired
		return InvokeMethodFalseReturn<MemFnPtrT>();
	}


//======================Apply free function. For c++ 11==============================================

	/** Implementation of ApplyTuple. */
	template<typename FuncType, typename... TupleArgsTypes, size_t... index>
	//template<typename FuncType, size_t... index, typename... TupleElemTypes>
	requires std::is_invocable_v<FuncType, TupleArgsTypes...>
	inline void ApplyImpl(FuncType&& func_obj,
						std::tuple<TupleArgsTypes&&...>& tuple_p,
						std::index_sequence<index...>) {
		func_obj(std::get<index>(tuple_p)...);
	}

	/** Invoke callable object with tuple args. Needs C++ 11. */
	template<typename FuncType, typename... TupleArgsTypes>
	requires std::is_invocable_v<FuncType, TupleArgsTypes...>
	inline void Apply(FuncType&& func_obj, std::tuple<TupleArgsTypes&&...>& tuple_p) {
		// Can't make const args tuple
		ApplyImpl(std::forward<FuncType>(func_obj), tuple_p,
				std::make_index_sequence<std::tuple_size_v<std::tuple<TupleArgsTypes&&...>>>{});
	}


//=======================Apply Method with tuple of arguments==============================================================

	/** Implementation of ApplyMethod. */
	template<typename MemFnPtrT, typename ObjectT, typename ArgsTupleT, size_t... I>
	inline auto ApplyMethodImpl(MemFnPtrT mem_fn, ObjectT&& object, ArgsTupleT&& args,
								std::index_sequence<I...>)
			->	std::conditional_t< !std::is_void_v<typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool, typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool> >
	{ // tuple unpacking give better performance, than lambda.
		return InvokeMethod(mem_fn, std::forward<ObjectT>(object),
							std::get<I>( std::forward<ArgsTupleT>(args) )...);
	}

	/**
	* Invokes member function on object with tuple of arguments.
	*
	* @param mem_fn			pointer to member function
	* @param object			object, on which member function will be called
	* @param args			tuple of arguments for member function call
	* @return				tuple(success flag, return value of member function call. Or Nothing = void return )
	*/
	template<typename MemFnPtrT, typename ObjectT, typename ArgsTupleT>
	inline auto ApplyMethod(MemFnPtrT mem_fn, ObjectT&& object, ArgsTupleT&& args)
			->	std::conditional_t< !std::is_void_v<typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool, typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool> >
	{
		using Indices = std::make_index_sequence< std::tuple_size_v< std::remove_cvref_t<ArgsTupleT> > >;
		return ApplyMethodImpl(mem_fn, std::forward<ObjectT>(object),
								std::forward<ArgsTupleT>(args), Indices{});
	}

	/**
	* Invokes member function on weak_ptr pointer to object using std::apply and tuple arguments.
	*
	* @param mem_fn			pointer to member function
	* @param object_ptr		pointer to object, on which member function will be called
	* @param args			tuple of arguments for member function call
	* @return				tuple(success flag, return value of member function call. Or Nothing = void return )
	*/
	template<typename MemFnPtrT, typename ObjectT, typename ArgsTupleT>
	inline auto ApplyMethodByPtr(MemFnPtrT mem_fn,
								ObjectT* object_ptr,
								ArgsTupleT&& args)
			->	std::conditional_t< !std::is_void_v<typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool, typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool> >
	{
		if (!object_ptr) { // precondition // TODO: C++17 replace to optional
			return InvokeMethodFalseReturn<MemFnPtrT>();
		}
		return ApplyMethod(mem_fn, *object_ptr, std::forward<ArgsTupleT>(args));
	}

	/**
	* Invokes member function on weak_ptr pointer to object using std::apply and tuple arguments.
	*
	* @param mem_fn			pointer to member function
	* @param object_ptr		pointer to object, on which member function will be called
	* @param args			tuple of arguments for member function call
	* @return				tuple(success flag, return value of member function call. Or Nothing = void return )
	*/
	template<typename MemFnPtrT, typename ObjectT, typename ArgsTupleT>
	inline auto ApplyMethodByPtr(MemFnPtrT mem_fn,
								std::weak_ptr<ObjectT> object_ptr,
								ArgsTupleT&& args)
			->	std::conditional_t< !std::is_void_v<typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool, typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool> >
	{
		if (auto shared_locked = object_ptr.lock()) {
			return ApplyMethod(mem_fn, *shared_locked, std::forward<ArgsTupleT>(args));
		} // if expired
		return InvokeMethodFalseReturn<MemFnPtrT>();
	}

//=======================ApplyMethodPart==================================================================

	/** Implementation of ApplyMethod. */
	template<typename MemFnPtrT, typename ObjectT, typename... ArgsBeforeT,
			size_t... I, typename ArgsTupleT, typename... ArgsAfterT>
	inline auto ApplyMethodPartImpl(MemFnPtrT mem_fn, ObjectT&& object,
									ArgsBeforeT&&... args_before,
									std::index_sequence<I...>,	// placed here for better types defining
									ArgsTupleT&& args_tuple,
									ArgsAfterT&&... args_after)
			->	std::conditional_t< !std::is_void_v<typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool, typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool> >
	{ // tuple unpacking give better performance, than lambda.
		return InvokeMethod(mem_fn, std::forward<ObjectT>(object),
                            std::forward<ArgsBeforeT>(args_before)...,
							std::get<I>(std::forward<ArgsTupleT>(args_tuple))...,
							std::forward<ArgsAfterT>(args_after)...);
	}

	/**
	* Invokes member function on object with tuple of arguments.
	*
	* @param mem_fn			pointer to member function
	* @param object			object, on which member function will be called
	* @param args			tuple of arguments for member function call
	* @return				tuple(success flag, return value of member function call. Or Nothing = void return )
	*/
	template<typename MemFnPtrT, typename ObjectT, typename ArgsTupleT, typename... ArgsBeforeT, typename... ArgsAfterT>
	inline auto ApplyMethodPart(MemFnPtrT mem_fn, ObjectT&& object,
								ArgsBeforeT&&... args_before,
								ArgsTupleT&& args_tuple,
								ArgsAfterT&&... args_after)
			->	std::conditional_t< !std::is_void_v<typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool, typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
				std::tuple<bool> >
	{
		using Indices = std::make_index_sequence<std::tuple_size_v<ArgsTupleT>>;
		return ApplyMethodPartImpl(mem_fn, std::forward<ObjectT>(object),
									std::forward<ArgsBeforeT>(args_before)...,
									Indices{},	// for better type defining by compiler
									std::forward<ArgsTupleT>(args_tuple),
									std::forward<ArgsAfterT>(args_after)...);
		// tuple_cat realization - concatenation of all args
	}

} // !namespace util



namespace help {
	/** T is used for function execution. */
	//using Task = std::function<void()>;

	/** Alias of Task. */
	//using Action = Task;

} // !namespace help



/** Unused code. */
namespace deprecated {
	//=================Different variants of realization call member function=================

	/** Invokes member function on concrete object using call on indside std::function object. */
	//template<typename MemFnPtrT, typename ObjectT, typename... ArgsTs>
	//[[deprecated]]
	//inline void InvokeMemFn(MemFnPtrT mem_fn,
	//						ObjectT&& object,
	//						ArgsTs&&... args) {
	//	if (mem_fn) { // precondition
	//		std::function<void()> {
	//			[&](ArgsTs&&... params) {
	//				// Object must have mem_fn in its definition.
	//				return (std::forward<ObjectT>(object).*mem_fn)(std::forward<ArgsTs>(params)...);
	//				} }();
	//	}
	//};

	//template<typename MemFnPtrT, typename ObjectT, typename... ArgsTs>
	//[[deprecated]]
	//inline auto ApplyObjMemFn_lambda(MemFnPtrT mem_fn,
	//								ObjectT&& object,
	//								std::tuple<ArgsTs...>&& args)
	//		-> std::conditional_t< !std::is_void_v<typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
	//		std::tuple<bool, typename MemFnPtrTrait<MemFnPtrT>::ReturnT>,
	//		std::tuple<bool> >
	//{
	//	using Traits = MemFnPtrTrait<MemFnPtrT>;
	//	using ReturnT = typename Traits::ReturnT;
	//	/*using ApplyObjReturnT = std::conditional_t< !std::is_void_v<ReturnT>,
	//													std::tuple<bool, ReturnT>,
	//													std::tuple<bool> >;*/

	//	static_assert(std::is_same_v<std::remove_cvref_t<typename Traits::ObjectT>,
	//		std::remove_cvref_t<ObjectT> >,
	//		"ClassT of Pointer to member function must be equal to ObjectT.");
	//	static_assert(std::is_object_v< std::remove_reference_t<ObjectT> >,	// reference to object is not object
	//		"object: object must be object type. F.e. class, struct, union.");
	//	static_assert(std::is_member_function_pointer_v< std::remove_reference_t<MemFnPtrT> >,
	//		"mem_fn: Expected member function pointer.");
	//	static_assert(HasMemberFn_v<MemFnPtrT>,
	//		"The specified method does not exist in the given class.");

	//	if (!mem_fn) { // precondition // TODO: check args count and types
	//		if constexpr (!std::is_void_v<ReturnT>) { return std::make_tuple(false, ReturnT{}); }
	//		else { return std::make_tuple(false); }
	//	}

	//	auto call_lambda{ [&](ArgsTs&&... params) -> decltype(auto) {
	//		// Object must have mem_fn in its definition.
	//		return (std::forward<ObjectT>(object).*mem_fn)(std::forward<ArgsTs>(params)...);
	//	} };

	//	if constexpr (!std::is_void_v<ReturnT>) { // member function returns some type
	//		return std::make_tuple(true, std::apply(call_lambda, std::forward<std::tuple<ArgsTs...>>(args)));
	//	}
	//	else {
	//		std::apply(call_lambda, std::forward<std::tuple<ArgsTs...>>(args));
	//		return std::make_tuple(true); // member function returns void type
	//	}
	//}

} // !namespace deprecated


#endif // !FUNCTIONAL_HPP
