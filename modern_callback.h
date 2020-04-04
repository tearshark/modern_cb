//�ִ��ص�(Modern Callback)
//
//һ��ʹ�ûص����������첽���������漰���¸��
//_Input_t...���첽���������������
//_Signature_t: ���첽�ص��ĺ���ǩ����Ӧ�����㡮void(_Exception_t, _Result_t...)�����ߡ�void(_Result_t...)�����ͣ�
//_Callable_t���ص��������ǣ�����ǻص�����������Ҫ����_Signature_t��ǩ�����͡�����ص����������һ�Σ���ֻ�ܵ���һ�Σ�
//_Return_t���첽�����ķ���ֵ��
//_Result_t...���첽������ɺ�Ľ��ֵ����Ϊ�ص���������β��֣�����������������������
//_Exception_t���ص��������쳣�� �����ϲ���쳣�������������֣����͵��첽���뽫�쳣�����׵���
//
//�ڻص�������ģ���_Input_t.../_Result_t/_Exception_t(��ѡ)���첽�����ṩ�Ĺ��������еĲ��֣�_Callable_t/_Return_t
//���ֲ���ֱ��ʹ�ã�����ͨ��������ȥ���⴦����������������һ����չ��futureģʽ��������ģʽ�Ļ��ᣬ�Լ�֧��Э�̵Ļ��ᡣ

#pragma once

#include <tuple>

//׼��return_void_t��adapter_t���첽����ʹ��
namespace modern_callback
{
	//ͨ��һ����ӵ������������void���﷨���⣬�Ա����Ż�����ֵ
	struct return_void_t
	{
		void get() {}
	};

	//�ص���������ģ����
	//_Callable_t Ҫ���� _Signature_t ǩ��
	//��������ת��token�⣬�����κ���Ч�Ĺ���
	//��Ч�����ȴ����л�����ȥ��
	template<typename _Callable_t, typename _Signature_t>
	struct adapter_t
	{
		using callback_type = _Callable_t;
		using return_type = return_void_t;

		static std::tuple<callback_type, return_type> traits(_Callable_t&& token)
		{
			return { std::forward<_Callable_t>(token), {} };
		}
	};
}

//���ߺ�汾д��
#define MODERN_CALLBACK_TRAITS(_Token_value, _Signature_t) \
	using _Adapter_t__ = modern_callback::adapter_t<std::remove_cv_t<std::remove_reference_t<_Callable_t>>, _Signature_t>; \
	auto _Adapter_value__ = _Adapter_t__::traits(std::forward<_Callable_t>(_Token_value))
#define MODERN_CALLBACK_CALL() std::move(std::get<0>(_Adapter_value__))
#define MODERN_CALLBACK_RETURN() return std::move(std::get<1>(_Adapter_value__)).get()

#if 0
//tostring_async ��ʾ���������߳����_Input_t������ֵ��ת��Ϊstd::string���͵�_Result_t��
//Ȼ�����_Signature_tΪ ��void(std::string &&)�� ���͵� _Callable_t��
//�����쳣������û��_Exception_t��
//
template<typename _Input_t, typename _Callable_t>
auto tostring_async(_Input_t&& value, _Callable_t&& token)
{
	//����������
	using _Adapter_t = modern_callback::adapter_t<typename std::remove_reference_t<std::remove_cv_t<_Callable_t>>, void(std::string)>;
	//ͨ����������ü���_Signature_t���͵������Ļص����Լ�����ֵ_Return_t
	auto adapter = _Adapter_t::traits(std::forward<_Callable_t>(token));

	//callback��tokenδ����ͬһ������������δ����ͬһ������
	std::thread([callback = std::move(std::get<0>(adapter)), value = std::forward<_Input_t>(value)]
		{
			using namespace std::literals;
			std::this_thread::sleep_for(0.1s);
			callback(std::to_string(value));
		}).detach();

	//������������_Return_t����
	return std::move(std::get<1>(adapter)).get();
}
#endif
