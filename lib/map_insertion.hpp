//@	{
//@	 "targets":[{"name":"map_insertion.hpp","type":"include"}]
//@	}

#ifndef WAD64_LIB_MAP_INSERTION_HPP
#define WAD64_LIB_MAP_INSERTION_HPP

#include <utility>
#include <cassert>

namespace Wad64
{
	/**
	* \brief Holder for a reference to an inserted item. If associated with the container, the item
	* will be removed on destruction, unless the insertion has been commited with the `commit`
	* member function template.
	*/
	template<class MapType>
	class MapInsertion
	{
	public:
		MapInsertion(): m_valid{false}, m_storage{nullptr} {}

		explicit MapInsertion(MapType::iterator&& i): m_valid{true}, m_storage{nullptr}, m_value{std::move(i)} {}

		explicit MapInsertion(MapType* storage, typename MapType::iterator&& val)
			: m_valid{true}
			, m_storage{storage}
			, m_value{std::move(val)}
		{
		}

		MapInsertion(MapInsertion&& other) noexcept:
		m_valid{other.m_valid},
		m_storage{std::exchange(other.m_storage, nullptr)},
		m_value{std::move(other.m_value)}
		{
		}

		MapInsertion& operator=(MapInsertion&& other) noexcept
		{
			m_valid = other.m_valid;
			m_storage = std::exchange(other.m_storage, nullptr);
			m_value = other.m_value;
			return *this;
		}

		bool itemInserted() const { return m_storage != nullptr; }

		bool valid() const { return m_valid; }

		~MapInsertion()
		{
			if(itemInserted())
			{
				m_storage->erase(m_value);
			}
		}

		template<class Callback>
		void commit(typename MapType::mapped_type&& entry, Callback&& cb)
		{
			assert(valid());
			cb(entry);
			m_value->second = std::move(entry);
			reset();
		}

	private:
		void reset()
		{ m_storage = nullptr; }

		bool m_valid;
		MapType* m_storage;
		typename MapType::iterator m_value;
	};
}

#endif