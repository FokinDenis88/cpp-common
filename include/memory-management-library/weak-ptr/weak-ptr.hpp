#ifndef WEAK_PTR_HPP
#define WEAK_PTR_HPP


#include <algorithm> // remove_if
#include <execution> // execution policies
#include <memory>
#include <type_traits> // is_same_v
#include <tuple> // tie
#include <utility> // pair

// Containers
#include <forward_list>
#include <set>

#include "containers-library/generic-container.hpp"


namespace util {
	/* Work with weak_ptr in containers. Process weak_ptr Container. Erase. */

	using ExpiredFnT = decltype([](const auto& value_weak_ptr) { return value_weak_ptr.expired(); });

//======================EqualOwner======================================================

	/** Functor to compare objects via shared_ptr inside weak_ptr */
	template<typename ValueT>
	struct EqualOwner {
		/**
		 * Compares if two shared_ptr have the same owner.
		 *
		 * Complexity: amortized O(1)
		 *
		 * @param	shared_ptr search ptr. Is shared for decreasing number of locks.
		 * @param	current_ptr
		 * @return	result of equality check
		 */
		inline bool operator()(const std::shared_ptr<ValueT>& lhs,
								const std::shared_ptr<ValueT>& rhs) const noexcept {
			// a==b, if (not a<b) and (not b<a)
			return lhs && rhs && !lhs.owner_before(rhs) && !rhs.owner_before(lhs);
		}

		/**
		* Compares if two weak_ptr are alive and have the same owner, stored object.
		* Use, when first weak_ptr is locked for performance.
		*
		* Complexity: amortized O(1)
		*
		* @param searched_shared		search ptr. Is shared for decreasing number of locks.
		* @param current_ptr			rhs comparation object
		* @return						result of equality check
		*/
		inline bool operator()(const std::weak_ptr<ValueT>& lhs,
								const std::weak_ptr<ValueT>& rhs) const noexcept {
			auto lhs_shared = lhs.lock();
			auto rhs_shared = rhs.lock();

			// a==b, if (not a<b) and (not b<a)
			return lhs_shared && rhs_shared && !lhs_shared.owner_before(rhs_shared) &&
					!rhs_shared.owner_before(lhs_shared);
		}

		/**
		* Compares if two weak_ptr are alive and have the same owner, stored object.
		* Use this version when you already have a locked shared_ptr.
		*
		* Complexity: amortized O(1)
		*
		* @param searched_shared		search ptr. Is shared for decreasing number of locks.
		* @param current_ptr			rhs comparation object
		* @return						result of equality check
		*/
		inline bool operator()(const std::shared_ptr<ValueT>& searched_shared,
								const std::weak_ptr<ValueT>& current_ptr) const noexcept {
			auto current_shared = current_ptr.lock();

			// a==b, if (not a<b) and (not b<a)
			return searched_shared && current_shared && !searched_shared.owner_before(current_shared) &&
				!current_shared.owner_before(searched_shared);
		}
	}; // !struct EqualOwner

	/**
	* Compares if two weak_ptr are alive and have the same owner, stored object.
	* Use, when first weak_ptr is locked for performance.
	*
	* Complexity: amortized O(1)
	*
	* @param searched_shared		search ptr. Is shared for decreasing number of locks.
	* @param current_ptr			rhs comparation object
	* @return						result of equality check
	*/
	template<typename ValueT>
	inline bool EqualOwnerFn(const std::weak_ptr<ValueT>& lhs,
									  const std::weak_ptr<ValueT>& rhs) noexcept {
		return EqualOwner<ValueT>()(lhs, rhs);
	}

	/**
	* Compares if two weak_ptr are alive and have the same owner, stored object.
	* Use this version when you already have a locked shared_ptr.
	*
	* Complexity: amortized O(1)
	*
	* @param searched_shared		search ptr. Is shared for decreasing number of locks.
	* @param current_ptr			rhs comparation object
	* @return						result of equality check
	*/
	template<typename ValueT>
	inline bool EqualOwnerFn(const std::shared_ptr<ValueT>& searched_shared,
									const std::weak_ptr<ValueT>& current_ptr) noexcept {
		return EqualOwner<ValueT>()(searched_shared, current_ptr);
	}

	/** A specialized version of the hash function for std::weak_ptr */
	template<typename ValueT>
	struct HashWeakPtr { // ! not the best choice. will be many collisions, if many weak_ptr expired.
		size_t operator()(const std::weak_ptr<ValueT>& wp) const noexcept {
			if (auto sp = wp.lock()) {
				return std::hash<std::shared_ptr<ValueT>>{}(sp);
			}
			return 0;
		}
	};
	// Hash for unordered_set, unordered_map must be stable and not to change with expiration of weak_ptr

//============================Find=====================================================================

	/**
	 * Find first weak_ptr, that is alive and has same stored pointer.
	 * Without auto clean of expired weak_ptrs.
	 *
	 * Complexity: O(n).
	 * Mutex: read
	 *
	 * @return		iterator to equal weak_ptr or to end.
	 */
	template<typename ContainerT, typename ValueT, typename ExecPolicyT>
	inline auto FindEqualOwner(const ContainerT& container,
								const std::weak_ptr<ValueT> searched_ptr,
								ExecPolicyT policy = std::execution::seq)
			-> decltype(container.end())
	{
		using value_type = typename ContainerT::value_type;
		static_assert(std::is_same_v<value_type, std::weak_ptr<ValueT>>,
					"The type mismatch between container elements and weak_ptr");

		auto end{ container.end() };
		if (container.empty()) { return end; } // precondition

		auto searched_shared = searched_ptr.lock();
		if (!searched_shared) { return end; }

		return std::find_if(policy, container.begin(), end,
			[&searched_shared](const auto& current_ptr) {
				return EqualOwnerFn(searched_shared, current_ptr);			// O(1)
			}
		); // lambda
	}

	/**
	 * Find first weak_ptr, that is alive and has same stored pointer.
	 * With auto cleanup of expired weak_ptrs.
	 *
	 * Complexity: O(n).
	 * Mutex: read
	 *
	 * @return		iterator to equal weak_ptr or to end.
	 */
	template<typename ContainerT, typename ValueT, typename ExecPolicyT>
	inline auto FindEqualOwnerNClean(const ContainerT& container,
									const std::weak_ptr<ValueT> searched_ptr,
									ExecPolicyT policy = std::execution::seq)
			-> decltype(container.end())
	{
		using value_type = typename ContainerT::value_type;
		static_assert(std::is_same_v<value_type, std::weak_ptr<ValueT>>,
					"The type mismatch between container elements and weak_ptr");

		auto end{ container.end() };
		if (container.empty()) { return end; } // precondition

		auto searched_shared = searched_ptr.lock();
        if (!searched_shared) { return end; }

		auto it_current{ container.begin() };
		bool found_equal{ false };
		do { // find expired or equal
			it_current = std::find_if(policy, it_current, end,					//O(n)
				[&searched_shared](const auto& current_weak) { // expired or equal
                    if (current_weak.expired()
						|| EqualOwnerFn(searched_shared, current_weak)) {
						return true;
					}
					return false;
				}
			); // lambda
			if (*it_current.expired()) { // cleanup
				it_current = generic::EraseIt(container, it_current);			// O(1)
			} else {
				found_equal = true;
			}
		} // !do
		while (!found_equal && it_current != end);

		return it_current;
	}

	template<typename ContainerT, typename ValueT, typename ExecPolicyT>
	inline bool HasValue(const ContainerT& container,
						const std::weak_ptr<ValueT> searched_ptr,
						ExecPolicyT policy = std::execution::seq) {
		return FindEqualOwner(container, searched_ptr, policy) != container.end();
	}

	template<typename ContainerT, typename ValueT, typename ExecPolicyT>
	inline bool HasValueNClean(const ContainerT& container,
								const std::weak_ptr<ValueT> searched_ptr,
								ExecPolicyT policy = std::execution::seq) {
		return FindEqualOwnerNClean(container, searched_ptr, policy) != container.end();
	}

//=======================Erase=====================================================================================

	/**
	 * Erase first weak_ptr in container, that is alive and has same stored pointer. Container stores weak_ptr.
	 *
	 * Complexity: set = O(log n). Other containers = O(n).
	 * Mutex: write.
	 *
	 * @param container
	 * @param searched_ptr
	 * @param policy
	 * @return				count of expired weak_ptr
	 */
	template<typename ContainerT, typename ValueT, typename ExecPolicyT>
	inline auto EraseEqualOwner(ContainerT& container,
								const std::weak_ptr<ValueT> searched_ptr,
								ExecPolicyT policy = std::execution::seq)
			-> decltype(container.end())
	{
		auto it_equal = FindEqualOwner(container, searched_ptr, policy);
		return generic::EraseIt(container, it_equal);
	}

	/**
	 * Erase first weak_ptr in container, that is alive and has same stored pointer. Container stores weak_ptr.
	 * With auto clean of expired weak_ptrs.
	 *
	 * Complexity: O(n)
	 * Mutex: write
	 *
	 * @param container
	 * @param searched_ptr
	 * @param policy
	 */
	template<typename ContainerT, typename ValueT, typename ExecPolicyT>
	inline auto EraseEqualOwnerNClean(ContainerT& container,
										const std::weak_ptr<ValueT> searched_ptr,
										ExecPolicyT policy = std::execution::seq)
			-> decltype(container.end())
	{
		auto it_equal = FindEqualOwnerNClean(container, searched_ptr, policy);
		return generic::EraseIt(container, it_equal);
	}


	/**
	 * Erase all expired weak_ptr from container
	 *
	 * Complexity: O(n)
	 * Mutex: write
	 */
	template<typename ContainerT, typename ExecPolicyT>
	inline void EraseAllExpired(ContainerT& container, ExecPolicyT policy = std::execution::seq) {
		// Generic Function. Maybe used not only in observer. So no mutex lock inside.
		if (container.empty()) { return; }

		auto expired_fn = [](const auto& value_ptr) { return value_ptr.expired(); };
		generic::RemoveIf(container, expired_fn, policy);		// O(n)
	};

	/**
	 * Erase custom n number of expired weak_ptr from container
	 *
	 * Complexity: O(n)
	 * Mutex: write
	 */
	template<typename ContainerT, typename ExecPolicyT>
	inline void EraseNExpired(ContainerT& container,
								const size_t expired_count,
								ExecPolicyT policy = std::execution::seq) {
		if (container.empty() || expired_count == 0) { return; } // Precondition
		//static_assert(std::is_same_v<typename ContainerT::value_type, std::weak_ptr<ValueT>>,
						//"The type mismatch between container elements and weak_ptr");

		size_t find_count{};
		auto expired_fn = [&expired_count, &find_count](const typename ContainerT::value_type& value_ptr) {
			if (find_count < expired_count && value_ptr.expired()) { // after N expired found, value_ptr.expired() is not called
				++find_count;
				return true;
			}
			return false;
		}; // !lambda
		generic::RemoveIf(container, expired_fn, policy);		// O(n)
	};
	// TODO: may work not on expired_count, but on it_last_expired - iterator to last expired.

} // !namespace util




namespace deprecated {
    using namespace util;

	/**
	 * Erase first weak_ptr in container, that is alive and has same stored pointer. Container stores weak_ptr.
	 *
	 * Complexity: set = O(log n). Other containers = O(n).
	 * Mutex: write.
	 *
	 * @param container
	 * @param searched_ptr
	 * @param policy
	 * @return	count of expired weak_ptr
	 */
	template<typename ContainerT, typename ValueT, typename ExecPolicyT>
	[[deprecated]]
	inline size_t EraseEqualWeakPtr_Old(ContainerT& container,
										const std::weak_ptr<ValueT> searched_ptr,
										ExecPolicyT policy = std::execution::seq) {
		size_t expired_count{};
		if (container.empty()) { return expired_count; }
		static_assert(std::is_same_v<typename ContainerT::value_type, std::weak_ptr<ValueT>>,
					"The type mismatch between container elements and weak_ptr");

		auto it_equal{ container.end() };
		if constexpr (std::is_same_v<ContainerT, std::forward_list<typename ContainerT::value_type>>) { // forward_list
			std::shared_ptr<ValueT> searched_shared{};
			auto equal_owner = [&searched_shared, &expired_count](const auto& current_ptr) {
				if (current_ptr.expired()) { ++expired_count; }
				return EqualOwnerFn(searched_shared, current_ptr);								// O(1)
			}; // !lambda end

			{
				searched_shared = searched_ptr.lock();	// weak_ptr
				if (searched_shared) { container.remove_if(equal_owner); }								// O(n)
			}
		} else { // All other types of containers
			// std::move(searched_ptr) is no necessary, cause weak_ptr hold 2 pointers.
			// Copy or Move operations are equal in performance.

			//auto result_fn{ FindEqualOwner(container, searched_ptr, policy) }; // set = O(log n) ; others = O(n)
			std::tie(it_equal, expired_count) = FindEqualOwner(container, searched_ptr, policy); // set = O(log n) ; others = O(n)
			//it_equal = result_fn.first;
			//expired_count = result_fn.second;
			if (it_equal != container.end()) { container.erase(it_equal); }
		}
		return expired_count;
	}
	// TODO: refactor -> 1) First GenericFind(). 2) GenericErase(). Deletion for set is simple

} // !namespace deprecated

#endif // !WEAK_PTR_HPP
