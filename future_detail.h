//
//ͨ���ִ��ص�(Modern Callback)�� ʹ�ûص�������ģ�ͣ�
//���첽�ص�������չ��֧��futureģʽ��������ģʽ���Լ�Э�̡�
//
//future���ж��֣���Ӧ�����ṩ��ѭpromise/future�ԣ�����std::promise/std::future�÷�
//�����Ļ���������һ������ͨ�õ�֧��future��callback��

#pragma once

namespace modern_callback
{
	namespace future_detail
	{
		//ʵ��callback_t�Ļ��࣬����дһЩ�ظ�����
		template<typename _Promise_traits, typename _Result_t>
		struct callback_base_t
		{
			//�ص������Ľ�����ͣ��Ѿ��ų������쳣����
			using result_type = _Result_t;

			//ͨ��_Promise_traits��ȡ������promise����
			using promise_type = typename _Promise_traits::template promise_type<result_type>;

			//�������һ��std::promise<_Result_t>����������ֵ���쳣
			//������promise������future��Ϊ����ֵ_Return_t����tostring_async���ء�
			mutable promise_type _promise;

			auto get_future() const
			{
				return this->_promise.get_future();
			}
		};

		//�����ʵ����Ϊ������callback�������첽�ص��������滻token��
		//��ʵ��Ӧ���У���Ҫ����Ƿ����쳣���������ֵΪ0��1���������������⴦���ʻ���Ҫͨ�������ƫ�ػ��汾��֧�֡�
		//������쳣��������Ҫ����ʵ��Ӧ��ȥ���ﻯ���������ʾͨ��std::exception_ptr��Ϊ�쳣���ݵ������
		template<typename...>
		struct callback_t;

		//���쳣���޽����callback���ͣ�void()
		template<typename _Promise_traits>
		struct callback_t<_Promise_traits> : public callback_base_t<_Promise_traits, void>
		{
			using callback_base_t<_Promise_traits, void>::callback_base_t;

			void operator()() const
			{
				this->_promise.set_value();
			}
		};

		//���쳣���޽����callback���ͣ�void(exception_ptr)
		template<typename _Promise_traits>
		struct callback_t<_Promise_traits, std::exception_ptr> : public callback_base_t<_Promise_traits, void>
		{
			using callback_base_t<_Promise_traits, void>::callback_base_t;

			void operator()(std::exception_ptr eptr) const
			{
				if (!eptr)
					this->_promise.set_value();
				else
					this->_promise.set_exception(std::move(eptr));
			}
		};

		//���쳣���������callback���ͣ�void(_Result_t)
		template<typename _Promise_traits, typename _Result_t>
		struct callback_t<_Promise_traits, _Result_t> : public callback_base_t<_Promise_traits, _Result_t>
		{
			using callback_base_t<_Promise_traits, _Result_t>::callback_base_t;

			template<typename Arg>
			void operator()(Arg&& arg) const
			{
				this->_promise.set_value(std::forward<Arg>(arg));
			}
		};

		//���쳣���������callback���ͣ�void(std::exception_ptr, _Result_t)
		template<typename _Promise_traits, typename _Result_t>
		struct callback_t<_Promise_traits, std::exception_ptr, _Result_t> : public callback_base_t<_Promise_traits, _Result_t>
		{
			using callback_base_t<_Promise_traits, _Result_t>::callback_base_t;

			template<typename Arg>
			void operator()(std::exception_ptr eptr, Arg&& arg) const
			{
				if (!eptr)
					this->_promise.set_value(std::forward<Arg>(arg));
				else
					this->_promise.set_exception(std::move(eptr));
			}
		};

		//���쳣��������callback���ͣ�void(_Result_t...)
		template<typename _Promise_traits, typename... _Result_t>
		struct callback_t<_Promise_traits, _Result_t...> : public callback_base_t<_Promise_traits, std::tuple<_Result_t...> >
		{
			using callback_base_t<_Promise_traits, std::tuple<_Result_t...> >::callback_base_t;

			template<typename... Args>
			void operator()(Args&&... args) const
			{
				static_assert(sizeof...(Args) == sizeof...(_Result_t), "");
				this->_promise.set_value(std::make_tuple(std::forward<Args>(args)...));
			}
		};

		//���쳣��������callback���ͣ�void(std::exception_ptr, _Result_t...)
		template <typename _Promise_traits, typename... _Result_t>
		struct callback_t<_Promise_traits, std::exception_ptr, _Result_t...> : public callback_base_t<_Promise_traits, std::tuple<_Result_t...> >
		{
			using callback_base_t<_Promise_traits, std::tuple<_Result_t...> >::callback_base_t;

			template<typename... Args>
			void operator()(std::exception_ptr eptr, Args&&... args) const
			{
				static_assert(sizeof...(Args) == sizeof...(_Result_t), "");
				if (!eptr)
					this->_promise.set_value(std::make_tuple(std::forward<Args>(args)...));
				else
					this->_promise.set_exception(std::move(eptr));
			}
		};



		//��callback_t���׵Ļ��_Return_t����
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


		//����callback_t + return_t ʵ�ֵ�callback������
		template<typename _Token_as_callable_t, typename... _Result_t>
		struct adapter_impl_t
		{
			using traits_type = std::remove_reference_t<_Token_as_callable_t>;
			using callback_type = callback_t<traits_type, _Result_t...>;
			using result_type = typename callback_type::result_type;
			using return_type = return_t<traits_type, result_type>;

			static std::tuple<callback_type, return_type> traits(const _Token_as_callable_t& /*û�˹����������*/)
			{
				callback_type callback{};
				auto future = callback.get_future();

				return { std::move(callback), return_type{std::move(future)} };
			}
		};
	}
}
