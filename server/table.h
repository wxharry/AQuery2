// TODO: Replace `cout, printf` with sprintf&fputs and custom buffers

#ifndef _TABLE_H
#define _TABLE_H

#include "types.h"
#include "vector_type.hpp"
#include <iostream>
#include <string>
#include "io.h"
#undef ERROR
template <typename T>
class vector_type;
template <>
class vector_type<void>;

#ifdef _MSC_VER
namespace types {
	enum Type_t;
	template <typename T>
	struct Types;
	template <class T1, class T2>
	struct Coercion;
}
#endif
template<typename _Ty>
class ColView;
template<typename _Ty>
class ColRef : public vector_type<_Ty>
{
public:
	typedef ColRef<_Ty> Decayed_t;
	const char* name;
	types::Type_t ty = types::Type_t::ERROR;
	ColRef() : vector_type<_Ty>(0), name("") {}
	ColRef(const uint32_t& size, const char* name = "") : vector_type<_Ty>(size), name(name) {}
	ColRef(const char* name) : name(name) {}
	ColRef(const uint32_t size, void* data, const char* name = "") : vector_type<_Ty>(size, data), name(name) {}
	void init(const char* name = "") { ty = types::Types<_Ty>::getType();  this->size = this->capacity = 0; this->container = 0; this->name = name; }
	void initfrom(uint32_t sz, void*container, const char* name = "") {
		ty = types::Types<_Ty>::getType();  
		this->size = sz;
		this->capacity = 0;
		this->container = (_Ty*)container; 
		this->name = name; 
	}
	template<template <typename ...> class VT, typename T>
	void initfrom(const VT<T>& v, const char* name = ""){
		ty = types::Types<_Ty>::getType();  
		this->size = v.size;
		this->capacity = 0;
		this->container = (_Ty*)(v.container); 
		this->name = name; 
	}
	template <class T>
	ColRef<_Ty>& operator =(ColRef<T>&& vt) {
		this->container = (_Ty*)vt.container;
		this->size = vt.size;
		this->capacity = vt.capacity;
		vt.capacity = 0; // rvalue's 
		return *this;
	}
	ColRef(const char* name, types::Type_t ty) : name(name), ty(ty) {}
	using vector_type<_Ty>::operator[];
	using vector_type<_Ty>::operator=;
	using vector_type<_Ty>::subvec;
	using vector_type<_Ty>::subvec_memcpy;
	using vector_type<_Ty>::subvec_deep;
	ColView<_Ty> operator [](const vector_type<uint32_t>&idxs) const {
		return ColView<_Ty>(*this, idxs);
	}

	void out(uint32_t n = 4, const char* sep = " ") const {
		const char* more = "";
		if (n < this->size)
			more = " ... ";
		else
			n = this->size;

		std::cout << '(';
		if (n > 0)
		{
			uint32_t i = 0;
			for (; i < n - 1; ++i)
				std::cout << this->operator[](i) << sep;
			std::cout << this->operator[](i);
		}
		std::cout<< more;
		std::cout << ')';
	}
	template<typename T>
	ColRef<T> scast();
	
	ColRef<_Ty>* rename(const char* name) {
		this->name = name;
		return this;
	}

	// defined in table_ext_monetdb.hpp
	void* monetdb_get_col();
	
};

template<typename _Ty>
class ColView {
public: 
	typedef ColRef<_Ty> Decayed_t;
	const vector_type<uint32_t>& idxs;
	const ColRef<_Ty> orig;
	const uint32_t& size;
	ColView(const ColRef<_Ty>& orig, const vector_type<uint32_t>& idxs) : orig(orig), idxs(idxs), size(idxs.size) {}
	ColView(const ColView<_Ty>& orig, const vector_type<uint32_t>& idxs) : orig(orig.orig), idxs(idxs), size(idxs.size) {
		for (uint32_t i = 0; i < size; ++i) 
			idxs[i] = orig.idxs[idxs[i]];
	}
	_Ty& operator [](const uint32_t& i) const {
		return orig[idxs[i]];
	}
	struct Iterator_t {
		const uint32_t* val;
		const ColRef<_Ty>& orig;
		constexpr Iterator_t(const uint32_t* val, const ColRef<_Ty>& orig) noexcept : val(val), orig(orig) {}
		_Ty& operator*() { return orig[*val]; }
		bool operator != (const Iterator_t& rhs) { return rhs.val != val; }
		Iterator_t& operator++ () {
			++val;
			return *this;
		}
		Iterator_t operator++ (int) {
			Iterator_t tmp = *this;
			++val;
			return tmp;
		}
	};
	Iterator_t begin() const {
		return Iterator_t(idxs.begin(), orig);
	}
	Iterator_t end() const {
		return Iterator_t(idxs.end(), orig);
	}
	void out(uint32_t n = 4, const char* sep = " ") const {
		n = n > size ? size : n;
		std::cout<<'(';
		for (uint32_t i = 0; i < n; ++i) 
			std::cout << this->operator[](i)<< sep;
		std::cout << ')';
	}
	operator ColRef<_Ty>() {
		auto ret = ColRef<_Ty>(size);
		for (uint32_t i = 0; i < size; ++i)
			ret[i] = orig[idxs[i]];
		return ret;
	}
	ColView<_Ty> subvec(uint32_t start, uint32_t end) {
		uint32_t len = end - start;
		return ColView<_Ty>(orig, idxs.subvec(start, end));
	}
	ColRef<_Ty> subvec_deep(uint32_t start, uint32_t end) {
		uint32_t len = end - start;
		ColRef<_Ty> subvec(len);
		for (uint32_t i = 0; i < len; ++i)
			subvec[i] = operator[](i);
		return subvec;
	}
	inline ColRef<_Ty> subvec(uint32_t start = 0) { return subvec_deep(start, size); }
};
template <template <class...> class VT, class T>
std::ostream& operator<<(std::ostream& os, const VT<T>& v)
{
	v.out();
	return os;
}
#ifdef __SIZEOF_INT128__
std::ostream& operator<<(std::ostream& os, __int128 & v);
std::ostream& operator<<(std::ostream& os, __uint128_t & v);
#endif
template <class Type>
struct decayed_impl<ColView, Type> { typedef ColRef<Type> type; };

template<typename _Ty>
template<typename T>
inline ColRef<T> ColRef<_Ty>::scast()
{
	this->ty = types::Types<T>::getType();
	return *(ColRef<T> *)this;
}
using uColRef = ColRef<void>;

template<class ...Types> struct TableInfo;
template<class ...Types> struct TableView;

template <long long _Index, bool order = true, class... _Types>
constexpr inline auto& get(const TableInfo<_Types...>& table) noexcept {
	if constexpr (order)
		return *(ColRef<std::tuple_element_t<_Index, std::tuple<_Types...>>> *) & (table.colrefs[_Index]);
	else 
		return *(ColRef<std::tuple_element_t<-1-_Index, std::tuple<_Types...>>> *) & (table.colrefs[-1-_Index]);
}

template <long long _Index, class... _Types>
constexpr inline ColRef<std::tuple_element_t<_Index, std::tuple<_Types...>>>& get(const TableView<_Types...>& table) noexcept {
	return *(ColRef<std::tuple_element_t<_Index, std::tuple<_Types...>>> *) & (table.info.colrefs[_Index]);
}

template <class V>
struct is_vector_impl<ColRef<V>> : std::true_type {};
template <class V>
struct is_vector_impl<ColView<V>> : std::true_type {};
template <class V>
struct is_vector_impl<vector_type<V>> : std::true_type {};

template<class ...Types>
struct TableView;

template<class ...Types>
struct TableInfo {
	const char* name;
	ColRef<void>* colrefs;
	uint32_t n_cols;
	typedef std::tuple<Types...> tuple_type;
	void print(const char* __restrict sep, const char* __restrict end) const;

	template <class ...Types2>
	struct lineage_t {
		TableInfo<Types...>* this_table;
		TableInfo<Types2...>* table;
		vector_type<uint32_t> rid;
		constexpr lineage_t(TableInfo<Types...>*this_table, TableInfo<Types2...> *table) 
			: this_table(this_table), table(table), rid(0) {}
		constexpr lineage_t() : this_table(0), table(0), rid(0) {}

		template <int col>
		inline auto& get(uint32_t idx) {
			return get<col>(*table)[rid[idx]];
		}
		void emplace_back(const uint32_t& v) {
			rid.emplace_back(v);
		}
	};
	
	template<class ...Types2>
	auto bind(TableInfo<Types2...>* table2) {
		return lineage_t(this, table2);
	}
	
	template <size_t i = 0>
	auto& get_col() {
		return *reinterpret_cast<ColRef<std::tuple_element_t <i, tuple_type>>*>(colrefs + i);
	}

	template <size_t j = 0>
	typename std::enable_if<j == sizeof...(Types) - 1, void>::type print_impl(const uint32_t& i, const char* __restrict sep = " ") const;
	template <size_t j = 0>
	typename std::enable_if<j < sizeof...(Types) - 1, void>::type print_impl(const uint32_t& i, const char* __restrict sep = " ") const;
	template <size_t ...Idxs>
	struct GetTypes {
		typedef typename std::tuple<typename std::tuple_element<Idxs, tuple_type>::type ...> type;
	};
	
	template <size_t ...Idxs>
	using getRecordType = typename GetTypes<Idxs...>::type;
	TableInfo(const char* name, uint32_t n_cols);
	TableInfo(const char* name = "", const char **col_names = nullptr);
	template <int prog = 0>
	inline void materialize(const vector_type<uint32_t>& idxs, TableInfo<Types...>* tbl = nullptr) { // inplace materialize
		if constexpr(prog == 0) tbl = (tbl == 0 ? this : tbl);
		if constexpr (prog == sizeof...(Types)) return;
		else {
			auto& col = get<prog>(*this);
			auto new_col = decays<decltype(col)>{idxs.size};
			for(uint32_t i = 0; i < idxs.size; ++i)
				new_col[i] = col[idxs[i]];
			get<prog>(*tbl) = new_col;
			materialize<prog + 1>(idxs, tbl);
		}
	}
	inline TableInfo<Types...>* materialize_copy(const vector_type<uint32_t>& idxs) { 
		auto tbl = new TableInfo<Types...>(this->name, sizeof...(Types));
		materialize<0>(idxs, tbl);
		return tbl;
	}
	template<int ...cols>
	inline vector_type<uint32_t>* order_by(vector_type<uint32_t>* ord = nullptr) {
		if (!ord) {
			ord = new vector_type<uint32_t>(colrefs[0].size);
			for (uint32_t i = 0; i < colrefs[0].size; ++i)
				(*ord)[i] = i;
		}
		std::sort(ord->begin(), ord->end(), [this](const uint32_t& lhs, const uint32_t& rhs) {
			return
				std::forward_as_tuple((cols >= 0 ? get<cols, (cols >= 0)>(*this)[lhs] : -get<cols, (cols >= 0)>(*this)[lhs]) ...)
				<
				std::forward_as_tuple((cols >= 0 ? get<cols, (cols >= 0)>(*this)[rhs] : -get<cols, (cols >= 0)>(*this)[rhs]) ...);
			});
		return ord;
	}
	template <int ...cols>
	auto order_by_view () {
		return TableView<Types...>(order_by<cols...>(), *this);
	}

	// Print 2 -- generate printf string first, supports flattening, supports sprintf/printf/fprintf
	template <int col, int ...rem_cols, class Fn, class ...__Types> 
	inline void print2_impl(Fn func, const uint32_t& i, const __Types& ... args) const {
		using this_type = typename std::tuple_element<col, tuple_type>::type;
		const auto& this_value = get<col>(*this)[i];
		const auto& next = [&](auto &v) {
			if constexpr (sizeof...(rem_cols) == 0)
				func(args..., print_hook(v));
			else
				print2_impl<rem_cols...>(func, i, args ..., print_hook(v));
		};
		if constexpr (is_vector_type<this_type>)
			for (int j = 0; j < this_value.size; ++j)
				next(this_value[j]);
		else
			next(this_value);
	}
	std::string get_header_string(const char* __restrict sep, const char* __restrict end) const{
		std::string header_string = std::string();
		for (int i = 0; i < sizeof...(Types); ++i)
			header_string += std::string(this->colrefs[i].name) + sep + '|' + sep;
		const size_t l_sep = strlen(sep) + 1;
		if (header_string.size() - l_sep >= 0)
			header_string.resize(header_string.size() - l_sep);
		header_string += end + std::string(header_string.size(), '=') + end;
		return header_string;
	}
	template <int ...cols>
	void print2(const char* __restrict sep = ",", const char* __restrict end = "\n",
		const vector_type<uint32_t>* __restrict view = nullptr, FILE* __restrict fp = nullptr) const {
		
		std::string printf_string =
			generate_printf_string<typename std::tuple_element<cols, tuple_type>::type ...>(sep, end);
		std::string header_string = std::string();
		constexpr static int a_cols[] = { cols... };
		for(int i = 0; i < sizeof...(cols); ++i)
			header_string += std::string(this->colrefs[a_cols[i]].name) + sep;
		const size_t l_sep = strlen(sep);
		if(header_string.size() - l_sep >= 0)
			header_string.resize(header_string.size() - l_sep);
		
		const auto& prt_loop = [&fp, &view, &printf_string, *this](const auto& f) {
#ifdef __SIZEOF_INT128__			
			constexpr auto num_hge = count_type<__int128_t, __uint128_t>((tuple_type*)(0));
			char cbuf[num_hge * 41];
			setgbuf(cbuf);
#endif
			if(view)
				for (int i = 0; i < view->size; ++i){
					print2_impl<cols...>(f, (*view)[i], printf_string.c_str());
					setgbuf();
				}
			else
				for (int i = 0; i < colrefs[0].size; ++i){
					print2_impl<cols...>(f, i, printf_string.c_str());		
					setgbuf();
				}
		};

		if (fp)
		{
			fprintf(fp, "%s%s", header_string.c_str(), end);
			prt_loop([&fp](auto... args) { fprintf(fp, args...); });
		}
		else {
			printf("%s%s", header_string.c_str(), end);
			prt_loop(printf);
		}
	}
	template <int ...vals> struct applier {
	inline constexpr static void apply(const TableInfo<Types...>& t, const char* __restrict sep = ",", const char* __restrict end = "\n",
		const vector_type<uint32_t>* __restrict view = nullptr, FILE* __restrict fp = nullptr)
	{ t.template print2<vals ...>(sep, end, view, fp); }};
	
	inline void printall(const char* __restrict sep = ",", const char* __restrict end = "\n",
		const vector_type<uint32_t>* __restrict view = nullptr, FILE* __restrict fp = nullptr) {
		applyIntegerSequence<sizeof...(Types), applier>::apply(*this, sep, end, view, fp);
	}

	TableInfo< Types... >* rename(const char* name) {
		this->name = name;
		return this;
	}
	// defined in monetdb_conn.cpp
	void monetdb_append_table(void* srv, const char* alt_name = nullptr);
};


template<class ...Types>
struct TableView {
	const vector_type<uint32_t>* idxs;
	const TableInfo<Types...>& info;
	constexpr TableView(const vector_type<uint32_t>* idxs, const TableInfo<Types...>& info) noexcept : idxs(idxs), info(info) {}
	void print(const char* __restrict sep, const char* __restrict end) const;
	template <size_t j = 0>
	typename std::enable_if<j == sizeof...(Types) - 1, void>::type print_impl(const uint32_t& i, const char* __restrict sep = " ") const;
	template <size_t j = 0>
	typename std::enable_if < j < sizeof...(Types) - 1, void>::type print_impl(const uint32_t& i, const char* __restrict sep = " ") const;
	
	~TableView() {
		delete idxs;
	}
};

template <class T>
constexpr static inline bool is_vector(const ColRef<T>&) {
	return true;
}
template <class T>
constexpr static inline bool is_vector(const vector_type<T>&) {
	return true;
}

template<class ...Types>
TableInfo<Types...>::TableInfo(const char* name, uint32_t n_cols) : name(name), n_cols(n_cols) {
	this->colrefs = (ColRef<void>*)malloc(sizeof(ColRef<void>) * n_cols);
	for (uint32_t i = 0; i < n_cols; ++i) {
		this->colrefs[i].init();
	}
}
template<class ...Types>
TableInfo<Types...>::TableInfo(const char* name, const char** col_names) : name(name), n_cols(sizeof...(Types)) {
	this->colrefs = (ColRef<void>*)malloc(sizeof(ColRef<void>) * this->n_cols);
	for (uint32_t i = 0; i < n_cols; ++i) {
		this->colrefs[i].init(col_names? col_names[i] : "");
	}
}
template <class ...Types>
template <size_t j>
inline typename std::enable_if<j == sizeof...(Types) - 1, void>::type
TableView<Types ...>::print_impl(const uint32_t& i, const char* __restrict sep) const {
	std::cout << (get<j>(*this))[(*idxs)[i]];
}

template<class ...Types>
template<size_t j>
inline typename std::enable_if < j < sizeof...(Types) - 1, void>::type
	TableView<Types...>::print_impl(const uint32_t& i, const char* __restrict sep) const
{
	std::cout << (get<j>(*this))[(*idxs)[i]] << sep;
	print_impl<j + 1>(i, sep);
}

template<class ...Types>
inline void TableView<Types...>::print(const char* __restrict sep, const char* __restrict end) const {
	std::string header_string = info.get_header_string(sep, end);
	std::cout << header_string.c_str();
	
	int n_rows = 0;
	if (info.colrefs[0].size > 0)
		n_rows = info.colrefs[0].size;
	for (int i = 0; i < n_rows; ++i) {
		print_impl(i);
		std::cout << end;
	}
}
template <class ...Types>
template <size_t j>
inline typename std::enable_if<j == sizeof...(Types) - 1, void>::type
	TableInfo<Types ...>::print_impl(const uint32_t& i, const char* __restrict sep) const {
	std::cout << (get<j>(*this))[i];
}

template<class ...Types>
template<size_t j>
inline typename std::enable_if<j < sizeof...(Types) - 1, void>::type 
	TableInfo<Types...>::print_impl(const uint32_t& i, const char* __restrict sep) const
{
	std::cout << (get<j>(*this))[i] << sep;
	print_impl<j+1>(i, sep);
}

template<class ...Types>
inline void TableInfo<Types...>::print(const char* __restrict sep, const char* __restrict end) const {
	
	std::string header_string = get_header_string(sep, end);
	std::cout << header_string.c_str();
	
	int n_rows = 0;
	if (n_cols > 0 && colrefs[0].size > 0)
		n_rows = colrefs[0].size;
	for (int i = 0; i < n_rows; ++i) {
		print_impl(i);
		std::cout << end;
	}
}
template <class T1, class T2, template<typename ...> class VT, template<typename ...> class VT2>
decayed_t<VT, typename types::Coercion<T1, T2>::type> operator -(const VT<T1>& lhs, const VT2<T2>& rhs) {
	auto ret = decayed_t<VT, typename types::Coercion<T1, T2>::type>(lhs.size);
	for (int i = 0; i < lhs.size; ++i)
		ret[i] = lhs[i] - rhs[i];
	return ret;
}
template <class T1, class T2, template<typename ...> class VT>
decayed_t<VT, typename types::Coercion<T1, T2>::type> operator -(const VT<T1>& lhs, const T2& rhs) {
	auto ret = decayed_t<VT, typename types::Coercion<T1, T2>::type>(lhs.size);
	for (int i = 0; i < lhs.size; ++i)
		ret[i] = lhs[i] - rhs;
	return ret;
}
template <class T1, class T2, template<typename ...> class VT>
decayed_t<VT, typename types::Coercion<T1, T2>::type> operator -(const T2& lhs, const VT<T1>& rhs) {
	auto ret = decayed_t<VT, typename types::Coercion<T1, T2>::type>(rhs.size);
	for (int i = 0; i < rhs.size; ++i)
		ret[i] = lhs - rhs[i];
	return ret;
}
template <class T1, class T2, template<typename ...> class VT, template<typename ...> class VT2>
decayed_t<VT, typename types::Coercion<T1, T2>::type> operator +(const VT<T1>& lhs, const VT2<T2>& rhs) {
	auto ret = decayed_t<VT, typename types::Coercion<T1, T2>::type>(lhs.size);
	for (int i = 0; i < lhs.size; ++i)
		ret[i] = lhs[i] + rhs[i];
	return ret;
}
template <class T1, class T2, template<typename ...> class VT>
decayed_t<VT, typename types::Coercion<T1, T2>::type> operator +(const VT<T1>& lhs, const T2& rhs) {
	auto ret = decayed_t<VT, typename types::Coercion<T1, T2>::type>(lhs.size);
	for (int i = 0; i < lhs.size; ++i)
		ret[i] = lhs[i] + rhs;
	return ret;
}
template <class T1, class T2, template<typename ...> class VT>
decayed_t<VT, typename types::Coercion<T1, T2>::type> operator +(const T2& lhs, const VT<T1>& rhs) {
	auto ret = decayed_t<VT, typename types::Coercion<T1, T2>::type>(rhs.size);
	for (int i = 0; i < rhs.size; ++i)
		ret[i] = lhs + rhs[i];
	return ret;
}
template <class T1, class T2, template<typename ...> class VT, template<typename ...> class VT2>
decayed_t<VT, typename types::Coercion<T1, T2>::type> operator *(const VT<T1>& lhs, const VT2<T2>& rhs) {
	auto ret = decayed_t<VT, typename types::Coercion<T1, T2>::type>(lhs.size);
	for (int i = 0; i < lhs.size; ++i)
		ret[i] = lhs[i] * rhs[i];
	return ret;
}
template <class T1, class T2, template<typename ...> class VT>
decayed_t<VT, typename types::Coercion<T1, T2>::type> operator *(const VT<T1>& lhs, const T2& rhs) {
	auto ret = decayed_t<VT, typename types::Coercion<T1, T2>::type>(lhs.size);
	for (int i = 0; i < lhs.size; ++i)
		ret[i] = lhs[i] * rhs;
	return ret;
}
template <class T1, class T2, template<typename ...> class VT>
decayed_t<VT, typename types::Coercion<T1, T2>::type> operator *(const T2& lhs, const VT<T1>& rhs) {
	auto ret = decayed_t<VT, typename types::Coercion<T1, T2>::type>(rhs.size);
	for (int i = 0; i < rhs.size; ++i)
		ret[i] = lhs * rhs[i];
	return ret;
}
template <class T1, class T2, template<typename ...> class VT, template<typename ...> class VT2>
decayed_t<VT, types::GetFPType<typename types::Coercion<T1, T2>::type>> operator /(const VT<T1>& lhs, const VT2<T2>& rhs) {
	auto ret = decayed_t<VT, types::GetFPType<typename types::Coercion<T1, T2>::type>>(lhs.size);
	for (int i = 0; i < lhs.size; ++i)
		ret[i] = lhs[i] / rhs[i];
	return ret;
}
template <class T1, class T2, template<typename ...> class VT>
decayed_t<VT, types::GetFPType<typename types::Coercion<T1, T2>::type>> operator /(const VT<T1>& lhs, const T2& rhs) {
	auto ret = decayed_t<VT, types::GetFPType<typename types::Coercion<T1, T2>::type>>(lhs.size);
	for (int i = 0; i < lhs.size; ++i)
		ret[i] = lhs[i] / rhs;
	return ret;
}
template <class T1, class T2, template<typename ...> class VT>
decayed_t<VT, types::GetFPType<typename types::Coercion<T1, T2>::type>> operator /(const T2& lhs, const VT<T1>& rhs) {
	auto ret = decayed_t<VT, types::GetFPType<typename types::Coercion<T1, T2>::type>>(rhs.size);
	for (int i = 0; i < rhs.size; ++i)
		ret[i] = lhs / rhs[i];
	return ret;
}

template <class ...Types>
void print(const TableInfo<Types...>& v, const char* delimiter = " ", const char* endline = "\n") {
	v.print(delimiter, endline);
}
template <class ...Types>
void print(const TableView<Types...>& v, const char* delimiter = " ", const char* endline = "\n") {
	v.print(delimiter, endline);
}
template <class T>
void print(const T& v, const char* delimiter = " ") {
	std::cout<< v;
	// printf(types::printf_str[types::Types<T>::getType()], v);
}
#ifdef __SIZEOF_INT128__
template <>
void print<__int128_t>(const __int128_t& v, const char* delimiter);
template <>
void print<__uint128_t>(const __uint128_t&v, const char* delimiter);
#endif
template <>
void print<bool>(const bool&v, const char* delimiter);

template <class T>
void inline print_impl(const T& v, const char* delimiter, const char* endline) {
	for (const auto& vi : v) {
		print(vi);
		std::cout << delimiter;
		// printf("%s", delimiter);
	}
	std::cout << endline;
	//printf("%s", endline);
}

template <class T, template<typename> class VT>
typename std::enable_if<!std::is_same<VT<T>, TableInfo<T>>::value>::type 
print(const VT<T>& v, const char* delimiter = " ", const char* endline = "\n") {
	print_impl(v, delimiter, endline);
}
 

#endif
