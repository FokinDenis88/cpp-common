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


namespace common {
	/* Work with weak_ptr in containers. Process weak_ptr Container. Erase. */

	using ExpiredFnT = decltype([](const auto& value_weak_ptr) { return value_weak_ptr.expired(); });


	/** Function to compare objects via shared_ptr inside weak_ptr */
	template<typename ValueT>
	struct IsWeakPtrOwnerEqual {
		/**
		 * Compares if two weak_ptr are alive and have the same owner, stored object.
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
		inline bool operator()(const std::shared_ptr<ValueT>& searched_shared,
								const std::weak_ptr<ValueT>& current_ptr) const noexcept {
			auto current_shared = current_ptr.lock();

			// a==b, if (not a<b) and (not b<a)
			return searched_shared && current_shared && !searched_shared.owner_before(current_shared) &&
					!current_shared.owner_before(searched_shared);
		}
	}; // !struct IsWeakPtrOwnerEqual



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

		auto expired = [](const auto& value_ptr) { return value_ptr.expired(); };
		GenericRemoveIf(container, expired, policy);		// O(n)
	};

	/**
	 * Erase custom n number of expired weak_ptr from container
	 *
	 * Complexity: O(n)
	 * Mutex: write
	 */
	template<typename ContainerT, typename ExecPolicyT>
	inline void EraseNExpiredWeakPtr(ContainerT& container,
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
		GenericRemoveIf(container, expired_fn, policy);		// O(n)
	};
	// TODO: may work not on expired_count, but on it_last_expired - iterator to last expired.


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
	inline size_t EraseEqualWeakPtr(ContainerT& container,
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
				return IsWeakPtrOwnerEqual(searched_shared, current_ptr);								// O(1)
			}; // !lambda end
			{
				searched_shared = searched_ptr.lock();	// weak_ptr
				if (searched_shared) { container.remove_if(equal_owner); }								// O(n)
			}
		} else { // All other types of containers
			// std::move(searched_ptr) is no necessary, cause weak_ptr hold 2 pointers.
			// Copy or Move operations are equal in performance.

			//auto result_fn{ FindEqualWeakPtr(container, searched_ptr, policy) }; // set = O(log n) ; others = O(n)
			std::tie(it_equal, expired_count) = FindEqualWeakPtr(container, searched_ptr, policy); // set = O(log n) ; others = O(n)
			//it_equal = result_fn.first;
			//expired_count = result_fn.second;
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
	 * @param container
	 * @param searched_ptr
	 * @param policy
	 */
	template<typename ContainerT, typename ValueT, typename ExecPolicyT>
	inline void EraseEqualWeakPtrNClean(ContainerT& container,
										const std::weak_ptr<ValueT> searched_ptr,
										ExecPolicyT policy = std::execution::seq) {
		size_t expired_count{ EraseEqualWeakPtr(container, searched_ptr, policy) };
		// Cleanup expired weak_ptr
		EraseNExpiredWeakPtr(container, expired_count, policy);
	}

//===========================Find weak_ptr====================================================

	/**
	 * Find first weak_ptr, that is alive and has same stored pointer.
	 *
	 * Complexity: set = O(log n). Other containers = O(n).
	 * Mutex: read
	 *
	 * @return	First = iterator to equal weak_ptr or to end.
	 *			Second = size_t count of expired weak_ptr for cleanup.
	 */
	template<typename ContainerT, typename ValueT, typename ExecPolicyT>
	inline auto FindEqualWeakPtr(const ContainerT& container,
								const std::weak_ptr<ValueT> searched_ptr,
								ExecPolicyT policy = std::execution::seq)
			-> std::pair<decltype(container.end()), size_t>
	{
		using value_type = typename ContainerT::value_type;
		auto result_fn{ std::make_pair(container.end(), size_t{}) };
		if (container.empty()) { return result_fn; } // precondition
		static_assert(std::is_same_v<value_type, std::weak_ptr<ValueT>>,
						"The type mismatch between container elements and weak_ptr");

		auto& it_equal{ result_fn.first };
		size_t& expired_count{ result_fn.second };

		if constexpr (std::is_same_v< ContainerT, std::set<value_type> >) { // set
			// set must be compared with owner_less in declaration of set variable
			it_equal = container.find(searched_ptr);												// O(log n)
		} else { // All other types of containers
			std::shared_ptr<ValueT> searched_shared{};
			auto equal_owner = [&searched_shared, &expired_count](const auto& current_ptr) {
				if (current_ptr.expired()) { ++expired_count; }
				return IsWeakPtrOwnerEqual(searched_shared, current_ptr);								// O(1)
			}; // !lambda
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
	template<typename ContainerT, typename ValueT, typename ExecPolicyT>
	inline auto FindEqualWeakPtrNClean(ContainerT& container,
										const std::weak_ptr<ValueT> searched_ptr,
										ExecPolicyT policy = std::execution::seq)
			-> decltype(container.end())
	{
		auto result_fn{ FindEqualWeakPtr(container, searched_ptr, policy) };
		// Cleanup expired weak_ptr
		EraseNExpiredWeakPtr(container, result_fn.second, policy);
		return result_fn.first;
	}

} // !namespace common

#endif // !WEAK_PTR_HPP
