//
//ͨ���ִ��ص�(Modern Callback)�� ʹ�ûص�������ģ�ͣ�
//���첽�ص�������չ��֧��futureģʽ��������ģʽ���Լ�Э�̡�

#pragma once
#include <future>
#include "future_detail.h"

namespace modern_callback
{
	namespace detail
	{
		//һ����һ��ʹ��std::promise/std::future�ĸ����ࡣ
		//����໹������ȡpromise/future�Ե����͡�
		struct std_future_t
		{
			template<typename _Result_t>
			using promise_type = std::promise<_Result_t>;

			template<typename _Result_t>
			using future_type = std::future<_Result_t>;
		};
	}

	//����ƫ�ػ�_Callable_tΪstd_future_t���͵�adapter_t
	//�����Ļص�������callback_t����������_Return_t��return_t��
	//���callback_t��promise<result_type>����return_t��future<result_type>���������һ��promise/future�ԡ�
	//promise�������Ļص������ý��ֵ��
	//future���ظ������߻�ȡ���ֵ��
	template<typename R, typename... _Result_t>
	struct adapter_t<detail::std_future_t, R(_Result_t...)> : public future_detail::adapter_impl_t<detail::std_future_t, _Result_t...>
	{
	};
}

//������������������ȫ�ֱ������������������Ҳ�У�����ÿ��Ҫдuse_future_t{}���鷳Щ��
//�Ժ��ʹ��std_future������첽������token�����ˡ�
//���������ʵ����Ҫʵ�ʴ��Σ����ᱻ�������Ż�û�ˡ�
//������Ҫָ��_Callable_t������Ϊstd_future_t��
//�Ӷ����첽�����ڣ�ʹ��ƫ�ػ���adapter_t<std_future_t, ...>���ѡ�
constexpr modern_callback::detail::std_future_t std_future{};
