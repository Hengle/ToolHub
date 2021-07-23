#include <Common/Common.h>
#include <Database/IJsonDatabase.h>
#include <Database/JsonObject.h>
#include <Common/unique_ptr.h>

void test() {
	using namespace toolhub::db;
	auto db = CreateSimpleJsonDB();
	auto rootObj = db->GetRootObject();
	auto subArr = db->CreateJsonArray();
	subArr->Add(5);
	subArr->Add(8.3);
	subArr->Add("fuck"_sv);

	auto subObj = db->CreateJsonObject();
	subObj->Set("shit"_sv, 53);
	subObj->Set("shit1"_sv, 12.5);

	rootObj->Set("array"_sv, subArr);
	rootObj->Set("dict"_sv, subObj);
	auto vec = db->Serialize();
	std::cout << vec.size() << '\n';

	auto cloneDB = CreateSimpleJsonDB();
	cloneDB->Read(std::move(vec));
	auto cloneRoot = cloneDB->GetRootObject();
	subArr->Set(1, 10.5);

	auto rIte = cloneRoot->GetIterator();
	LINQ_LOOP(i, *rIte) {
		auto func = [](auto&& f) {
			std::cout << f << '\n';
		};
		i->value.visit(
			func,
			func,
			func,
			func,
			func);
	}
	auto cloneArr = cloneRoot->GetArray("array"_sv);
	if (cloneArr) {
		auto subSer = subArr->GetSerData();
		std::cout << "yes array!\n";
		auto ite = (*cloneArr)->GetIterator();
		LINQ_LOOP(i, *ite) {
			//std::cout << i->GetType() << '\n';
			auto func = [](auto&& f) {
				std::cout << f << '\n';
			};
			i->visit(
				func,
				func,
				func,
				func,
				func);
		}
	}
	auto cloneDict = cloneRoot->GetDict("dict"_sv);
	if (cloneDict) {
		std::cout << "yes dict!\n";
		auto ite = (*cloneDict)->GetIterator();
		LINQ_LOOP(i, *ite) {
			auto func = [](auto&& f) {
				std::cout << f << '\n';
			};
			i->value.visit(
				func,
				func,
				func,
				func,
				func);
		}
	}
}

class TestClass {
public:
	TestClass() {
	}
	TestClass(TestClass& t)
		: TestClass((TestClass const&)t) {}
	TestClass(TestClass const&& t)
		: TestClass((TestClass const&)t) {}
	TestClass(TestClass const&) {
		std::cout << "copy\n";
	}
	TestClass(TestClass&&) {
		std::cout << "move\n";
	}
	template<typename T>
	TestClass(T&& t) {
		std::cout << "template\n";
	}
	~TestClass() {
		std::cout << "dispose\n";
	}
};

int main() {
	vengine_init_malloc();
	//vstd::string sb = "fuck";
	//std::cout << sb << '\n';
	//std::cout << sb.size();
	test();

	return 0;
}