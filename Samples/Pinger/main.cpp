#include <stormancer.h>

int main()
{
	{
		Stormancer::MsgPackMaybe<int> mb1;
		mb1 = new int(127);
		std::stringstream ss;
		msgpack::pack(ss, mb1);
		std::string str = ss.str();
		msgpack::unpacked ret;
		msgpack::unpack(ret, str.data(), str.size());
		Stormancer::MsgPackMaybe<int> mb2;
		ret.get().convert(&mb2);
		auto b1 = mb2.hasValue();
		if (b1)
		{
			auto i1 = mb2.get();
			std::cout << *i1;
		}
		else
		{
			std::cout << "nil";
		}
	}

	std::cin.ignore();

	return 0;
}
