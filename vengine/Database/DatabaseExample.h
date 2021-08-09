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
	auto db = dbParent->CreateOrGetDatabase(0, {});
	// Get Root Json Object
	auto rootObj = db->GetRootObject();
	// Create a json array
	auto subArr = rootObj->AddOrGetArray("array");
	subArr->Add(5);
	subArr->Add(8.3);
	subArr->Add("string1"_sv);
	// Create a json dictionary
	auto subObj = rootObj->AddOrGetDict("dict");
	subObj->Set("number"_sv, 53);
	subObj->Set("number1"_sv, 12.5);
	// Set RootObj
	//Full Serialize Data
	auto vec = db->Serialize();
	std::cout << "Serialize Size: " << vec.size() << " bytes\n";
	//Incremental Serialize Data
	subObj->Set("number"_sv, 26);
	auto updateV = db->IncreSerialize();
	std::cout << "Update Size: " << updateV.size() << " bytes\n";

	/////////////// Clone Database by serialize binary
	auto cloneDB = dbParent->CreateOrGetDatabase(1, {});
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
	cloneDB->Read(updateV, &evtTrigger);
	auto cloneRoot = cloneDB->GetRootObject();
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
				[](auto&& f) {
					std::cout << f << '\n';
				},
				[](auto&& f) {
					std::cout << f << '\n';
				},[](auto&& f) {},
				[](auto&& f) {});
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
				[](auto&& f) {
					std::cout << f << '\n';
				},
				[](auto&& f) {
					std::cout << f << '\n';
				},
				[](auto&& f) {},
				[](auto&& f) {});
		}
		(*cloneDict)->Dispose();
	}
	/*
	cloneRoot->Set("array1", subArr);
	auto cloneArr1 = cloneRoot->GetArray("array1"_sv);
	IJsonSubDatabase* bb = db;
	if (cloneArr1) {
		auto ite = (*cloneArr1)->GetIterator();
		//Iterate and print all elements
		LINQ_LOOP(i, *ite) {
			auto func = [](auto&& f) {
				std::cout << f << '\n';
			};
			i->visit(
				func,
				func,
				func,
				[](auto&& f) {
					std::cout << f.instanceID << '\n';
				},
				[](auto&& f) {
					std::cout << f.instanceID << '\n';
				});
		}
		(*cloneArr)->Dispose();
	}
	*/
}