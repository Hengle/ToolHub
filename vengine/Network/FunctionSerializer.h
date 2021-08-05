#pragma once
#include <Common/Common.h>
#include <Common/Runnable.h>
namespace vstd {
template<typename T>
struct SerDe {
	static_assert(std::is_trivial_v<T>, "only trivial type can be serialized!");
	static T Get(std::span<uint8_t>& data) {
		T const* ptr = reinterpret_cast<T const*>(data.data());
		data = std::span<uint8_t>(data.data() + sizeof(T), data.size() - sizeof(T));
		return *ptr;
	}
	static void Set(T const& data, vstd::vector<uint8_t>& vec) {
		vec.push_back_all(reinterpret_cast<uint8_t const*>(&data), sizeof(T));
	}
};
template<>
struct SerDe<vstd::string> {
	static vstd::string Get(std::span<uint8_t>& sp) {
		auto strLen = SerDe<uint>::Get(sp);
		auto ptr = sp.data();
		sp = std::span<uint8_t>(ptr + strLen, sp.size() - strLen);
		return vstd::string(vstd::string_view(
			reinterpret_cast<char const*>(ptr),
			strLen));
	}
	static void Set(vstd::string const& data, vstd::vector<uint8_t>& arr) {
		SerDe<uint>::Set(data.size(), arr);
		arr.push_back_all(reinterpret_cast<uint8_t const*>(data.data()), data.size());
	}
};
template<>
struct SerDe<vstd::string_view> {
	static vstd::string_view Get(std::span<uint8_t>& sp) {
		auto strLen = SerDe<uint>::Get(sp);
		auto ptr = sp.data();
		sp = std::span<uint8_t>(ptr + strLen, sp.size() - strLen);
		return vstd::string_view(
			reinterpret_cast<char const*>(ptr),
			strLen);
	}
	static void Set(vstd::string_view const& data, vstd::vector<uint8_t>& arr) {
		SerDe<uint>::Set(data.size(), arr);
		arr.push_back_all(reinterpret_cast<uint8_t const*>(data.begin()), data.size());
	}
};

template<typename T, VEngine_AllocType alloc, bool tri>
struct SerDe<vstd::vector<T, alloc, tri>> {
	using Value = vstd::vector<T, alloc, tri>;
	static Value Get(std::span<uint8_t>& sp) {
		Value sz;
		auto s = SerDe<uint>::Get(sp);
		sz.push_back_func(
			[&]() {
				return SerDe<T>::Get(sp);
			},
			s);
		return sz;
	}
	static void Set(Value const& data, vstd::vector<uint8_t>& arr) {
		SerDe<uint>::Set(data.size(), arr);
		for (auto&& i : data) {
			SerDe<T>::Set(i, arr);
		}
	}
};

template<typename K, typename V, typename Hash, typename Equal, VEngine_AllocType alloc>
struct SerDe<HashMap<K, V, Hash, Equal, alloc>> {
	using Value = HashMap<K, V, Hash, Equal, alloc>;
	static Value Get(std::span<uint8_t>& sp) {
		Value sz;
		auto capa = SerDe<uint>::Get(sp);
		sz.reserve(capa);
		for (auto&& i : vstd::range(capa)) {
			auto key = SerDe<K>::Get(sp);
			auto value = SerDe<V>::Get(sp);
			sz.Emplace(
				std::move(key),
				std::move(value));
		}
		return sz;
	}
	static void Set(Value const& data, vstd::vector<uint8_t>& arr) {
		SerDe<uint>::Set(data.size(), arr);
		for (auto&& i : data) {
			SerDe<K>::Set(i.first, arr);
			SerDe<V>::Set(i.second, arr);
		}
	}
};
template<typename... Args>
struct SerDe<vstd::variant<Args...>> {
	using Value = vstd::variant<Args...>;
	template<typename T>
	static void ExecuteGet(Value* placePtr, std::span<uint8_t>& sp) {
		new (placePtr) Value(SerDe<T>::Get(sp));
	}
	template<typename T>
	static void ExecuteSet(void const* placePtr, vstd::vector<uint8_t>& sp) {
		SerDe<T>::Set(*reinterpret_cast<T const*>(placePtr), sp);
	}
	static Value Get(std::span<uint8_t>& sp) {
		auto type = SerDe<uint8_t>::Get(sp);
		funcPtr_t<void(Value*, std::span<uint8_t>&)> ptrs[sizeof...(Args)] = {
			ExecuteGet<Args>...};
		StackObject<Value> value;
		ptrs[type](value, sp);
		return *value;
	}
	static void Set(Value const& data, vstd::vector<uint8_t>& arr) {
		SerDe<uint8_t>::Set(data.GetType(), arr);
		funcPtr_t<void(void const*, vstd::vector<uint8_t>&)> ptrs[sizeof...(Args)] = {
			ExecuteSet<Args>...};
		ptrs[data.GetType()](&data, arr);
	}
};

template<typename A, typename B>
struct SerDe<std::pair<A, B>> {
	using Value = std::pair<A, B>;
	static Value Get(std::span<uint8_t>& sp) {
		return Value{SerDe<A>::Get(sp), SerDe<B>::Get(sp)};
	}
	static void Set(Value const& data, vstd::vector<uint8_t>& arr) {
		SerDe<A>::Set(data.first, arr);
		SerDe<B>::Set(data.second, arr);
	}
};

template<>
struct SerDe<std::span<uint8_t>> {
	using Value = std::span<uint8_t>;
	static Value Get(Value& sp) {
		auto sz = SerDe<uint>::Get(sp);
		Value v(sp.data(), sz);
		sp = Value(sp.data() + sz, sp.size() - sz);
		return v;
	}
	static void Set(Value const& data, vstd::vector<uint8_t>& arr) {
		SerDe<uint>::Set(data.size(), arr);
		arr.push_back_all(data);
	}
};

template<typename Func>
struct SerDeAll_Impl;

template<typename Ret, typename... Args>
struct SerDeAll_Impl<Ret(Args...)> {
	template<typename Func>
	static decltype(auto) Call(
		Func&& func) {
		return [f = std::forward<Func>(func)](std::span<uint8_t> data) {
			std::apply(f, std::tuple<Args...>{SerDe<std::remove_cvref_t<Args>>::Get(data)...});
		};
	}

	static vstd::vector<uint8_t> Ser(
		Args const&... args) {
		vstd::vector<uint8_t> vec;
		auto lst = {(SerDe<std::remove_cvref_t<Args>>::Set(args, vec), ' ')...};
		return vec;
	}
};

template<typename Func>
using SerDeAll = SerDeAll_Impl<FuncType<std::remove_cvref_t<Func>>>;

}// namespace vstd