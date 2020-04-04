//
//通过现代回调(Modern Callback)， 使用回调适配器模型，
//将异步回调函数扩展到支持future模式，调用链模式，以及协程。

#pragma once
#include "future_detail.h"

namespace modern_callback
{
	namespace detail
	{
		//同理，可以制作支持C++20的协程的下列一系列类（其实，这才是我的最终目的）
		struct use_librf_t
		{
			template<typename _Result_t>
			using promise_type = resumef::awaitable_t<_Result_t>;

			template<typename _Result_t>
			using future_type = resumef::future_t<_Result_t>;
		};
	}

	template<typename R, typename... _Result_t>
	struct adapter_t<detail::use_librf_t, R(_Result_t...)> : public future_detail::adapter_impl_t<detail::use_librf_t, _Result_t...>
	{
	};

	//所以，我现在的看法是，支持异步操作的库，尽可能如此设计回调。这样便于支持C++20的协程。以及future::then这样的任务链。
	//这才是“摩登C++”！
}

constexpr modern_callback::detail::use_librf_t use_librf{};
