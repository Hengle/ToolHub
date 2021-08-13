#pragma once

/*
A json structure database.
A simple example of interface
*/

#include <Database/DatabaseInclude.h>
#include <Database/IJsonDatabase.h>
#include <Database/IJsonObject.h>
#include <Common/VObject.h>
void jsonTest(
	toolhub::db::Database const* database) {
	using namespace toolhub::db;
	// Generate a database
	auto dbParent = MakeObjectPtr(database->CreateDatabase());
	auto db = dbParent->CreateDatabase({});
	db->NameGUID("Root", vstd::Guid(true));
	// Get Root Json Object
	auto rootObj = db->CreateJsonObject(db->GetNamedGUID("Root"));
	db->CreateJsonArray();
	db->CreateJsonArray();
	db->CreateJsonArray();
	db->CreateJsonArray();
	auto ptr = db->CreateJsonArray();
	ThreadPool tp(std::thread::hardware_concurrency());
	// Create a json array
	auto subArr = rootObj->AddOrGetArray("array");
	subArr->Add(5);
	subArr->Add(8.3);
	subArr->Add("string1"_sv);
	// Create a json dictionary
	auto subObj = rootObj->AddOrGetDict("dict");
	subObj->Set("number"_sv, 53);
	subObj->Set("number1"_sv, 12.5);
	subObj->Set("wrong", rootObj);
	// Set RootObj
	//Full Serialize Data
	subArr->Add(vstd::Guid(true));
	subObj->Set("number"_sv, 26);
	subArr->Set(1, 141);
	subArr->Add(vstd::Guid(true));
	db->CollectGarbage(&tp, vstd::vector<vstd::Guid>{db->GetNamedGUID("Root")}).Complete();
	auto vec = db->Serialize();
	std::cout << "Serialize Size: " << vec.size() << " bytes\n";
	//Incremental Serialize Data

	/////////////// Clone Database by serialize binary
	auto cloneDB = dbParent->CreateDatabase({});
	struct EventTrigger : public IDatabaseEvtVisitor {
		void AddDict(IJsonRefDict* newDict) override {
			std::cout << "Add Dict!" << '\n';
		}
		void RemoveDict(IJsonRefDict* removedDict) override {
			std::cout << "Remove Dict!" << '\n';
		}
		void AddArray(IJsonRefArray* newDict) override {
			std::cout << "Add Array!" << '\n';
		}
		void RemoveArray(IJsonRefArray* newDict) override {
			std::cout << "Remove Array!" << '\n';
		}
	};
	EventTrigger evtTrigger;
	cloneDB->Read(vec, &evtTrigger);
	auto cloneRoot = *cloneDB->GetNode(cloneDB->GetNamedGUID("Root")).try_get<IJsonRefDict*>();
	auto rIte = cloneRoot->GetIterator();
	// cross database link
	// cloneArr == subArr
	auto cloneArrVariant = cloneRoot->Get("array"_sv);
	auto cloneArr = cloneArrVariant.try_get<IJsonValueArray*>();
	if (cloneArr) {
		auto ite = (*cloneArr)->GetIterator();
		//Iterate and print all elements
		LINQ_LOOP(i, *ite) {
			auto func = [](auto&& f) {
				std::cout << f << '\n';
			};
			i->visit(
				func,
				func,
				func,
				[](auto&& f) {},
				[](auto&& f) {},
				[](auto&& f) { std::cout << f.ToString(true) << '\n'; });
		}
		(*cloneArr)->Dispose();
	}

	auto cloneDict = cloneRoot->Get("dict"_sv).try_get<IJsonValueDict*>();
	if (cloneDict) {
		auto ite = (*cloneDict)->GetIterator();
		//Iterate and print all elements
		LINQ_LOOP(i, *ite) {
			auto func = [](auto&& f) {
				std::cout << f << '\n';
			};
			std::cout << "key: " << i->key << " Value: ";
			i->value.visit(
				func,
				func,
				func,
				[](auto&& f) {},
				[](auto&& f) {},
				[](auto&& f) { std::cout << f.ToString(true) << '\n'; });
		}
		(*cloneDict)->Dispose();
	}
}