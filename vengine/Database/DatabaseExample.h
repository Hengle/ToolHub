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
	auto db = database->CreateDatabase();
	size_t binaryKey = 234641372;
	{
		auto rootObj = db->GetRootNode();
		// Create a json array
		
		// Create a json dictionary
		{
			auto subObj = db->CreateDict();

			subObj->Set("number"_sv, 53);
			subObj->Set("number1"_sv, 14);
			// Set RootObj
			//Full Serialize Data
			subObj->Set("number2"_sv, 26);
			subObj->Set(binaryKey, 656);

			auto middleArr = db->CreateArray();

			auto subArr = db->CreateArray();
			subArr->Add(5);
			subArr->Add(8.3);
			subArr->Add("string1"_sv);
			subArr->Add(vstd::Guid(true));
			subArr->Add(vstd::Guid(true));
			middleArr->Add(std::move(subArr));
			middleArr->Add(std::move(subObj));

			rootObj->Set("dict", std::move(middleArr));
		}
	}
	std::cout << db->Print() << '\n';
	return;
	auto cloneDB = database->CreateDatabase();
	cloneDB->Read(db->Serialize());
	{
		auto rootObj = cloneDB->GetRootNode();
		auto ite = rootObj->GetIterator();
		LINQ_LOOP(i, *ite) {
			auto strv = i->key.try_get<vstd::string_view>();
			if (strv) {
				std::cout << *strv << '\n';
			}
		}
		auto printVariant = [](ReadJsonVariant const& var) {
			auto func = [](auto&& v) { std::cout << v << '\n'; };
			var.visit(
				func,
				func,
				func,
				[](auto&&) {},
				[](auto&&) {},
				[](vstd::Guid const& v) {
					std::cout << v.ToString() << '\n';
				});
		};
		auto printKey = [](Key const& k) {
			auto func = [](auto&& v) { std::cout << v << ' '; };
			k.visit(
				func,
				func,
				[](vstd::Guid const& v) {
					std::cout << v.ToString() << ' ';
				});
		};
		auto subArr = (*rootObj->Get("dict").try_get<IJsonArray*>())->Get(0).try_get<IJsonArray*>();
		if (subArr) {
			auto arrIte = (*subArr)->GetIterator();
			LINQ_LOOP(i, *arrIte) {
				printVariant(*i);
			}
		}
		auto subDict = (*rootObj->Get("dict").try_get<IJsonArray*>())->Get(1).try_get<IJsonDict*>();
		std::cout << "searched binary: ";

		printVariant((*subDict)->Get(binaryKey));
		std::cout << '\n';
		if (subDict) {
			auto dictIte = (*subDict)->GetIterator();
			LINQ_LOOP(i, *dictIte) {
				printKey(i->key);
				printVariant(i->value);
			}
		}
	}
}