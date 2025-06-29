#ifndef WEAK_METHOD_INVOKER_HPP
#define WEAK_METHOD_INVOKER_HPP

#include <memory>
#include <type_traits> // conditional_t
#include <tuple>
#include <utility> // pair, forward

#include "memory-management-library/weak-ptr/weak-ptr.hpp"
#include "general-utilities-library/functional/functional.hpp"


namespace common {

	/*
	* ApplyMemFn
	* Summary:

This code provides flexible utilities to invoke member functions via pointers or weak pointers with arguments supplied as tuples. It handles various cases including:

    Member functions returning void or non-void types.
    Null or expired pointers.
    Type safety through static assertions.
    Efficient argument unpacking via index sequences.

This pattern is useful in callback systems, reflection-like mechanisms, or generic frameworks where functions are invoked dynamically with parameters stored in tuples.
	*/


//=================WeakMemFnCall===============================================================

	/**
	* Data needed for calling member function on object, that can be already destructed.
	* In every function call there is implicit argument this pointer to object.
	*/
	template<typename MemFnPtrT, typename ObjectT, typename... ArgsTs>
	struct WeakMethodCallData {
		/** Pointer to member function. */
		MemFnPtrT mem_fn{};

		/** Weak pointer to object, that will be used for calling member function. */
		std::weak_ptr<ObjectT> object_ptr{};

		/** Tuple of arguments for member function call, except this pointer argument. */
		std::tuple<ArgsTs...> args{};
	};

	/**
	* Invokes member function on weak_ptr pointer to object using std::apply and tuple arguments.
	*
	* @param mem_fn			pointer to member function
	* @param object_ptr		pointer to object, on which member function will be called
	* @param args			arguments for member function call
	* @return				tuple(success flag, return value of member function call. Or Nothing = void return )
	*/
	/*template<typename MemFnPtrT, typename ObjectT, typename... ArgsTs>
	inline auto InvokeMethodPtr(WeakMemFnCallData<MemFnPtrT, ObjectT, ArgsTs...>&& call_data)
			->	std::conditional_t< !std::is_void_v<typename MemFnPtrTraits<MemFnPtrT>::ReturnT>,
				std::tuple<bool, typename MemFnPtrTraits<MemFnPtrT>::ReturnT>,
				std::tuple<bool> >
	{
		return ApplyPtrMemFn(call_data.mem_fn, call_data.object_ptr,
							std::forward<std::tuple<ArgsTs...>>(call_data.args));
	}*/


	/**
	* Member function call on weak pointer to object.
	* Stores all data and arguments needed for member function call.
	* Can be compared with each other and used in hash table.
	*
	* Invariant:		object must exists.
	*					mem_fn mustn't be nullptr. mem_fn must point to exists member function of object.
	*					Number of arguments must be equal to number of arguments of member function.
	*					Ts of arguments must be proper.
	*					mem_fn != nullptr && !object.expired().
	*
	* @tparam ReturnT		The return type of member function.
	* @tparam ObjectT		Type of object, for which member function will be called.
	* @tparam TupleArgsT	Tuple of Arguments for member function call, all except this pointer.
	*/
	template<typename MemFnPtrT, typename ObjectT, typename TupleArgsT>
	class WeakMethodInvoker {
	public:
		using Traits = MemFnPtrTrait<MemFnPtrT>;
		/** The return from member function call. Real return of member function. */
		using ReturnT = typename Traits::ReturnT;
		/** Tuple of arguments types for member function call. */
		using ArgsT = typename Traits::ArgsT;

		/*
		* The return from weak member function call. tuple<success flag, ReturnT>
		* It can fail, cause expired object. So has success flag.
		*/
		using WeakMemFnReturnT = std::conditional_t< !std::is_void_v<ReturnT>,
														std::tuple<bool, ReturnT>,
														std::tuple<bool> >;
		//using WeakMemFnCallDataT = WeakMemFnCallData<MemFnPtrT, ObjectT, ArgsT...>;

		/** Preconditions saving invariant. */
		static_assert(std::is_same_v<std::remove_cvref_t<typename Traits::ObjectT>,
														std::remove_cvref_t<ObjectT> >,
						"ClassT of Pointer to member function must be equal to ObjectT.");
		static_assert(std::is_object_v< std::remove_reference_t<ObjectT> >,	// reference to object is not object
						"object: object must be object type. F.e. class, struct, union.");
		static_assert(std::is_member_function_pointer_v< std::remove_reference_t<MemFnPtrT> >,
						"mem_fn: Expected member function pointer.");
		static_assert(HasMemberFn_v<MemFnPtrT>,
						"The specified method does not exist in the given class.");
		static_assert(std::is_same_v< std::remove_cvref_t<ArgsT>, std::remove_cvref_t<TupleArgsT> >,
						"Types of arguments in member function pointer and in tuple of arguments must be equal.");


		WeakMethodInvoker() = default;

		/**
		* Save call data in weak member function call object.
		*
		* @param mem_fn			pointer to member function
		* @param object_ptr		pointer to object, on which member function will be called
		* @param args			arguments for member function call
		*/
		WeakMethodInvoker(	MemFnPtrT					mem_fn,
							std::weak_ptr<ObjectT>		object_ptr,
							TupleArgsT&&				args) {
			SetInvokeData(mem_fn, object_ptr, std::forward<TupleArgsT>(args));
		}


		/**
		* Save call data in weak member function call object.
		*
		* @param mem_fn			pointer to member function
		* @param object_ptr		pointer to object, on which member function will be called
		* @param args			arguments for member function call
		*
		* @return				if call data is valid result = true
		*/
		inline bool SetInvokeData(MemFnPtrT				mem_fn,
								std::weak_ptr<ObjectT>	object_ptr,
								TupleArgsT&&	args) noexcept {
			if (IsInvokeDataValid(mem_fn, object_ptr, std::forward<TupleArgsT>(args)) &&
				   SaveInvokeData(mem_fn, object_ptr, std::forward<TupleArgsT>(args)))
			{
				return true;
			}

			return false;
		}


		/**
		* Invokes member function on weak_ptr pointer to object using tuple of arguments.
		*
		* @return		tuple(success flag, return value of member function call. Or Nothing = void return )
		*/
		inline WeakMemFnReturnT operator()() const { // object_ptr will be locked inside InvokeMethodPtrTuple
			return InvokeMethodPtrTuple(mem_fn, object_ptr, std::forward<TupleArgsT>(args));
		}

		/** Function for saving invariant */
		inline bool IsInvokeDataValid() const noexcept {
			if (!object_ptr_.expired() && mem_fn_) { return true; }
			return false;
		}

//-------------------Comparations & ----------------------------------------------

		bool operator==(const WeakMethodInvoker& other) const noexcept {
			// compare mem_fn
			if (mem_fn_ != other.mem_fn_) { return false; }

			// object_ptr
			if (!IsWeakPtrOwnerEqual(object_ptr_, other.object_ptr_)) { return false; }

			// compare mem_fn
			if (!(args_ == other.args_)) { return false; }

			return true;
		}

		/** It is very difficult to compare two member function call. Unavailable task. So there is stub here. */
		bool operator<(const WeakMethodInvoker& other) const noexcept {
			//return std::tie(mem_fn_, object_ptr_, args_) < std::tie(other.mem_fn_, other.object_ptr_, other.args_);
		}

//----------------------Hashing---------------------------------------------------

		/**
		* Create hash from all components of weak member function call.
		*
		* @return	hash value for WeakMethodInvoker components of call
		*/
		inline size_t Hash() const noexcept {
			return HashStable();
		}

		/**
		* Create hash from all components of weak member function call.
		* Needs to lock weak_ptr object_ptr.
		*
		* @return	hash value for WeakMethodInvoker components of call
		*/
		inline size_t HashStable() const noexcept {
			size_t seed{ 0 };
			seed = MixHashMemFn(seed);
			seed = MixHashObjectPtrStable(seed);
			seed = MixHashArgs(seed);
			return seed;
		}

		/**
		* Create hash from all components of weak member function call.
		* No need to lock weak_ptr object_ptr. May be more collisions.
		*
		* @return	hash value for WeakMethodInvoker components of call
		*/
		inline size_t HashQuick() const noexcept {
			size_t seed{ 0 };
			seed = MixHashMemFn(seed);
			seed = MixHashObjectPtrQuick(seed);
			seed = MixHashArgs(seed);
			return seed;
		}


//-------------------Setter & Getters---------------------------------------

		// TODO: Check Data Validity in setter methods

		inline void set_mem_fn(const MemFnPtrT new_mem_fn) noexcept							{ mem_fn_ = new_mem_fn; }
		inline void set_object_ptr(const std::weak_ptr<ObjectT>& new_object_ptr) noexcept	{ object_ptr_ = new_object_ptr; }
		inline void set_args(TupleArgsT&& new_args) noexcept {
			args_ = std::forward<TupleArgsT>(new_args);
		}

		inline const MemFnPtrT&					mem_fn() const noexcept		{ return mem_fn_; }
		inline const std::weak_ptr<ObjectT>&	object_ptr() const noexcept { return object_ptr_; }
		inline const TupleArgsT&				args() const noexcept		{ return args_; }

	private:
		/** Function for saving invariant */
		inline bool IsInvokeDataValid(MemFnPtrT mem_fn,
			std::weak_ptr<ObjectT> object_ptr,
			TupleArgsT&& args) const noexcept {
			// Main check is done at template instantiation in static_asserts of class
			if (!object_ptr.expired() && mem_fn) { return true; }
			// TODO: add more conditions. f.e. args count condition.
			return false;
		}

		/**
		* Save data without checking it's validity.
		*
		* @return	success flag
		*/
		inline bool SaveInvokeData(MemFnPtrT mem_fn,
									std::weak_ptr<ObjectT> object_ptr,
									TupleArgsT&& args) noexcept {
			if (auto shared_locked = object_ptr.lock()) {
				mem_fn_ = mem_fn;
				object_ptr_ = object_ptr;
				args_ = std::forward<TupleArgsT>(args);
				return true;
			}
			return false;
		}

//------------Hash-------------------------------------------------------------------------------
		// 0x9e3779b9 - magic number for hashing

		/**
		* Mixing hash. Use member function address. Return new seed value.
		*
		* @return	mixed new seed value.
		*/
		inline size_t MixHashMemFn(size_t seed) const noexcept {
			std::hash<void*> hasher_func;
			seed ^= hasher_func(reinterpret_cast<const void*>(mem_fn_)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			return seed;
		}

		/**
		* Mixing hash. Lock weak_ptr and get his address.
		*
		* @return	mixed new seed value.
		*/
		inline size_t MixHashObjectPtrStable(size_t seed) const noexcept {
			if (auto shared_locked = object_ptr_.lock()) {
				std::hash<void*> hasher_obj;
				seed ^= hasher_obj(reinterpret_cast<const void*>(shared_locked.get())) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}

		/**
		* Mixing hash. Add unique key of weak_ptr owner using count of weak_ptr. No need to lock.
		* May be more collisions.
		*
		* @return	mixed new seed value.
		*/
		inline size_t MixHashObjectPtrQuick(size_t seed) const noexcept {
			std::hash<size_t> hasher_owner;
			seed ^= hasher_owner(object_ptr_.use_count()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			return seed;
		}

		/**
		* Mixing hash. Add arguments in tuple.
		*
		* @return	mixed new seed value.
		*/
		inline size_t MixHashArgs(size_t seed) const noexcept {
			seed = ApplyMethodPart(&WeakMethodInvoker<MemFnPtrT, ObjectT,TupleArgsT>::MixHashArgsImpl,
									*this, seed, std::forward<TupleArgsT>(args_));
			return seed;
		}

		template<typename... ArgsT>
		inline size_t MixHashArgsImpl(size_t seed, ArgsT&&... args) const noexcept {
			((seed ^= std::hash<decltype(args)>{}(args)+0x9e3779b9 + (seed << 6) + (seed >> 2)) , ...); // fold expression
			return seed;
		}

		/*inline size_t MixHashArgs(size_t seed) const noexcept {
			std::apply([&seed](auto&&... args) {
			((seed ^= std::hash<decltype(args)>{}(args)+0x9e3779b9 + (seed << 6) + (seed >> 2)), ...);
				}, args_);
			return seed;
		}*/


		// TODO: Use random hash seed for better distribution
		//std::random_device rd;
		//unsigned int seed = rd();
		//struct CustomStringHasher {
		//	std::size_t operator()(const std::string& s) const noexcept {
		//		// Используем стандартный хэш с новым seed'ом
		//		return std::hash<std::string>{}(s) ^ seed;
		//	}
		//};

//------------Data-------------------------------------------------------------------------------

		/** Pointer to member function. */
		MemFnPtrT mem_fn_{};

		/** Weak pointer to object, that will be used for calling member function. */
		std::weak_ptr<ObjectT> object_ptr_{};

		/** Tuple of arguments for member function call, except this pointer argument. */
		TupleArgsT args_{};
	}; // !class WeakMethodInvoker
	/*
	* TODO: maybe add validity flag. weak invoker may be in invalid state, cause of wrong data in
	* constructor or mutators functions
	* TODO: maybe not allow to have invalid state by saving invariant.
	*/

	/*
	* Design choices:
	*	Maybe variant with  std::weak_ptr<void> object and std::function<void()> member function.
	*	But it will be impossible to compare WeakMemberFn objects and to use this class in hash table.
	*/


	// It is very difficult to compare two weak member functions call by < less operator for std::set,
	// cause they has different types of arguments.
	// You can say, that one member function call is less than another, if objects in both are equal and all arguments
	//		of one call are less than arguments of another call. But is so rare condition.
	// Maybe only by comparing WeakMethodInvoker address.
	//



//=================Class for storing WeakMethodInvoker homogeneously in container===================

	/**
	* Abstract. Callable object, that invoke some action - function returning void.
	* It may be lambda, std::function, std::bind, functor, free functions, functions members.
	*
	* Class for Wrapper storing weak member function call.
	*/
	class IMethodAction {
	protected:
		IMethodAction() = default;
		IMethodAction(const IMethodAction&) = delete; // polymorphic guard. class suppress copy/move C.67
		IMethodAction& operator=(const IMethodAction&) = delete;
		IMethodAction(IMethodAction&&) noexcept = delete;
		IMethodAction& operator=(IMethodAction&&) noexcept = delete;
	public:
		virtual ~IMethodAction() = default;

		/** Copy this object and return new copy. */
		virtual std::unique_ptr<IMethodAction> Clone() const noexcept = 0;

		/** Move this object to new unique_ptr. */
		virtual std::unique_ptr<IMethodAction> TransferOwnership() noexcept = 0;


		/** Call action without return value. */
		virtual void operator()() const = 0;

		virtual bool operator==(const IMethodAction& other) const noexcept = 0;

		virtual bool operator<(const IMethodAction& other) const noexcept = 0;

		/** Return hash value for this object. */
		virtual size_t Hash() const noexcept = 0;

	}; // !class IWeakMethodInvokerWrap


	/*
	* Wrapper class for  calling Action() member functions of objects, that maybe already destructed and expired.
	*
	* Pattern: Bridge. Used for type erasing and storing in container.
	*/
	template<typename MemFnPtrT, typename ObjectT, typename TupleArgsT>
	class WeakMethodAction : public IMethodAction {
	public:
		using WeakMethodInvokerType = WeakMethodInvoker<MemFnPtrT, ObjectT, TupleArgsT>;

		using Traits = WeakMethodInvokerType::Traits;
		using ReturnT = WeakMethodInvokerType::ReturnT;
		using ArgsT = WeakMethodInvokerType::ArgsT;

		WeakMethodAction() = default;
	protected:
		WeakMethodAction(const WeakMethodAction&) = delete; // polymorphic guard. class suppress copy/move C.67
		WeakMethodAction& operator=(const WeakMethodAction&) = delete;
		WeakMethodAction(WeakMethodAction&&) noexcept = delete;
		WeakMethodAction& operator=(WeakMethodAction&&) noexcept = delete;
	public:
		~WeakMethodAction() override = default;

//----------------------Constructors------------------------------------------------------------

		/**
		* Save call data in weak member function call object.
		*
		* @param mem_fn			pointer to member function
		* @param object_ptr		pointer to object, on which member function will be called
		* @param args			arguments for member function call
		*/
		WeakMethodAction(MemFnPtrT					mem_fn,
						std::weak_ptr<ObjectT>		object_ptr,
						TupleArgsT&&				args) {
			invoker_.SetInvokeData(mem_fn, object_ptr, std::forward<TupleArgsT>(args));
		}

		/** Copy this object and return new copy. */
		std::unique_ptr<IMethodAction> Clone() const noexcept override {
			return std::make_unique<WeakMethodInvokerType>(invoker_);
		}

		/** Move this object to new unique_ptr. */
		std::unique_ptr<IMethodAction> TransferOwnership() noexcept override {
			return std::make_unique<WeakMethodInvokerType>(std::move(invoker_));
		}

//-----------------------------------------------------------------------------------------------------

		/**
		* Save call data in weak member function call object.
		*
		* @param mem_fn			pointer to member function
		* @param object_ptr		pointer to object, on which member function will be called
		* @param args			arguments for member function call
		*
		* @return				if call data is valid result = true
		*/
		inline bool SetInvokeData(MemFnPtrT				mem_fn,
								std::weak_ptr<ObjectT>	object_ptr,
								TupleArgsT&&			args) noexcept {
			return invoker_.SetInvokeData(mem_fn, object_ptr, std::forward<TupleArgsT>(args));
		}


		/** Invoke action without return value. */
		void operator()() const override {
			invoker_();
		}

		bool operator==(const IMethodAction& other) const noexcept override {// it is not safe, if there is more inhereted classes
			return invoker_.operator==(static_cast<const WeakMethodInvokerType>(other));
		}

		bool operator<(const IMethodAction& other) const noexcept override {
			return invoker_.operator<(static_cast<const WeakMethodInvokerType>(other));
		}

		/** Return hash value for this object. */
		size_t Hash() const noexcept override {
			return invoker_.Hash();
		}

	private:
		WeakMethodInvokerType invoker_{};
	}; // !class WeakMethodAction


	/*
	* Wrapper class for  calling Action() member functions of objects, that maybe already destructed and expired.
	*
	* Pattern: Bridge. Used for type erasing and storing in container.
	*/
	class MethodAction { // TODO: !refactor MethodAction
	public:
		MethodAction() = default;
		MethodAction(const MethodAction&) = delete; // polymorphic guard. class suppress copy/move C.67
		MethodAction& operator=(const MethodAction&) = delete;
		MethodAction(MethodAction&&) noexcept = default;
		MethodAction& operator=(MethodAction&&) noexcept = default;
		~MethodAction() = default;

		/**
		* Save call data in weak member function call object.
		*
		* @param mem_fn			pointer to member function
		* @param object_ptr		pointer to object, on which member function will be called
		* @param args			arguments for member function call
		*/
		template<typename MemFnPtrT, typename ObjectT, typename TupleArgsT>
		MethodAction(MemFnPtrT					mem_fn,
					std::weak_ptr<ObjectT>		object_ptr,
					TupleArgsT&&				args)
			: impl_{ MakeImpl(mem_fn, object_ptr, std::forward<TupleArgsT>(args)) }
		{
		}


		/**
		* Save call data in weak member function call object.
		*
		* @param mem_fn			pointer to member function
		* @param object_ptr		pointer to object, on which member function will be called
		* @param args			arguments for member function call
		*
		* @return				if call data is valid result = true
		*/
		template<typename MemFnPtrT, typename ObjectT, typename TupleArgsT>
		inline bool SetInvokeData(MemFnPtrT				mem_fn,
									std::weak_ptr<ObjectT>	object_ptr,
									TupleArgsT&& args) noexcept {
			impl_.reset(MakeImpl<MemFnPtrT, ObjectT, TupleArgsT>());
			return static_cast<WeakMethodAction<MemFnPtrT, ObjectT, TupleArgsT>>(impl_).SetInvokeData( mem_fn, object_ptr,
																								std::forward<TupleArgsT>(args) );
		}


		/** Copy this object and return new copy. */
		std::unique_ptr<IMethodAction> Clone() const noexcept {
			if (!impl_) { return std::unique_ptr<IMethodAction>(); }
			return impl_->Clone();
		}

		/** Move this object to new unique_ptr. */
		std::unique_ptr<IMethodAction> TransferOwnership() noexcept {
			if (!impl_) { return std::unique_ptr<IMethodAction>(); }
			return impl_->TransferOwnership();
		}


		/** Call action without return value. */
		void operator()() const {
			if (!impl_) { return; }
			impl_->operator()();
		}

		bool operator==(const IMethodAction& other) const noexcept {
			if (!impl_) { return false; }
			return impl_->operator==(other);
		}

		bool operator<(const IMethodAction& other) const noexcept {
			if (!impl_) { return false; }
			return impl_->operator<(other);
		}

		/** Return hash value for this object. */
		size_t Hash() const noexcept {
			if (!impl_) { return 0; }
			return impl_->Hash();
		}

	private:
		template<typename MemFnPtrT, typename ObjectT, typename TupleArgsT>
		inline std::unique_ptr<IMethodAction>
				MakeImpl(MemFnPtrT				mem_fn,
						std::weak_ptr<ObjectT>	object_ptr,
						TupleArgsT&&			args) {
			return std::make_unique<WeakMethodAction<MemFnPtrT, ObjectT, TupleArgsT>>(mem_fn, object_ptr,
																					std::forward<TupleArgsT>(args));
		}

		template<typename MemFnPtrT, typename ObjectT, typename TupleArgsT>
		inline std::unique_ptr<IMethodAction> MakeImpl() {
			return std::make_unique<WeakMethodAction<MemFnPtrT, ObjectT, TupleArgsT>>();
		}
//---------------Data-----------------------------------------------------------

		std::unique_ptr<IMethodAction> impl_{};
		// TODO: Big problem with call data validity!
		// TODO: checks for empty ptr impl_ or invariant impl_ != nullptr!
		// TODO: try to erase all if (impl) by saving invariant impl_ != nullptr!
		// TODO: maybe static asserts in templated functions?

	}; // !class MethodAction

} // !namespace common



namespace std {
	//template <> struct hash<WeakMemberFn> {
	//	size_t operator()(const WeakMemberFn& wmf) const {
	//		// Реализация хэширования
	//	}
	//};

	//std::hash;
	/*std::equal_to;
	std::less;*/

	/*operator==
	operator<*/
}





namespace help {
	/** T is used for function execution. */
	//using Task = std::function<void()>;

	/** Alias of Task. */
	//using Action = Task;

} // !namespace help


#endif // !WEAK_METHOD_INVOKER_HPP

