//
//通过现代回调(Modern Callback)， 使用回调适配器模型，
//将异步回调函数扩展到支持future模式，调用链模式，以及协程。
//
//future库有多种，但应当都提供遵循promise/future对，兼容std::promise/std::future用法
//这样的话，可以做一个更加通用的支持future的callback类

#pragma once

namespace modern_callback
{
	namespace future_detail
	{
		//实现callback_t的基类，避免写一些重复代码
		template<typename _Promise_traits, typename _Result_t>
		struct callback_base_t
		{
			//回调函数的结果类型，已经排除掉了异常参数
			using result_type = _Result_t;

			//通过_Promise_traits获取真正的promise类型
			using promise_type = typename _Promise_traits::template promise_type<result_type>;

			//通过_Promise_traits获取真正的future类型
			using future_type = typename _Promise_traits::template future_type<result_type>;

			//此类持有一个std::promise<_Result_t>，便于设置值和异常
			//而将与promise关联的future作为返回值_Return_t，让tostring_async返回。
			mutable promise_type _promise;

			auto get_future() const
			{
				return this->_promise.get_future();
			}
		};

		//此类的实例作为真正的callback，交给异步回调函数，替换token。
		//在实际应用中，需要针对是否有异常参数，结果值为0，1，多个等情况做特殊处理，故还需要通过更多的偏特化版本来支持。
		//具体的异常参数，需要根据实际应用去特里化。这里仅演示通过std::exception_ptr作为异常传递的情况。
		template<typename...>
		struct callback_t;

		//无异常，无结果的callback类型：void()
		template<typename _Promise_traits>
		struct callback_t<_Promise_traits> : public callback_base_t<_Promise_traits, void>
		{
			using callback_base_t<_Promise_traits, void>::callback_base_t;

			void operator()() const
			{
				promise_type p = std::move(this->_promise);	//杜绝可能this在回调中被析构
				p.set_value();
			}
		};

		//有异常，无结果的callback类型：void(exception_ptr)
		template<typename _Promise_traits>
		struct callback_t<_Promise_traits, std::exception_ptr> : public callback_base_t<_Promise_traits, void>
		{
			using callback_base_t<_Promise_traits, void>::callback_base_t;

			void operator()(std::exception_ptr eptr) const
			{
				promise_type p = std::move(this->_promise);
				if (!eptr)
					p.set_value();
				else
					p.set_exception(std::move(eptr));
			}
		};

		//无异常，单结果的callback类型：void(_Result_t)
		template<typename _Promise_traits, typename _Result_t>
		struct callback_t<_Promise_traits, _Result_t> : public callback_base_t<_Promise_traits, _Result_t>
		{
			using callback_base_t<_Promise_traits, _Result_t>::callback_base_t;

			template<typename Arg>
			void operator()(Arg&& arg) const
			{
				promise_type p = std::move(this->_promise);
				p.set_value(std::forward<Arg>(arg));
			}
		};

		//有异常，单结果的callback类型：void(std::exception_ptr, _Result_t)
		template<typename _Promise_traits, typename _Result_t>
		struct callback_t<_Promise_traits, std::exception_ptr, _Result_t> : public callback_base_t<_Promise_traits, _Result_t>
		{
			using callback_base_t<_Promise_traits, _Result_t>::callback_base_t;

			template<typename Arg>
			void operator()(std::exception_ptr eptr, Arg&& arg) const
			{
				promise_type p = std::move(this->_promise);
				if (!eptr)
					p.set_value(std::forward<Arg>(arg));
				else
					p.set_exception(std::move(eptr));
			}
		};

		//无异常，多结果的callback类型：void(_Result_t...)
		template<typename _Promise_traits, typename... _Result_t>
		struct callback_t<_Promise_traits, _Result_t...> : public callback_base_t<_Promise_traits, std::tuple<_Result_t...> >
		{
			using callback_base_t<_Promise_traits, std::tuple<_Result_t...> >::callback_base_t;

			template<typename... Args>
			void operator()(Args&&... args) const
			{
				static_assert(sizeof...(Args) == sizeof...(_Result_t), "");
				promise_type p = std::move(this->_promise);
				p.set_value(std::make_tuple(std::forward<Args>(args)...));
			}
		};

		//有异常，多结果的callback类型：void(std::exception_ptr, _Result_t...)
		template <typename _Promise_traits, typename... _Result_t>
		struct callback_t<_Promise_traits, std::exception_ptr, _Result_t...> : public callback_base_t<_Promise_traits, std::tuple<_Result_t...> >
		{
			using callback_base_t<_Promise_traits, std::tuple<_Result_t...> >::callback_base_t;

			template<typename... Args>
			void operator()(std::exception_ptr eptr, Args&&... args) const
			{
				static_assert(sizeof...(Args) == sizeof...(_Result_t), "");
				promise_type p = std::move(this->_promise);
				if (!eptr)
					p.set_value(std::make_tuple(std::forward<Args>(args)...));
				else
					p.set_exception(std::move(eptr));
			}
		};



		//与callback_t配套的获得_Return_t的类
		template<typename _Future_traits, typename _Result_t>
		struct return_t
		{
			using result_type = _Result_t;
			using future_type = typename _Future_traits::template future_type<result_type>;
			future_type _future;

			return_t(future_type&& ft)
				: _future(std::move(ft)) {}

			future_type get()
			{
				return std::move(_future);
			}
		};


		//利用callback_t + return_t 实现的callback适配器
		template<typename _Token_as_callable_t, typename... _Result_t>
		struct adapter_impl_t
		{
			using traits_type = std::remove_reference_t<_Token_as_callable_t>;
			using callback_type = callback_t<traits_type, _Result_t...>;
			using return_type = return_t<traits_type, typename callback_type::result_type>;
			using result_type = typename return_type::future_type;
			using future_type = typename return_type::future_type;

			static std::tuple<callback_type, return_type> traits(const _Token_as_callable_t& /*没人关心这个变量*/)
			{
				callback_type callback{};
				auto future = callback.get_future();

				return { std::move(callback), return_type{std::move(future)} };
			}
		};
	}
}
