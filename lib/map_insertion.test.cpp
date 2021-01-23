//@	{
//@	 "targets":[{"name":"map_insertion.test","type":"application", "autorun":1}]
//@	}

#include "./map_insertion.hpp"

#include <cassert>
#include <map>
#include <string>

namespace
{
	using Map = std::map<std::string, int>;
	using Insertion = Wad64::MapInsertion<Map>;
}

namespace Testcases
{
	void wad64MapInsertionDefaultConstruct()
	{
		Insertion obj;
		assert(!obj.valid());
		assert(!obj.itemInserted());
	}

	void wad64MapInsertionConstructWithoutContainer()
	{
		Map foo;
 		auto i = foo.insert(std::pair{"Bar", 123});
		{
			Insertion ins{std::move(i.first)};
			assert(ins.valid());
			assert(!ins.itemInserted());
		}
		assert(foo.contains("Bar"));
	}

	void wad64MapInsertionConstructWithContainer()
	{
		Map foo;
 		auto i = foo.insert(std::pair{"Bar", 123});
		{
			Insertion ins{&foo, std::move(i.first)};
			assert(ins.valid());
			assert(ins.itemInserted());
		}
		assert(!foo.contains("Bar"));
	}

	void wad64MapInsertionConstructWithContainerAndCommit()
	{
		Map foo;
 		auto i = foo.insert(std::pair{"Bar", 0});
		{
			Insertion ins{&foo, std::move(i.first)};
			assert(ins.valid());
			assert(ins.itemInserted());
			ins.commit(123, [](int val) {
				assert(val == 123);
			});
			assert(!ins.itemInserted());
		}
		assert(foo.contains("Bar"));
		assert(foo.find("Bar")->second == 123);
	}

	void wad64MapInsertionConstructWithContainerAndCommitCallbackThrows()
	{
		Map foo;
 		auto i = foo.insert(std::pair{"Bar", 0});
		{
			Insertion ins{&foo, std::move(i.first)};
			assert(ins.valid());
			assert(ins.itemInserted());
			try
			{
				ins.commit(123, [](int) { throw "Blah"; });
			}
			catch(...)
			{
			}
			assert(ins.valid());
			assert(ins.itemInserted());
		}
		assert(!foo.contains("Bar"));
	}

	void wad64MapInsertionConstructWithContainerMove()
	{
		Map foo;
 		auto i = foo.insert(std::pair{"Bar", 123});

		Insertion ins{&foo, std::move(i.first)};
		assert(ins.valid());
		assert(ins.itemInserted());

		auto other = std::move(ins);
		assert(!ins.itemInserted());
		assert(other.itemInserted());

		ins = std::move(other);
	}
}

int main()
{
	Testcases::wad64MapInsertionDefaultConstruct();
	Testcases::wad64MapInsertionConstructWithoutContainer();
	Testcases::wad64MapInsertionConstructWithContainer();
	Testcases::wad64MapInsertionConstructWithContainerAndCommit();
	Testcases::wad64MapInsertionConstructWithContainerAndCommitCallbackThrows();
	Testcases::wad64MapInsertionConstructWithContainerMove();
	return 0;
}