//
//通过现代回调(Modern Callback)， 使用回调适配器模型，
//将异步回调函数扩展到支持future模式，调用链模式，以及协程。

#pragma once
#include <future>
#include "future_detail.h"

namespace modern_callback
{
	namespace detail
	{
		//一、做一个使用std::promise/std::future的辅助类。
		//这个类还负责萃取promise/future对的类型。
		struct std_future_t
		{
			template<typename _Result_t>
			using promise_type = std::promise<_Result_t>;

			template<typename _Result_t>
			using future_type = std::future<_Result_t>;
		};
	}

	//二、偏特化_Callable_t为std_future_t类型的adapter_t
	//真正的回调类型是callback_t，返回类型_Return_t是return_t。
	//配合callback_t的promise<result_type>，和return_t的future<result_type>，正好组成一对promise/future对。
	//promise在真正的回调里设置结果值；
	//future返回给调用者获取结果值。
	template<typename R, typename... _Result_t>
	struct adapter_t<detail::std_future_t, R(_Result_t...)> : public future_detail::adapter_impl_t<detail::std_future_t, _Result_t...>
	{
	};
}

//三、申明这个辅助类的全局变量。不申明这个变量也行，就是每次要写use_future_t{}，麻烦些。
//以后就使用std_future，替代异步函数的token参数了。
//这个参数其实不需要实质传参，最后会被编译器优化没了。
//仅仅是要指定_Callable_t的类型为std_future_t，
//从而在异步函数内，使用偏特化的adapter_t<std_future_t, ...>而已。
constexpr modern_callback::detail::std_future_t std_future{};
