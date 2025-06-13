#ifndef OBSERVER_CALLBACK_HPP
#define OBSERVER_CALLBACK_HPP

// TODO: compress include list
#include <atomic>
#include <algorithm> // remove_if
#include <execution> // execution policies
#include <forward_list>
#include <functional>
#include <future>	// for Async Update
#include <memory>
#include <mutex>
#include <iterator>
#include <initializer_list>	// for AttachObservers
#include <string>
#include <shared_mutex>
#include <system_error>	// thread execution exception
#include <set>
#include <thread>	// concurrency and thread safety SubjectWeakHub
#include <type_traits>
#include <list>
#include <utility> // pair
#include <unordered_set>
#include <vector>

// Container variants

#include "error/custom-exception.hpp"
//#include "error/error.hpp"

namespace common

namespace std {
	/** A specialized version of the hash function for std::weak_ptr */
	template<typename T>
	struct hash<std::weak_ptr<T>> {
		size_t operator()(const std::weak_ptr<T>& wp) const noexcept {
			if (auto sp = wp.lock()) {
				return std::hash<std::shared_ptr<T>>{}(sp);
			}
			return 0;
		}
	};
	// Hash for unordered_set, unordered_map must be stable and not to change with expiration of weak_ptr
}

/** Software Design Patterns */
namespace pattern {
	namespace behavioral {
			// https://en.wikipedia.org/wiki/Observer_pattern
			// https://eliogubser.com/posts/2021-04-cpp-observer/	Simple observer pattern using modern C++
			//		eliogubser Used Function wrapper for Update command.
			//
			// Friend patterns: Mediator. ChangeManager act like mediator between Observers and Subjects,
			//		encapsulate complex mechanics of Update.
			// Singleton is used in ChangeManager.
			// Command. Realization of Observer may store Delegates in Subject to Observer's functions.
			//
			// Classes Observer and Subject depends on each other by state

			/**
			 * If too many subjects are observed by little amount of observers you can use hash table
			 * to economy space, making access time worser.
			 * If there are many subject, that observer must be subscribed. Needs to add Subject& param to Observer.Update()
			 */

			 /**
			  * Observer with "event propagation" or in realization of "Listener".
			  *
			  * Push, Pull protocols of update.
			  * Push is less decoupled. Subject knows what data is needed by observer.
			  * Pull is more decoupled. Subject knows nothing about observer. Observer decides what data is needed.
			  *
			  * Aspect defines what events are interested for observer.
			  * Realization by subscribing to concrete events.
			  * Second, by sending information about event when Updating
			  *
			  * One Observer may has many Subjects. One Subject can has many Observers.
			  */

			  /**
			   * Update function may be called in Subject's Notify() in mode Sync or Async (Message, Events Queue).
			   * You can use parallel threads, if Observer's Update() is huge, slow, resource eating code.
			   * Or if there is huge count of observers.
			   *
			   * Lapsed Listener Problem - memory leak, if there is strong references, pointers to Observers.
			   *
			   * Message Queue. By creating a dedicated message queue server (and sometimes an extra message handler
			   * object) as an extra stage between the observer and the object being observed, thus decoupling the components.
			   *
			   * Change Manager - is Mediator Pattern, that helps to move all refs to Observer from Subject to Manager.
			   * Change Manager is best solution for complex update logic.
			   */

			   // Observers always responsible for controlling relationship between Subjects and Observers. Not by Subjects.
			   // Subjects only notify Observers and don't know anything about them.
				/*
				* Most popular realization of Observer Pattern is, when one Observer is observing one Subject.
				* This realization give opportunity to have create of events and decrease dependency.
				*/

				/*
				* Boost.Signals. Can be a good alternative to implementing the Observer pattern in C++.
				* They provide a flexible and efficient way to manage event-driven programming and
				* facilitate communication between objects.
				*/


		namespace observer_boost {
			// https://www.boost.org/doc/libs/1_63_0/doc/html/signals.html
		}

		namespace observer_weak_cmd {

//=============================Helper Functions for ObserverMulti & SubjectMulti=====================================


			class IWeakMemberFn {
			public:
			};

			/**
			* Class for calling member functions of objects, that maybe already destructed and expired.
			*/
			template<typename ObjectType>
			class WeakMemberFn : public IWeakMemberFn { // For Observer with weak_ptr std::function inside Subject
			public:
				/** Command function std::function<void()>. */
				using MemberFunctionType = std::function<void()>;
				/** Pointer to Object, that maybe already destructed. */
				using WeakPtrObject = std::weak_ptr<ObjectType>;

				/** Type Pointer to Member Function. */
				template<typename ActionReturnType, typename... ActionParamTypes>
				using MemberFunctionPtrType = ActionReturnType(ObjectType::*)(ActionParamTypes...);


				WeakMemberFn() = default;
				/**
				 * Bind member function with saving weak_ptr on the object of member function.
				 * Arguments are binded with std::function(void()).
				 *
				 * @param action_ptr			pointer to member function
				 * @param receiver_p			ref to receiver object
				 * @param ...action_args_p		arguments for member function call
				 */
				template<typename ReturnType, typename... ParamTypes>
				WeakMemberFn(const MemberFunctionPtrType<ReturnType, ObjectType, ParamTypes...> action_ptr,
										const WeakPtrObject receiver_ptr,
										ParamTypes&&... action_args_p) {
					Bind(action_ptr, receiver_ptr, action_args_p...);
				}

				/**
				 * Bind member function with saving weak_ptr on the object of member function.
				 * Arguments are binded with std::function(void()).
				 *
				 * @param action_ptr			pointer to member function
				 * @param receiver_p			ref to receiver object
				 * @param ...action_args_p		arguments for member function call
				 * @return				expired	flag of receiver weak_ptr pointing to object, which member function is called.
				 */
				template<typename ReturnType, typename... ParamTypes>
				//requires std::is_class_v<ReceiverType>
				bool Bind(const MemberFunctionPtrType<ReturnType, ObjectType, ParamTypes...> action_ptr,
							const WeakPtrObject receiver_ptr,
							ParamTypes&&... action_args_p) {	// noexcept: bind maybe has exceptions
					if (!action_ptr || receiver_ptr.expired()) { return; } // precondition

					if (auto shared_receiver = receiver_ptr.lock()) {
						// Type can be converted from any pointer to fn type to void(), if all arguments of fn are binded
						// Only reinterpret_cast helps to convert one pointer to member function to another, of another type
						// void() is unified type for all command functions.
						member_function_ = std::bind(action_ptr, &*shared_receiver,
													std::forward<ParamTypes>(action_args_p)...);
					}
					object_ptr_ = receiver_ptr;
				}

				/**
				* Call member function of object, that maybe already destructed and expired
				*
				* @return	expired flag of weak_ptr pointing to object, which member function is called.
				*/
				bool operator()() const {
					if (!object_ptr_.expired()) {  // check cheap operation expireted, then lock for call
						if (auto shared_ptr = object_ptr_.lock()) {
							member_function_();
							return false;
						}
					}
					return true;
				}

			private:
				/** Pointer to Object, that maybe already destructed. */
				WeakPtrObject object_ptr_{};
				/** Binded with arguments command member function */
				MemberFunctionType member_function_{};
				// TODO: it is important to store all args of std::function seperately for duplicate check in Observer Pattern
				// Design choices:
				// 1) To store container in Subject of WeakMemberFn objects
				// 2) To store 2 containers in Subject of pair(weak_ptr to observers, std::function to member of observer)
			}; // !class WeakMemberFn



			class WeakMemberFnWrap {
			public:
				template<typename ObjectType>
				static std::shared_ptr<BaseWeakMemberFunction> Create(
														const std::weak_ptr<ObjectType>& receiver_ptr,
														void (ObjectType::* action_ptr)(),
														const std::tuple<>& args = {}) {
					auto impl = std::make_shared<WeakMemberFunctionImpl<ObjectType>>();
					impl->Bind(action_ptr, receiver_ptr); // Привязываем метод и аргумент (если нужен)
					return impl;
				}

				explicit WeakMemberFunction(std::shared_ptr<BaseWeakMemberFunction> impl)
					: impl_(impl) {
				}

				void operator()() const {
					(*impl_)(); // Вызываем реализацию
				}

				bool expired() const noexcept {
					return impl_->expired();
				}

			private:
				std::shared_ptr<BaseWeakMemberFunction> impl_;
			};


			//class WeakMemberFnWrap {
			//public:
			//	template<typename ObjectType>
			//	static std::shared_ptr<BaseWeakMemberFunction> Create(
			//		const std::weak_ptr<ObjectType>& receiver_ptr,
			//		void (ObjectType::* action_ptr)(),
			//		const std::tuple<>& args = {}) {
			//		auto impl = std::make_shared<WeakMemberFunctionImpl<ObjectType>>();
			//		impl->Bind(action_ptr, receiver_ptr); // Привязываем метод и аргумент (если нужен)
			//		return impl;
			//	}

			//	explicit WeakMemberFunction(std::shared_ptr<BaseWeakMemberFunction> impl)
			//		: impl_(impl) {
			//	}

			//	void operator()() const {
			//		(*impl_)(); // Вызываем реализацию
			//	}

			//	bool expired() const noexcept {
			//		return impl_->expired();
			//	}

			//private:
			//	std::shared_ptr<BaseWeakMemberFunction> impl_;
			//};



//===========================Generic Container Element Modification================================

			/**
			 * Add (emplace, push or insert) element from any type of container.
			 * Complexity: O(1)
			 * Mutex: write
			 */
			template<typename ContainerType>
			inline void GenericAddElement(ContainerType& container, typename ContainerType::value_type&& value) {
				using value_type = typename ContainerType::value_type;

				if constexpr (std::is_same_v<std::remove_cvref_t<ContainerType>,
											 std::forward_list<value_type>>) { // forward_list
					container.emplace_front(std::forward<value_type>(value));			// O(1)
				} else { // All other types of containers, except forward_list
					container.emplace(std::forward<value_type>(value));					// O(1)
					// emplace_back is better for  vector, deque and list
				}
			}

			/**
			 * Remove element from any type of container.
			 * Complexity: O(n)
			 * Mutex: write
			 *
			 * @param predicate for remove_if operation
			 */
			template<typename ContainerType, typename ExecPolicyType>
			inline void GenericRemoveIf(ContainerType& container,
										auto predicate,
										ExecPolicyType policy = std::execution::seq) {
				if (container.empty()) { return; } // Precondition

				if constexpr (std::is_same_v<ContainerType,
											std::forward_list<typename ContainerType::value_type>>) { // for Forward_list
					container.remove_if(predicate); // erase-remove idiom is not for forward_list
					// std::remove_if is not working with forward_list
				} else { // All other containers
					container.erase(std::remove_if(policy, container.begin(),
													container.end(), predicate), container.end()); // O(n)
				}
			}
			// TODO: GenericErase(), GenericFind() ?

//============================Process weak_ptr Container. Erase============================================

			using ExpiredFnType = decltype([](const auto& value_weak_ptr) { return value_weak_ptr.expired(); });

			/**
			 * Erase all expired weak_ptr from container
			 *
			 * Complexity: O(n)
			 * Mutex: write
			 */
			template<typename ContainerType, typename ExecPolicyType>
			inline void EraseAllExpired(ContainerType& container, ExecPolicyType policy = std::execution::seq) {
				// Generic Function. Maybe used not only in observer. So no mutex lock inside.
				if (container.empty()) { return; }

				auto expired = [](const auto& value_ptr) { return value_ptr.expired(); };
				GenericRemoveIf(container, expired, policy);		// O(n)
			};

			/**
			 * Erase custom n number of expired weak_ptr from container
			 *
			 * Complexity: O(n)
			 * Mutex: write
			 */
			template<typename ContainerType, typename ExecPolicyType>
			inline void EraseNExpiredWeakPtr(ContainerType& container,
											const size_t expired_count,
											ExecPolicyType policy = std::execution::seq) {
				if (container.empty() || expired_count == 0) { return; } // Precondition
				//static_assert(std::is_same_v<typename ContainerType::value_type, std::weak_ptr<ValueType>>,
								//"The type mismatch between container elements and weak_ptr");

				size_t find_count{};
				auto expired_fn = [&expired_count, &find_count](const typename ContainerType::value_type& value_ptr) {
					if (find_count < expired_count && value_ptr.expired()) { // after N expired found, value_ptr.expired() is not called
						++find_count;
						return true;
					}
					return false;
				}; // !lambda
				GenericRemoveIf(container, expired_fn, policy);		// O(n)
			};
			// TODO: may work not on expired_count, but on it_last_expired - iterator to last expired.


			/**
			 * Erase first weak_ptr in container, that is alive and has same stored pointer. Container stores weak_ptr.
			 *
			 * Complexity: set = O(log n). Other containers = O(n).
			 * Mutex: write.
			 *
			 * \param container
			 * \param searched_ptr
			 * \param policy
			 * @return	count of expired weak_ptr
			 */
			template<typename ContainerType, typename ValueType, typename ExecPolicyType>
			inline size_t EraseEqualWeakPtr(ContainerType& container,
											const std::weak_ptr<ValueType> searched_ptr,
											ExecPolicyType policy = std::execution::seq) {
				size_t expired_count{};
				if (container.empty() || searched_ptr.expired()) { return expired_count; }
				static_assert(std::is_same_v<typename ContainerType::value_type, std::weak_ptr<ValueType>>,
								"The type mismatch between container elements and weak_ptr");

				auto it_equal{ container.end() };
				if constexpr (std::is_same_v<ContainerType, std::forward_list<typename ContainerType::value_type>>) { // forward_list
					std::shared_ptr<ValueType> searched_shared{};
					auto equal_owner = [&searched_shared, &expired_count](const auto& current_ptr) {
						if (current_ptr.expired()) { ++expired_count; }
						return IsEqualWeakPtr(searched_shared, current_ptr);		// O(1)
					}; // !lambda end
					{
						searched_shared = searched_ptr.lock();	// weak_ptr
						if (searched_shared) { container.remove_if(equal_owner); }	// O(n)
					}
				} else { // All other types of containers
					// std::move(searched_ptr) is no necessary, cause weak_ptr hold 2 pointers.
					// Copy or Move operations are equal in performance.
					auto result_fn{ FindEqualWeakPtr(container, searched_ptr, policy) }; // set = O(log n) ; others = O(n)
					it_equal = result_fn.first;
					expired_count = result_fn.second;
					if (it_equal != container.end()) { container.erase(it_equal); }
				}
				return expired_count;
			}
			// TODO: refactor -> 1) First GenericFind(). 2) GenericErase(). Deletion for set is simple

			/**
			 * Erase first weak_ptr in container, that is alive and has same stored pointer. Container stores weak_ptr.
			 * With auto clean of expired weak_ptrs.
			 *
			 * Complexity: O(n)
			 * Mutex: write
			 *
			 * \param container
			 * \param searched_ptr
			 * \param policy
			 */
			template<typename ContainerType, typename ValueType, typename ExecPolicyType>
			inline void EraseEqualWeakPtrNClean(ContainerType& container,
												const std::weak_ptr<ValueType> searched_ptr,
												ExecPolicyType policy = std::execution::seq) {
				size_t expired_count{ EraseEqualWeakPtr(container, searched_ptr, policy) };
				// Cleanup expired weak_ptr
				EraseNExpiredWeakPtr(container, expired_count, policy);
			}

//===========================Find weak_ptr====================================================

			/** Function to compare objects via shared_ptr inside weak_ptr */
			//struct OwnerEqual {
			//	bool operator()(const std::weak_ptr<void>& lhs, const std::weak_ptr<void>& rhs) const noexcept {
			//		// Используем lock() для проверки доступности ресурсов
			//		auto lptr = lhs.lock(), rptr = rhs.lock();
			//		if (!lptr || !rptr)
			//			return false;
			//		return lptr.get() == rptr.get();
			//	}
			//};


			/**
			 * Compares if two weak_ptr are alive and have the same stored pointers.
			 *
			 * Complexity: amortized O(1)
			 *
			 * @param shared_ptr search ptr. Is shared for decreasing number of locks.
			 * @param current_ptr
			 * @return result of equality check
			 */
			template<typename ValueType>
			inline bool IsEqualWeakPtr(const std::weak_ptr<ValueType>& searched_shared,
										const std::weak_ptr<ValueType>& current_ptr) noexcept {
				if (current_ptr.expired() || !searched_shared) { return false; } // first less expensive expired(), then lock
				auto current_shared = current_ptr.lock();
				//return current_shared && searched_shared.get() == current_shared.get(); // stored object pointer may point to subobject
				return current_shared && (&*searched_shared == &*current_shared);	// compares only owners addresses
			}
			/**
			 * Compares if two weak_ptr are alive and have the same stored pointers.
			 *
			 * Complexity: amortized O(1)
			 *
			 * @param shared_ptr search ptr. Is shared for decreasing number of locks.
			 * @param current_ptr
			 * @return result of equality check
			 */
			template<typename ValueType>
			inline bool IsOwnerEqual(const std::shared_ptr<ValueType>& searched_shared,
									   const std::weak_ptr<ValueType>& current_ptr) noexcept {
				if (current_ptr.expired() || !searched_shared) { return false; } // first less expensive expired(), then lock
                auto current_shared = current_ptr.lock();
				//return current_shared && searched_shared.get() == current_shared.get(); // stored object pointer may point to subobject
				return current_shared && (&*searched_shared == &*current_shared);	// compares only owners addresses
			}

			/**
			 * Find first weak_ptr, that is alive and has same stored pointer.
			 *
			 * Complexity: set = O(log n). Other containers = O(n).
			 * Mutex: read
			 *
			 * @return	First = iterator to equal weak_ptr or to end.
			 *			Second = size_t count of expired weak_ptr for cleanup.
			 */
			template<typename ContainerType, typename ValueType, typename ExecPolicyType>
			inline auto FindEqualWeakPtr(const ContainerType& container,
										const std::weak_ptr<ValueType> searched_ptr,
										ExecPolicyType policy = std::execution::seq)
					-> std::pair<decltype(container.end()), size_t>
			{
				auto result_fn{ std::make_pair(container.end(), size_t{}) };
				if (container.empty() || searched_ptr.expired()) { return result_fn; } // precondition
				static_assert(std::is_same_v<typename ContainerType::value_type, std::weak_ptr<ValueType>>,
								"The type mismatch between container elements and weak_ptr");

				auto&	it_equal		{ result_fn.first };
				size_t& expired_count	{ result_fn.second };

				if constexpr (std::is_same_v<ContainerType, std::set<typename ContainerType::value_type>>) { // set
					// set must be compared with owner_less in declaration of set variable
					it_equal = container.find(searched_ptr);												// O(log n)
				} else { // All other types of containers
					std::shared_ptr<ValueType> searched_shared{};
					auto equal_owner = [&searched_shared, &expired_count](const auto& current_ptr) {
						if (current_ptr.expired()) { ++expired_count; }
						return IsEqualWeakPtr(searched_shared, current_ptr);								// O(1)
						};
					{
						// mustn't lock for long time, cause of resource use and life time
						// But better to lock for lowering number of locks in IsEqualWeakPtr
						searched_shared = searched_ptr.lock();
						if (searched_shared) { // minimal lock section
							it_equal = std::find_if(policy, container.begin(), container.end(), equal_owner); // O(n)
						}
					} // !weak_ptr lock
				}

				return result_fn;
			}

			/**
			 * Find first weak_ptr, that is alive and has same stored pointer.
			 * With auto clean of expired weak_ptrs.
			 *
			 * Complexity: O(n)
			 * Mutex: read + write
			 *
			 * @return	iterator to equal weak_ptr or to end.
			 */
			template<typename ContainerType, typename ValueType, typename ExecPolicyType>
			inline auto FindEqualWeakPtrNClean(ContainerType& container,
												const std::weak_ptr<ValueType> searched_ptr,
												ExecPolicyType policy = std::execution::seq)
					-> decltype(container.end())
			{
				auto result_fn{ FindEqualWeakPtr(container, searched_ptr, policy) };
				// Cleanup expired weak_ptr
				EraseNExpiredWeakPtr(container, result_fn.second, policy);
				return result_fn.first;
			}

//=====================Notify=====================================================================

			/**
			 * Notify all observers in container by lambda function, that
			 * encapsulate observer update function call.
			 * Can freeze main thread, if it is long to Update observer.
			 *
			 * Complexity: O(n)
			 * Mutex: read
			 *
			 * @param observers
			 * @param observer_update_fn		lambda that encapsulate observer update function call, f.e. observer->Update()
			 * @return	count of expired weak_ptr
			 */
			template<typename ContainerType, typename UpdateFunctionType, typename ExecPolicyType>
			inline size_t GenericNotifyWeakObservers(const ContainerType& observers,
													UpdateFunctionType observer_update_fn,
													ExecPolicyType policy = std::execution::seq) {
				size_t expired_count{};
				if (observers.empty) { return expired_count; } // precondition

				auto update_fn = [&observer_update_fn, &expired_count](const auto& observer_ptr) {
					if (observer_ptr.expired()) { // expired is less expensive, then lock. Thats why it is first
						++expired_count;
						return;
					}
					if (auto observer_shared{ observer_ptr.lock() }) {
						observer_update_fn(observer_shared);
					}
				}; // !lambda
				std::for_each(policy, observers.begin(), observers.end(), update_fn);
				return expired_count;
			}

			/**
			 * Notify all observers in container by lambda function, that
			 * encapsulate observer update function call.
			 * Can freeze main thread, if it is long to Update observer.
			 *
			 * Complexity: O(n)
			 * Mutex: read + write
			 *
			 * @param observers
			 * @param observer_update_fn		lambda that encapsulate observer update function call, f.e. observer->Update()
			 */
			template<typename ContainerType, typename UpdateFunctionType, typename ExecPolicyType>
			inline void GenericNotifyWeakObserversNClean(ContainerType& observers,
														UpdateFunctionType observer_update_fn,
														ExecPolicyType policy = std::execution::seq) {
				size_t expired_count{ GenericNotifyWeakObservers(observers, observer_update_fn, policy) };
				// Cleanup expired weak_ptr
				EraseNExpiredWeakPtr(observers, expired_count, policy);
			}

//==========================Simple Weak Observer version===========================================


			/**
			 * Abstract. Stateless. Interface of Observer.
			 * Alternative name - Subscriber.
			 */
			class IObserverWeak {
			protected:
				IObserverWeak() = default;
				IObserverWeak(const IObserverWeak&) = delete; // C.67	C.21 Abstract suppress Copy & Move
				IObserverWeak& operator=(const IObserverWeak&) = delete;
				IObserverWeak(IObserverWeak&&) noexcept = delete;
				IObserverWeak& operator=(IObserverWeak&&) noexcept = delete;
			public:
				virtual ~IObserverWeak() = default;

				/** Update the information about Subject */
				virtual void Update(const std::string& message) = 0;
			};


			/**
			 * Abstract. Stateless. Subject with weak_ptr dependencies.
			 * Alternative name - Publisher.
			 * weak_ptr helps to prevent dangling pointers and lapsed listener problem.
			 */
			class ISubjectWeak {
			public:
				using WeakPtrIObserverWeak = std::weak_ptr<IObserverWeak>;

			protected:
				ISubjectWeak() = default;
				ISubjectWeak(const ISubjectWeak&) = delete; // C.67	C.21
				ISubjectWeak& operator=(const ISubjectWeak&) = delete;
				ISubjectWeak(ISubjectWeak&&) noexcept = delete;
				ISubjectWeak& operator=(ISubjectWeak&&) noexcept = delete;
			public:
				virtual ~ISubjectWeak() = default;


				/** Update all attached observers */
				virtual void NotifyObservers(const std::string& message) const = 0;

				/**
				 * Add Observer to list of notification in Subject.
				 * There is shared_ptr<ObserverT> outside of class. Subject stores weak_ptr to that Observer.
				 */
				virtual void AttachObserver(const WeakPtrIObserverWeak observer_ptr) = 0;
				// not const, cause must attach subject in observer ptr

				/** Detach observer from notifying list */
				virtual void DetachObserver(const WeakPtrIObserverWeak observer_ptr) = 0;
			};
			/*
			* Design choices. Params in functions. Weak_ptr vs shared_ptr:
			* Best) weak_ptr in params gives guaranty, that we won't use destructed objects.
			* 2) If we use const shared_ptr& outside code must guaranty, that object will be alive, while function is working.
			* 3) If we use shared_ptr we increment strong counter of shared_ptr, increasing life cycle of object.
			*/



//==========================Extended version===========================================

			/** Empty messages code enum for default template arguments */
			enum class ObserverEmptyEnum {};

			/** Empty state of Subject for default template arguments */
			struct SubjectStateEmpty {};


			/**
			 * Abstract. Stateless. Interface of Observer.
			 * Alternative name - Subscriber.
			 * Hub means base class for many possible realizations.
			 * Use name hiding do make relevant Derived class.
			 */
			template<typename UpdateDataType>
			class IObserverWeakHub {
			protected:
				IObserverWeakHub() = default;
				IObserverWeakHub(const IObserverWeakHub&) = delete; // C.67	C.21 Abstract suppress Copy & Move
				IObserverWeakHub& operator=(const IObserverWeakHub&) = delete;
				IObserverWeakHub(IObserverWeakHub&&) noexcept = delete;
				IObserverWeakHub& operator=(IObserverWeakHub&&) noexcept = delete;
			public:
				~IObserverWeakHub() override = default;

				/** Update the information about Subject */
				virtual void Update() = 0;

				/** Update the information about Subject */
				virtual void Update(const std::string& message) = 0;


				/** Generic Update the information about Subject */
				virtual void Update(UpdateDataType data) = 0;

				/** Generic Update the information about Subject */
				virtual void Update(const UpdateDataType& data) = 0;

				/** Generic Update the information about Subject */
				virtual void Update(UpdateDataType&& data) = 0;
			}; // !class IObserverWeakHub
			/*
			* ChatGPT 4.5 | DeepSeek | Midjourney | Telegram | GigaChat
			* Design Choices:					Update() function.
			* 1. No Parameters:					void update()
			* 2. State Information:				void update(int state)
			* 3. Complex State Information:		void update(const StateType& state)
			* 4. Context Information:			void update(const Context& context)		Contextual information related to the update
			* 5. Multiple Parameters:			void update(int value, const std::string& message)
			* 6. Update Returning Status:		bool update()
			* 7. Callback Function:				void update(std::function<void()> callback)
			*
			* 8. Template-based Update Function:	template <typename T> class Observer { void update(const T& data); }
			* 9. Priorities or Importance Levels:	void update(int priority, const std::string& message)
			* 10. Asynchronous:						std::future<void> update()
			* 11. Subject reference/pointer:		void update(Subject* subject)
			* 12. EventData:						void update(const EventData& event_data)
			*		The notification may contain the specific event type, the value of the new state, and other useful data.
			* 13. Generic Interface:				void update(IEvent* event)
			*	class StateChangeEvent : public IEvent {
				public:
					int new_state;
				};

				void MyObserver::update(IEvent* event) {
					if(auto change_event = dynamic_cast<StateChangeEvent*>(event)) {}
				}
			* 14. Signal: Boost library
			*
			class Subject {
			private:
				boost::signals2::signal<void()> signal_;

			public:
				void notifyObservers() {
					signal_();
				}

				boost::signals2::connection connect(boost::signals2::slot<void()> slot) {
					return signal_.connect(slot);
				}
			};
			*
			*
			*/

			// Helper Type alias for different Update realizations
			using IObserverWeakMsg = IObserverWeakHub<std::string>;

			template<typename EnumType = ObserverEmptyEnum>
			//requires std::is_enum_v<EnumType>	// c++20 requires
			using IObserverWeakEnum = IObserverWeakHub<EnumType>;

			template<typename StateType = SubjectStateEmpty>
			using IObserverWeakState = IObserverWeakHub<StateType>;

			template<typename ContextType>
			using IObserverWeakContext = IObserverWeakHub<ContextType>;
			using IObserverWeakCallback = IObserverWeakHub<std::function<void()>>;


			/**
			 * Abstract. Stateless. Subject with weak_ptr dependencies.
			 * Alternative name - Publisher.
			 * weak_ptr helps to prevent dangling pointers and lapsed listener problem.
			 */
			template<typename UpdateDataType>
			class ISubjectWeakHub {
			public:
				using WeakPtrIObserverWeakHub = std::weak_ptr<IObserverWeakHub<UpdateDataType>>;

			protected:
				ISubjectWeakHub() = default;
				ISubjectWeakHub(const ISubjectWeakHub&) = delete; // C.67	C.21
				ISubjectWeakHub& operator=(const ISubjectWeakHub&) = delete;
				ISubjectWeakHub(ISubjectWeakHub&&) noexcept = delete;
				ISubjectWeakHub& operator=(ISubjectWeakHub&&) noexcept = delete;
			public:
				virtual ~ISubjectWeakHub() = default;


				/** Update all attached observers */
				virtual void NotifyObservers(const std::string& message) const = 0;

				/**
				 * Add Observer to list of notification in Subject.
				 * There is shared_ptr<ObserverT> outside of class. Subject stores weak_ptr to that Observer.
				 */
				virtual void AttachObserver(const WeakPtrIObserverWeakHub observer_ptr) = 0;
				// not const, cause must attach subject in observer ptr

				/** Detach observer from notifying list */
				virtual void DetachObserver(const WeakPtrIObserverWeakHub observer_ptr) = 0;
			};
			/*
			* Design choices. Params in functions. Weak_ptr vs shared_ptr:
			* Best) weak_ptr in params gives guaranty, that we won't use destructed objects.
			* 2) If we use const shared_ptr& outside code must guaranty, that object will be alive, while function is working.
			* 3) If we use shared_ptr we increment strong counter of shared_ptr, increasing life cycle of object.
			*/


//=============================Base Concrete Classes==============================================================

			/**
			 * Concrete. Simple observer class realize Upate() and Update(Subject&) functions.
			 * Class with empty function members. Stub for inheritance and name hiding.
			 *
			 * Invariant: don't attach expired weak_ptr. Value type must be weak_ptr. Mustn't duplicate weak_ptr.
			 * @tparam EnumType is enum class for messages for one of realization of Update.
			 *
			 * [[Testlevel(0)]]
			 */
			template<typename UpdateDataType>
			class ObserverWeakHubDefault : public IObserverWeakHub<UpdateDataType> {
			public:
				ObserverWeakHubDefault() = default;
				// Observer may be constructed without observable Subjects

			protected:
				ObserverWeakHubDefault(const ObserverWeakHubDefault&) = delete; // C.67	C.21
				ObserverWeakHubDefault& operator=(const ObserverWeakHubDefault&) = delete;
				ObserverWeakHubDefault(ObserverWeakHubDefault&&) noexcept = delete;
				ObserverWeakHubDefault& operator=(ObserverWeakHubDefault&&) noexcept = delete;
			public:
				~ObserverWeakHubDefault() override = default;

				/** Update the information about Subject */
				void Update() override {};

				/**
				* Update the information about Subject
				*
				* @param message	use in switch statements
				*/
				void Update(const std::string& message = "") override {
					// Implement specific update logic here
					// For example, logging or state update
					// For now, empty implementation
					//(void)message;  // suppress unused parameter warning
				};

				/** Generic Update the information about Subject */
				void Update(UpdateDataType data) override {};

				/** Generic Update the information about Subject */
				void Update(const UpdateDataType& data) override {};

				/** Generic Update the information about Subject */
				void Update(UpdateDataType&& data) override {};

			};	// !class ObserverWeak
			// Design choices: subject weak_ptr is necessary, when you need get more data for updating process.
			// In most cases subject weak_ptr is not needed.
			//
			// Design choices: param of template maybe: Types...	to give opportunity to send more args to Update function.


			/** Subject Stub. Class with default minimal realization. */
			template<typename UpdateDataType>
			class SubjectWeakHubDefault {
			public:
				using WeakPtrIObserverWeakHub = std::weak_ptr<IObserverWeakHub<UpdateDataType>>;

			protected:
				SubjectWeakHubDefault() = default;
				SubjectWeakHubDefault(const SubjectWeakHubDefault&) = delete; // C.67	C.21
				SubjectWeakHubDefault& operator=(const SubjectWeakHubDefault&) = delete;
				SubjectWeakHubDefault(SubjectWeakHubDefault&&) noexcept = delete;
				SubjectWeakHubDefault& operator=(SubjectWeakHubDefault&&) noexcept = delete;
			public:
				virtual ~SubjectWeakHubDefault() = default;


				/** Update all attached observers */
				virtual void NotifyObservers(const std::string& message) const {};

				/**
				 * Add Observer to list of notification in Subject.
				 * There is shared_ptr<ObserverT> outside of class. Subject stores weak_ptr to that Observer.
				 */
				virtual void AttachObserver(const WeakPtrIObserverWeakHub observer_ptr) {};
				// not const, cause must attach subject in observer ptr

				/** Detach observer from notifying list */
				virtual void DetachObserver(const WeakPtrIObserverWeakHub observer_ptr) {};
			};
			// Design: for template notification with ExecPolicy in SubjectWeakHub and name hiding. (TODO: refactor?)


			/**
			 * Concrete. Observers will be notified on Subject state changes.
			 * One Subject can has many Observers.
			 * Gives opportunity to communicate with different layers of the application.
			 * Plus, dependency is weak, cause it is dependency to an abstract object.
			 * weak_ptr helps to prevent dangling pointers and lapsed listener problem.
			 *
			 * Not recommend to use vector, as bad performance Detach.
			 * Don't forget to Notify Observers, where it is necessary, when Subject state changes.
			 *
			 * Invariant: don't attach & store expired weak_ptr. Mustn't duplicate weak_ptr.
			 *
			 * Complexity: maybe best for std::set = O(log n). But CleanOps are O(n)
			 *
			 * @tparam UpdateDataType		type of update data in param of update function in observer
			 * @tparam ExecPolicyType		Execution policy (e.g., std::execution::sequenced_policy)
			 * @tparam ContainerType_t		Container type holding weak_ptrs to observers
			 *
			 * [[Testlevel(0)]]
			 */
			template<typename UpdateDataType,
					 typename ContainerType = std::set<std::weak_ptr<IObserverWeakHub<UpdateDataType>>> >
			//requires std::is_execution_policy_v<ExecPolicyType> && std::is_enum_v<EnumType>	// c++20 requires
			class WeakCallbackSubject : public SubjectWeakHubDefault<UpdateDataType>
				//public std::enable_shared_from_this<SubjectWeak<typename ExecPolicyType, typename ContainerType_t>>
			{
			public:
				using value_type = typename ContainerType::value_type;
				using WeakPtrIObserverWeakHub = std::weak_ptr<IObserverWeakHub<UpdateDataType>>;	// not from derived for clarity
				//using IteratorNExpiredCount = std::pair<decltype(observers_.end()), size_t>;

				static_assert(std::is_same_v<value_type, WeakMemberFunction>, "Container elements must be .");


				// Container Types Alternatives
				using ContainerSet			= std::set<WeakPtrIObserverWeakHub, std::owner_less<WeakPtrIObserverWeakHub>>;
				using ContainerList			= std::list<WeakPtrIObserverWeakHub>;
				using ContainerForwardList	= std::forward_list<WeakPtrIObserverWeakHub>;
				//using ContainerVector		= std::vector<WeakPtrIObserverWeakHub>;
				//using ContainerDeque		= std::deque<WeakPtrIObserverWeakHub>;

				SubjectCommand() = default;
				// Subject may be constructed without observable Subjects

				template<typename IteratorType, typename ExecPolicyType>
				SubjectCommand(IteratorType begin, IteratorType end, ExecPolicyType policy = std::execution::seq) {
					AttachObserver(begin, end, policy);
				}

				template<typename ExecPolicyType>
				explicit SubjectCommand(const std::initializer_list<WeakPtrIObserverWeakHub>& init_list,
										ExecPolicyType policy = std::execution::seq) {
					//: observers_{ init_list.begin(), init_list.end() } {
					AttachObserver(init_list.begin(), init_list.end(), policy);
				};

				template<typename ContainerType, typename ExecPolicyType>
				explicit SubjectCommand(const ContainerType& container,
										ExecPolicyType policy = std::execution::seq) {
					static_assert(std::is_same_v<typename ContainerType::value_type, WeakPtrIObserverWeakHub>,
								"Elements of container, when AttachObserver must be weak_ptr to observer interface");
					AttachObserver(container.begin(), container.end(), policy);
				};

				template<typename ExecPolicyType>
				explicit SubjectCommand(const WeakPtrIObserverWeakHub observer_ptr,
										ExecPolicyType policy = std::execution::seq) {
					AttachObserver(observer_ptr, policy); // No differrence with std::move(observer_ptr)
				};

			protected:
				SubjectCommand(const SubjectCommand&) = delete; // C.67	C.21
				SubjectCommand& operator=(const SubjectCommand&) = delete;
				SubjectCommand(SubjectCommand&&) noexcept = delete;
				SubjectCommand& operator=(SubjectCommand&&) noexcept = delete;
			public:
				~SubjectCommand() override = default;

				// TODO: write notify methods for all stubs in ObserverHub class. With all variants of Update fn of observer.

				/*
				 * Update all attached observers in main thread.
				 *
				 * Complexity: set = O(log n), Other containers = O(n).
				 *
				 * @param message	message with information needed for Update.
				 */
				template<typename ExecPolicyType>
				void NotifyObservers(const std::string& message = "",
									const bool run_in_new_thread = true,
									ExecPolicyType policy = std::execution::seq) const {
					auto observer_update_fn = [&message](std::shared_ptr<IObserverWeakHub<UpdateDataType>> observer_ptr) {
						// observer_ptr in lambda is shared_ptr, cause in GenericNotify it is locked
						observer_ptr->Update(message);
					};
					MutexNotifyObserversNClean(observer_update_fn, policy);
				};

				/*
				 * Update all attached observers in main thread.
				 *
				 * Complexity: set = O(log n), Other containers = O(n).
				 *
				 * @param message		message with information needed for Update.
				 * @param new_thread	one thread from thread pool.
				 */
				template<typename ExecPolicyType>
				void NotifyObservers(std::thread& new_thread,
									const std::string& message = "",
									ExecPolicyType policy = std::execution::seq) const {
					auto observer_update_fn = [&message](std::shared_ptr<IObserverWeakHub<UpdateDataType>> observer_ptr) {
						// observer_ptr in lambda is shared_ptr, cause in GenericNotify it is locked
						observer_ptr->Update(message);
						};
					MutexNotifyObserversNClean(observer_update_fn, policy);
				};


				// TODO: Notification in another thread. Wait the end of notification process
				// TODO: bool run_in_new_thread = false
				// In new std::thread
				// Async variant

				/*void Update(UpdateDataType data) override {};
				void Update(const UpdateDataType& data) override {};
				void Update(UpdateDataType&& data) override {};*/


				/*
				 * Update all attached observers.
				 *
				 * Complexity: list = O(n). set = O(log n)
				 *
				 * @param message	message with information needed for Update.
				 */
				template<typename ExecPolicyType>
				void NotifyObservers(ExecPolicyType policy = std::execution::seq) const {
					auto observer_update_fn = [](std::shared_ptr<IObserverWeakHub<UpdateDataType>> observer_ptr) {
						observer_ptr->Update();
					};
					MutexNotifyObserversNClean(observer_update_fn, policy);
				};


				/**
				* Add Multiple Observer to list of Observers.
				* Only alive weak_ptr can be attached to container and only that is not duplicates.
				*
				* Complexity: O(K), K - count of attachable observers.
				* Complexity of adding each observer: maybe O(1), but is O(n), cause CleanOps.
				*
				* @param attachable_begin		start of range of observers, that will be added to Subject
				* @param attachable_end			end of range of observers, that will be added to Subject
				*/
				template<typename IteratorType, typename ExecPolicyType>
				void AttachObserver(IteratorType attachable_begin, IteratorType attachable_end,
									ExecPolicyType policy = std::execution::seq)
				{ // Main logic in this function to decrease count of function call, so lower overhead resources for call of fn.
					if (attachable_begin == attachable_end) { return; }	// Precondition
					static_assert(std::is_same_v<decltype(*attachable_begin), WeakPtrIObserverWeakHub>,
									"Iterator be dereferencable to weak_ptr to observer interface");

					auto attach_observer_fn = [this, &policy](const auto& observer_ptr) {
						auto has_observer{ HasObserver(observer_ptr, policy) };
						if (!has_observer.first) { // Duplicate control. Mustn't duplicate weak_ptr
							GenericAddElement(observers_, observer_ptr);		// O(1)
						}
						UpdateExpiredObserversCount(has_observer.second);
					}; // !lambda
					{
						std::unique_lock lock{ observers_shared_mtx_ };
						std::for_each(policy, attachable_begin, attachable_end, attach_observer_fn); // write	O(n)
					} // !lock
					CleanFoundExpiredObservers(policy);							// O(n)
				};

				/**
				 * Add Observer to list of Observers. Wrapper.
				 * Only alive weak_ptr can be attached to container and only that is not duplicates.
				 *
				 * Complexity: O(n)
				 */
				template<typename ExecPolicyType>
				inline void AttachObserver(const std::initializer_list<WeakPtrIObserverWeakHub>& init_list,
											ExecPolicyType policy = std::execution::seq) {
					if (init_list.size() == 0) { return; }	// Precondition
					AttachObserver(init_list.begin(), init_list.end(), policy);
				};

				/**
				 * Add Observer to list of Observers. Wrapper.
				 * Only alive weak_ptr can be attached to container and only that is not duplicates.
				 *
				 * Complexity: O(n)
				 */
				template<typename ContainerType, typename ExecPolicyType>
				inline void AttachObserver(const ContainerType& container, ExecPolicyType policy = std::execution::seq) {
					if (container.empty()) { return; }	// Precondition
					static_assert(std::is_same_v<typename ContainerType::value_type, WeakPtrIObserverWeakHub>,
									"Elements of container, when AttachObserver must be weak_ptr to observer interface");

					AttachObserver(container.begin(), container.end(), policy);
				};

				/**
				 * Add Observer to list of Observers. Wrapper.
				 * Only alive weak_ptr can be attached to container and only that is not duplicates.
				 *
				 * Complexity: O(n)
				 *
				 * @param observer_ptr	weak pointer to observer.
				 */
				template<typename ExecPolicyType>
				inline void AttachObserver(const WeakPtrIObserverWeakHub observer_ptr,
											ExecPolicyType policy = std::execution::seq) {
					if (observer_ptr.expired()) { return; }	// Precondition
					AttachObserver({ observer_ptr }, policy);	// for HasObserver
				};
				// not const, cause must attach subject in observer ptr


				/**
				 * Detach Multiple Observers.
				 * Can Detach only not expired weak_ptr, cause equality defined on alive objects.
				 *
				 * Complexity: O(k*n), k - count of detachable observers.
				 * Complexity of detaching each observer: maybe O(n), but is O(n), cause CleanOps.
				 *
				 * @param erasable_begin	begin of container with observers, that must be detached
				 * @param erasable_end		end of container with observers, that must be detached
				 */
				template<typename IteratorType, typename ExecPolicyType>
				inline void DetachObserver(IteratorType erasable_begin, IteratorType erasable_end,
											ExecPolicyType policy = std::execution::seq) {
					if (erasable_begin == erasable_end) { return; }	// Precondition
					static_assert(std::is_same_v<decltype(*erasable_begin), WeakPtrIObserverWeakHub>,
									"Iterator be dereferencable to weak_ptr to observer interface");

					size_t expired_count{};
					auto detach_observer_fn = [this, expired_count](const auto& observer_ptr) {
						// Can Detach only alive objects
						expired_count = EraseEqualWeakPtr(observers_, observer_ptr, policy);			// O(n)
						UpdateExpiredObserversCount(expired_count);
					}; // !lambda

					{
						std::unique_lock lock{ observers_shared_mtx_ }; // write
						std::for_each(policy, erasable_begin, erasable_end, detach_observer_fn); // write	O(k*n)
						CleanFoundExpiredObservers(policy);												// O(n)
					} // !lock
				};

				/**
				 * Detach Observer. Wrapper.
				 * Can Detach only not expired weak_ptr, cause equality defined on alive objects.
				 *
				 * Complexity: O(n)
				 */
				template<typename ExecPolicyType>
				inline void DetachObserver(const std::initializer_list<WeakPtrIObserverWeakHub>& init_list,
											ExecPolicyType policy = std::execution::seq) {
					if (init_list.size() == 0) { return; }	// Precondition
					DetachObserver(init_list.begin(), init_list.end(), policy);
				};

				/**
				 * Detach Observer. Wrapper.
				 * Can Detach only not expired weak_ptr, cause equality defined on alive objects.
				 *
				 * Complexity: O(n)
				 */
				template<typename ContainerType, typename ExecPolicyType>
				inline void DetachObserver(const ContainerType& container,
											ExecPolicyType policy = std::execution::seq) {
					if (container.empty()) { return; }	// Precondition
					static_assert(std::is_same_v<typename ContainerType::value_type, WeakPtrIObserverWeakHub>,
									"Elements of container, when AttachObserver must be weak_ptr to observer interface");

					DetachObserver(container.begin(), container.end(), policy);
				};

				/**
				 * Detach Observer.
				 * Can Detach only not expired weak_ptr, cause equality defined on alive objects.
				 *
				 * Complexity: O(n)
				 *
				 * @param observer_ptr weak pointer to observer.
				 */
				template<typename ExecPolicyType>
				inline void DetachObserver(const WeakPtrIObserverWeakHub observer_ptr,
											ExecPolicyType policy = std::execution::seq) {
					std::unique_lock lock{ observers_shared_mtx_ }; // write
					// Can Detach only alive objects
					size_t expired_count{ EraseEqualWeakPtr(observers_, observer_ptr, policy) };	// O(n)
					UpdateCountNCleanExpiredObservers(expired_count, policy);
				};


				/**
				 * Detach all expired weak_ptr objects in container
				 *
				 * Complexity: O(n)
				 */
				template<typename ExecPolicyType>
				inline void CleanupAllExpiredObservers(ExecPolicyType policy = std::execution::seq) const {
					std::unique_lock lock{ observers_shared_mtx_ }; // write
					EraseAllExpired(observers_, policy);
					// if subject is expired, it is deleted, so we don't need to detach observer in subject
				};

				/**
				 * Check if there is observer in Subject.
				 * Cleanup expired weak_ptr subjects in container.
				 *
				 * Complexity: O(n)
				 *
				 * \param subject_ptr
				 * \param policy
				 * \return
				 */
				template<typename ExecPolicyType>
				inline bool HasObserverNClean(const WeakPtrIObserverWeakHub observer_ptr,
										ExecPolicyType policy = std::execution::seq) const {
					std::pair<bool, size_t> has_observer{ HasObserver(observer_ptr, policy) };
					UpdateCountNCleanExpiredObservers(has_observer.second, policy);
					return has_observer.first;
				};

			private:
				/* Complexity: O(n) */
				template<typename ExecPolicyType>
				inline std::pair<bool, size_t> HasObserver(const WeakPtrIObserverWeakHub observer_ptr,
															ExecPolicyType policy = std::execution::seq) const {
					std::pair<decltype(observers_.end()), size_t> find_result{};
					{
						std::shared_lock lock{ observers_shared_mtx_ }; // read
						find_result = FindEqualWeakPtr(observers_, observer_ptr, policy);	// O(n)
					} // !lock

					bool has_observer = find_result.first != observers_.end();
					return std::make_pair(has_observer, find_result.second); // iterators maybe invalidated after cleanup
				};

				/**
				 * Detach all expired weak_ptr objects in container. Concurrent sync by mutex.
				 * Helps to minimize count of calls Cleanup in concurrent usage of this class.
				 * Cause many blocked threads may found equal expired weak_ptr.
				 * But cleanup is only done by counter found_expired_observers_.
				 * Locked with unique mutex.
				 *
				 * Complexity: O(n)
				 */
				template<typename ExecPolicyType>
				inline void CleanFoundExpiredObservers(ExecPolicyType policy = std::execution::seq) const {
					if (found_expired_observers_.load(std::memory_order_relaxed) > 0) { // precondition
						{
							std::unique_lock lock{ observers_shared_mtx_ };
							// Cleanup expired weak_ptr
							EraseNExpiredWeakPtr(observers_, found_expired_observers_, policy); // write	O(n)
						} // !lock
						ResetExpiredObserversCount();
					}
				}
				// TODO: Decrease the Complexity of cleaning to make attach & detach complexity = O(log n).

				/** Modify count of expired observers, if new count bigger */
				inline void UpdateExpiredObserversCount(const size_t new_count) const noexcept {
					if (new_count > found_expired_observers_) {
						found_expired_observers_ = new_count;
					}
                }
				/** Set count of expired observer to 0 */
				inline void ResetExpiredObserversCount() const noexcept {
					found_expired_observers_ = 0;
				}
				// TODO: replace assign of atomic with quicker ops = load(relaxed) + compare_exchange_weak

				/**
				 * Update count of expired observers and clean observers container.
				 * Locked with mutex.
				 *
				 * Complexity: O(n)
				 */
				template<typename ExecPolicyType>
				inline void UpdateCountNCleanExpiredObservers(const size_t new_count,
															ExecPolicyType policy = std::execution::seq) const {
					UpdateExpiredObserversCount(new_count);
					CleanFoundExpiredObservers(policy);
				}

				/**
				 * Thread safe. Notify all observers of Subject. In same thread.
				 * Used mutex to block shared resource.
				 *
				 * Complexity: O(n)
				 */
				template<typename UpdateFunctionType, typename ExecPolicyType>
				void MutexNotifyObserversNClean(UpdateFunctionType observer_update_fn,
												ExecPolicyType policy = std::execution::seq) const {
					size_t expired_count{};
					{
						std::shared_lock lock{ observers_shared_mtx_ };
						expired_count = GenericNotifyWeakObservers(observers_, observer_update_fn, policy); // read	O(n)
					} // !lock

					UpdateCountNCleanExpiredObservers(expired_count, policy);						// O(n)
				}

				/**
				 * Thread safe. Notify all observers of Subject in new thread of execution.
				 * Used mutex to block shared resource.
				 */
				template<typename ThreadPoolType, typename UpdateFunctionType, typename ExecPolicyType>
				void ThreadNotifyObserversNClean(ThreadPoolType& thread_pool,
												UpdateFunctionType observer_update_fn,
												ExecPolicyType policy = std::execution::seq) const {
					//thread_pool.enqueue(&SubjectWeakHub::MutexNotifyObserversNClean, this, observer_update_fn, policy);
					thread_pool.enqueue([this, &observer_update_fn, &policy]() {
											MutexNotifyObserversNClean(observer_update_fn, policy);
					});
				}
				/**
				 * Thread safe. Notify all observers of Subject in new thread of execution.
				 * Used mutex to block shared resource.
				 */
				template<typename ThreadPoolType, typename UpdateFunctionType, typename ExecPolicyType>
				void ThreadNotifyObserversNClean(UpdateFunctionType observer_update_fn,
													ExecPolicyType policy = std::execution::seq) const {
					std::thread notify_thread(&SubjectCommand::MutexNotifyObserversNClean, this, observer_update_fn, policy);
					notify_thread.detach();	// Thread will work separately
					// instead of detach thread object can be global, so you can monitor it
				}

				/**
				 * Thread safe. Notify all observers of Subject in new thread of execution.
				 * Used mutex to block shared resource.
				 */
				template<typename ThreadPoolType, typename UpdateFunctionType, typename ExecPolicyType>
				void ThreadNotifyObserversNCleanAsync(UpdateFunctionType observer_update_fn,
														ExecPolicyType policy = std::execution::seq) const {
					std::future result{ std::async(std::launch::async, &SubjectCommand::MutexNotifyObserversNClean,
											this, observer_update_fn, policy) };
				}


//-----------------------SubjectWeakHub Data-------------------------------------------------

				/**
				 * List of observers, that will be attach to Subject.
				 * Subject is not interested in owning of its Observers.
				 * So can be used weak_ptr, created from shared_ptr.
				 *
				 * Design: If there is too many subjects with few observers you can use hash table.
				 */
				mutable ContainerType observers_{}; // mutable is for const fn notify function cleaning of expired weak_ptr
				// Is not const, cause Attach, Detach functions call.

				/**
				 * Mutex for thread safety class operations.
				 * Helps to read data without lock. And to write data with lock.
				 */
				mutable std::shared_mutex observers_shared_mtx_{};
				/* Maybe order of concurent thread calls must be detach, attach, notify. First delete, then add, then notify */

				/**
				 * Atomic variable for concurrent auto cleaning of found by read operation expired observers.
				 * Necessary for decreasing number of calls cleanup function.
				 */
				mutable std::atomic_size_t found_expired_observers_{};

				/** Execution policy, that will work for all functions */
				//ExecPolicyType policy_{};

			};	// !class SubjectWeakHub
			/*
			* list
			* Pros: Efficient in terms of insertion and removal (O(1)), maintains order, can store duplicates.
			* - Good for scenarios where observers frequently join or leave.
			* Cons: Higher memory overhead due to pointers and less cache-friendly compared to arrays.
			* - Accessing elements by index is slower (O(n)).
			*
			* vector
			* Pros: Contiguous memory layout (better cache performance), faster access times using indices.
			* - Simple and straightforward to implement.
			* - If the number of observers is relatively stable and you do not expect frequent additions/removals, this is a good choice.
			* Cons: Inefficient for frequent insertions and removals (elements need to be shifted). Can be costly (O(n))
			*		since elements may need to be shifted.
			* - You will need to manually remove expired weak pointers when notifying observers.
			*
			* forward_list
			* Pros: Lower memory overhead than `std::list` because it only has one pointer per element, efficient
			*	for insertions and removals at the front.
			* Cons: Cannot access elements in the middle without iterating from the start, which makes it less flexible for random removals.
			*
			*
			* set
			* Pros: Automatically manages unique observers, allows for fast insertion and removal (O(log n)), sorted order.
			* Cons: No duplicates allowed, more memory overhead due to tree structure.
			*
			* unordered_set
			* Pros: Automatically handles uniqueness (no duplicates).
			* - Fast average-time complexity for insertions and removals (O(1)).
			* - The std::owner_less comparator allows for comparisons based on the underlying shared_ptr, which is useful for weak pointers.
			* Cons: Does not maintain any order of observers.
			* - Unstable for weak_ptr, cause if expired hash will change, so undefined behavior.
			* - Slightly higher memory overhead due to hash table.
			* - Stability of weak pointers: std::weak_ptr does not have a stable identity over its lifetime because if the object it
			*		points to expires, the internals of the weak_ptr can change, including the hash used in unordered_set. This instability
			*		makes it unreliable as keys in an unordered associative container.
			*
			* Recommendation
			* For scenarios where observers are frequently added and removed, **std::list** or **std::set** are generally the best choices:
			* - Use **std::list** if you need to allow duplicates and maintain insertion order.
			* - Use **std::set** if you need to ensure that observers are unique and you don’t mind the overhead for maintaining sorted order.
			*/



			struct StateSingle {
				int a_{ 0 };
				int b_{ 0 };
			};


			// Design Choices: Aggregation is better in the meaning of design of software.

			//template<typename ExecPolicyType = std::execution::sequenced_policy>
			//class MySubjectAggregation {
			//public:
			//	using WeakPtrIObserverWeak		= SubjectWeak<ExecPolicyType>::WeakPtrIObserverWeak;
			//	using SharedPtrISubjectWeak		= std::shared_ptr<ISubjectWeak>;

			//	void NotifyObservers(const std::string& message = "") const {
			//		if (subject_ptr_) { subject_ptr_->NotifyObservers(message); }
			//	}
			//	inline void AttachObserver(const WeakPtrIObserverWeak observer_ptr) {
			//		if (subject_ptr_) { subject_ptr_->AttachObserver(observer_ptr); }
			//	}
			//	inline void DetachObserver(const WeakPtrIObserverWeak observer_ptr) {
			//		if (subject_ptr_) { subject_ptr_->DetachObserver(observer_ptr); }
			//	}
			//	inline void DetachAllExpired() {
			//		if (subject_ptr_) { subject_ptr_->DetachAllExpired(); }
			//	}
			//	inline bool HasObserver(const WeakPtrIObserverWeak observer_ptr) const {
			//		if (subject_ptr_) { return subject_ptr_->HasObserver(observer_ptr); }
			//		return false;
			//	}


			//	inline void set_subject(const SharedPtrISubjectWeak subject_ptr_p) noexcept { subject_ptr_ = subject_ptr_p; }
			//	inline std::weak_ptr<ISubjectWeak> subject() const noexcept { return subject_ptr_; };

			//	StateSingle state_{};	// is public for testing

			//private:

			//	SharedPtrISubjectWeak subject_ptr_{};
			//};

			//class MyObserverAggregation {
			//public:
			//	using SharedPtrIObserverWeak = std::shared_ptr<IObserverWeak>;

			//	/**
			//	 * Update the information about Subject.
			//	 */
			//	void Update(const std::string& message = "") {
			//		if (observer_ptr_) { observer_ptr_->Update(message); }

			//		// + Some Actions
			//	};

			//	inline void set_observer(const SharedPtrIObserverWeak observer_ptr_p) noexcept {
			//		observer_ptr_ = observer_ptr_p;
			//	}
			//	inline std::weak_ptr<IObserverWeak> observer() const noexcept { return observer_ptr_; };

			//	StateSingle state_{};	// is public for testing

			//private:

			//	SharedPtrIObserverWeak observer_ptr_{};
			//};



			//template<typename ExecPolicyType = std::execution::sequenced_policy>
			//class MySubjectInheritance : public SubjectWeak<ExecPolicyType> {
			//public:
			//	using SybjectType = SubjectWeak<ExecPolicyType>;
			//	using WeakPtrIObserverWeak = SybjectType::WeakPtrIObserverWeak;

			//	MySubjectSingle() = default;
			//protected:
			//	MySubjectSingle(const MySubjectSingle&) = delete; // C.67	C.21
			//	MySubjectSingle& operator=(const MySubjectSingle&) = delete;
			//	MySubjectSingle(MySubjectSingle&&) noexcept = delete;
			//	MySubjectSingle& operator=(MySubjectSingle&&) noexcept = delete;
			//public:
			//	~MySubjectSingle() override = default;

			//	StateSingle state_{};	// is public for testing
			//};


			//class MyObserverInheritance : public ObserverWeak<> {
			//public:
			//	/*explicit MyObserverSingle(WeakPtrISubjectType subject_ptr) : ObserverWeakMulti(subject_ptr) {
			//	}*/

			//	MyObserverInheritance() = default;
			//protected:
			//	MyObserverInheritance(const MyObserverInheritance&) = delete; // C.67	C.21
			//	MyObserverInheritance& operator=(const MyObserverInheritance&) = delete;
			//	MyObserverInheritance(MyObserverInheritance&&) noexcept = delete;
			//	MyObserverInheritance& operator=(MyObserverInheritance&&) noexcept = delete;
			//public:
			//	~MyObserverInheritance() override = default;

			//	/**
			//	 * Update the information about Subject.
			//	 */
			//	void Update(const std::string& message = "") override {
			//		// Some Actions
			//	};

			//	StateSingle state_{};	// is public for testing
			//};

		} // !observer_weak_cmd


	} // !namespace behavioral
} // !namespace pattern

#endif // !OBSERVER_CALLBACK_HPP
