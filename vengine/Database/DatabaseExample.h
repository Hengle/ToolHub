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
	auto db = MakeObjectPtr(database->CreateSimpleJsonDB());
	// Get Root Json Object
	auto rootObj = db->GetRootObject();
	// Create a json array
	auto subArr = db->CreateJsonArray();
	subArr->Add(5);
	subArr->Add(8.3);
	subArr->Add("string1"_sv);
	// Create a json dictionary
	auto subObj = db->CreateJsonObject();
	subObj->Set("number"_sv, 53);
	subObj->Set("number1"_sv, 12.5);

	// Set RootObj
	rootObj->Set("array"_sv, subArr);
	rootObj->Set("dict"_sv, subObj);
	//Full Serialize Data
	auto vec = db->Serialize();
	std::cout << "Serialize Size: " << vec.size() << " bytes\n";
	//Incremental Serialize Data
	auto updateV = db->IncreSerialize();
	std::cout << "Update Size: " << updateV.size() << " bytes\n";

	/////////////// Clone Database by serialize binary
	auto cloneDB = MakeObjectPtr(database->CreateSimpleJsonDB());
	struct EventTrigger : public IDatabaseEvtVisitor {
		void AddDict(IJsonDict* newDict) override {
			std::cout << "Add Dict!" << '\n';
		}
		void RemoveDict(IJsonDict* removedDict) override {
			std::cout << "Remove Dict!" << '\n';
		}
		void AddArray(IJsonArray* newDict) override {
			std::cout << "Add Array!" << '\n';
		}
		void RemoveArray(IJsonArray* newDict) override {
			std::cout << "Remove Array!" << '\n';
		}
	};
	EventTrigger evtTrigger;
	cloneDB->Read(vec, &evtTrigger);
	std::cout << "Incremental!\n";
	cloneDB->Read(updateV, &evtTrigger);
	auto cloneRoot = cloneDB->GetRootObject();
	auto rIte = cloneRoot->GetIterator();
	auto cloneArr = cloneRoot->GetArray("array"_sv);
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
				func,
				func);
		}
		(*cloneArr)->Dispose();
	}
	auto cloneDict = cloneRoot->GetDict("dict"_sv);
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
				func,
				func);
		}
	}
}