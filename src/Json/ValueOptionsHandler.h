#pragma once

#include <rapidjson/reader.h>
#include "ValueSource.h"

struct ValueOptionsData
{
	float Min = 0.0f;
	float Max = 0.0f;
	float Step = 0.0f;

	std::string Value;
	std::vector<std::string> Options;
	std::vector<std::string> ShortNames;

	std::string SourceType;
	RE::TESForm* SourceForm;
	std::string ScriptName;
	std::string PropertyName;
	float DefaultValue = 0.0f;
	std::string DefaultValueStr = "";
	std::shared_ptr<ValueSource> ValueSource;
};

class ValueOptionsHandler
{
public:
	using Ch = rapidjson::UTF8<>::Ch;
	using SizeType = rapidjson::SizeType;

	ValueOptionsHandler(
		ValueOptionsData* data,
		const std::string& modName,
		const std::string& id,
		RE::TESForm* sourceForm,
		const std::string& scriptName);

	bool Complete();

	bool Bool(bool b);
	bool Int(int i);
	bool Uint(unsigned i);
	bool Double(double d);
	bool String(const Ch* str, SizeType length, bool copy);
	bool StartObject();
	bool Key(const Ch* str, SizeType length, bool copy);
	bool EndObject(SizeType memberCount);
	bool StartArray();
	bool EndArray(SizeType elementCount);

private:
	enum class State
	{
		End,
		Start,
		Min,
		Max,
		Step,
		Value,
		Options,
		OptionElem,
		ShortNames,
		ShortNameElem,
		SourceType,
		SourceForm,
		ScriptName,
		PropertyName,
		DefaultValue,
	};

	State _state = State::End;
	ValueOptionsData* _data;
	std::string _modName;
	std::string _id;
	RE::TESForm* _sourceForm;
	std::string _scriptName;
};
