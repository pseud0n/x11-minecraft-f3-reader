#ifndef DELAYED_FUNCTION_HPP
#define DELAYED_FUNCTION_HPP

#include <iostream>
#include <functional>
#include <tuple>
/*
void add(int x, int y) {
    std::cout << (x + y + 0.5) << "\n";
}
*/

template <typename... Ts>
class DelayedFunction {
public:
    using TupleT = std::tuple<Ts...>;
    using FunctionT = std::function<void(Ts...)>;
    using NumT = int;
//private:
    NumT remaining;
    const NumT initialRemaining;
    TupleT arguments;
    FunctionT const function;
    template <size_t... Is>
    void callHelper(std::index_sequence<Is...>);
//public:
    DelayedFunction(NumT, FunctionT const &, Ts...);
    void CountDown();
    void CountDownOrSilence();
    void CountDownOrResetCount();
    void CountDownOrSetCount(NumT);
    void SetCount(NumT);
    NumT GetCount();
    void CallNow();
};



#include <sstream>
#include <exception>

template <typename... Ts>
DelayedFunction<Ts...>::DelayedFunction(DelayedFunction::NumT remaining, DelayedFunction::FunctionT const & function, Ts... args)
	: remaining(remaining),
	  initialRemaining(remaining),
	  arguments(std::forward<Ts>(args)...),
	  function(function) {
}

template <typename... Ts>
template <size_t... Is>
void DelayedFunction<Ts...>::callHelper(std::index_sequence<Is...>) {
    //std::clog << function << "\n";
	function(std::forward<Ts>(std::get<Is>(arguments))...);
	//function();
}

template <typename... Ts>
void DelayedFunction<Ts...>::CountDown() {
    std::clog << "Counting down " << remaining << "\n";
	if (remaining == 0)
		CallNow();
	else if (remaining < 0) {
		std::ostringstream oss;
		oss << "DelayedFunction " << this << " has already been called (DelayedFunction.cpp); throwing exception";
		throw std::out_of_range(oss.str());
	}
	--remaining;
}

template <typename... Ts>
void DelayedFunction<Ts...>::CountDownOrSilence() {
	if (remaining == 0)
		CallNow();
	if (remaining >= 0)
		--remaining;
}

template <typename...Ts>
void DelayedFunction<Ts...>::CountDownOrResetCount() {
	CountDownOrSetCount(initialRemaining);
}

template <typename...Ts>
void DelayedFunction<Ts...>::CountDownOrSetCount(DelayedFunction<>::NumT to) {
	CountDown();
	if (remaining == -1) {
		remaining = to;
	}
}

template <typename...Ts>
void DelayedFunction<Ts...>::SetCount(DelayedFunction<>::NumT to) {
	remaining = to;
}

template <typename...Ts>
DelayedFunction<>::NumT DelayedFunction<Ts...>::GetCount() {
	return remaining;
}

template <typename...Ts>
void DelayedFunction<Ts...>::CallNow() {
	callHelper(std::make_index_sequence<sizeof... (Ts)>{});
}

#endif
