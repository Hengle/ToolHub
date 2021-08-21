#pragma vengine_package vengine_database

#include <Database/SimpleJsonLoader.h>
#include <Database/SimpleBinaryJson.h>
#include <Database/SimpleJsonValue.h>
namespace toolhub::db {
bool SimpleJsonLoader::Check(IJsonDatabase* parent, SimpleJsonVariant const& var) {
	bool res = false;
	auto setTrue = [&](auto&&) {
		res = true;
	};
	auto setNullCheck = [&](auto&& v) {
		res = (v.get() != nullptr);
	};
	var.value.visit(
		setTrue,
		setTrue,
		setTrue,
		setNullCheck,
		setNullCheck,
		setTrue);
	return res;
}
SimpleJsonVariant SimpleJsonLoader::DeSerialize(std::span<uint8_t>& arr, SimpleBinaryJson* db) {
	ValueType type = PopValue<ValueType>(arr);
	switch (type) {
		case ValueType::Int: {
			int64 v = PopValue<int64>(arr);
			return SimpleJsonVariant(v);
		}
		case ValueType::Float: {
			double d = PopValue<double>(arr);
			return SimpleJsonVariant(d);
		}

		case ValueType::String: {
			return SimpleJsonVariant(PopValue<vstd::string>(arr));
		}
		case ValueType::ValueDict: {
			auto ptr = db->dictValuePool.New(db);
			ptr->LoadFromSer(arr);
			return SimpleJsonVariant(ptr);
		}
		case ValueType::ValueArray: {
			auto ptr = db->arrValuePool.New(db);
			ptr->LoadFromSer(arr);
			return SimpleJsonVariant(ptr);
		}
		case ValueType::GUID: {
			return SimpleJsonVariant(PopValue<vstd::Guid>(arr));
		}
		default:
			return SimpleJsonVariant();
	}
}
void SimpleJsonLoader::Serialize(SimpleJsonVariant const& v, vstd::vector<uint8_t>& data) {
	size_t dataOffset = data.size();
	data.push_back(v.value.GetType());
	auto func = [&]<typename TT>(TT&& f) {
		PushDataToVector(f, data);
	};

	auto serValue = [&](auto&& d) {
		d->M_GetSerData(data);
	};
	v.value.visit(
		func,//int64
		func,//double
		func,//string
		[&](auto&& d) {
			serValue(static_cast<SimpleJsonValueDict*>(d.get()));
		},
		[&](auto&& d) {
			serValue(static_cast<SimpleJsonValueArray*>(d.get()));
		},
		func);
}
static void SetVariantDict(
	WriteJsonVariant& value,
	IJsonDict* d) {
	if (d == nullptr) {
		value.update(3, [](void* ptr) {
			new (ptr) IJsonDict*(nullptr);
		});
	}
	auto obj = static_cast<SimpleJsonValueDict*>(d);
	auto db = obj->GetDB();
	if (obj->IsEmpty())
		value = db->dictValuePool.New(db);
	else
		value = db->dictValuePool.New(db, d);
}
static void SetVariantArray(
	WriteJsonVariant& value,
	IJsonArray* d) {
	if (d == nullptr) {
		value.update(4, [](void* ptr) {
			new (ptr) IJsonArray*(nullptr);
		});
	}
	auto obj = static_cast<SimpleJsonValueArray*>(d);
	auto db = obj->GetDB();
	if (obj->IsEmpty())
		value = db->arrValuePool.New(db);
	else
		value = db->arrValuePool.New(db, d);
}
SimpleJsonVariant::SimpleJsonVariant(ReadJsonVariant const& v) {
	auto func = [&](auto&& v) {
		value = v;
	};
	v.visit(
		func,
		func,
		func,
		[&](auto&& d) {
			SetVariantDict(value, d);
		},
		[&](auto&& d) {
			SetVariantArray(value, d);
		},
		func);
}
SimpleJsonVariant::SimpleJsonVariant(SimpleJsonVariant const& v) {
	auto func = [&](auto&& v) {
		value = v;
	};
	v.value.visit(
		func,
		func,
		func,
		[&](auto&& d) {
			SetVariantDict(value, d.get());
		},
		[&](auto&& d) {
			SetVariantArray(value, d.get());
		},
		func);
}
ReadJsonVariant SimpleJsonVariant::GetVariant() const {
	auto func = [&](auto&& v) -> ReadJsonVariant {
		return v;
	};
	return value.visit(
		func,//int64
		func,//double
		func,//string
		[&](vstd::unique_ptr<IJsonDict> const& v) -> ReadJsonVariant {
			return static_cast<SimpleJsonValueDict*>(v.get());
		},
		[&](vstd::unique_ptr<IJsonArray> const& v) -> ReadJsonVariant {
			return static_cast<SimpleJsonValueArray*>(v.get());
		},
		func);
}

}// namespace toolhub::db