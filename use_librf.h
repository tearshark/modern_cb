//
//ͨ���ִ��ص�(Modern Callback)�� ʹ�ûص�������ģ�ͣ�
//���첽�ص�������չ��֧��futureģʽ��������ģʽ���Լ�Э�̡�

#pragma once
#include "future_detail.h"

namespace modern_callback
{
	namespace detail
	{
		//ͬ����������֧��C++20��Э�̵�����һϵ���ࣨ��ʵ��������ҵ�����Ŀ�ģ�
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

	//���ԣ������ڵĿ����ǣ�֧���첽�����Ŀ⣬�����������ƻص�����������֧��C++20��Э�̡��Լ�future::then��������������
	//����ǡ�Ħ��C++����
}

constexpr modern_callback::detail::use_librf_t use_librf{};
