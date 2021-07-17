#pragma once

#include <rapidjson/reader.h>
#include "Json/IHandler.h"

class ReaderHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, ReaderHandler>
{
public:
	using SizeType = rapidjson::SizeType;

	template <typename T, typename... Args>
	void PushHandler(Args&&... args)
	{
		_handlers.emplace(new T(std::forward<Args>(args)...));
	}

	void PopHandler()
	{
		_handlers.pop();
	}

	bool Null();
	bool Bool(bool b);
	bool Int(int i);
	bool Uint(unsigned i);
	bool Int64(std::int64_t i);
	bool Uint64(std::uint64_t i);
	bool Double(double d);
	bool RawNumber(const Ch* str, SizeType length, bool copy);
	bool String(const Ch* str, SizeType length, bool copy);
	bool StartObject();
	bool Key(const Ch* str, SizeType length, bool copy);
	bool EndObject(SizeType memberCount);
	bool StartArray();
	bool EndArray(SizeType elementCount);

private:
	std::stack<std::unique_ptr<IHandler>> _handlers;
};