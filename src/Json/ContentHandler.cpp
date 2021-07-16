#include "Json/ContentHandler.h"
#include "Utils.h"

ContentHandler::ContentHandler(PageLayout* pageLayout, const ScriptObjectPtr& script) :
	_pageLayout{ pageLayout }
{
	_form = static_cast<RE::TESForm*>(script->Resolve(0));
	_modName = Utils::GetModName(_form);
	_scriptName = script->GetTypeInfo()->GetName();
}

bool ContentHandler::Complete()
{
	return _state == State::End;
}

bool ContentHandler::Bool(bool b)
{
	switch (_state) {
	case State::IgnoreConflicts:
		_data.IgnoreConflicts = b;
		_state = State::Control;
		return true;
	case State::Action:
		return _action->Bool(b);
	default:
		return false;
	}
}

bool ContentHandler::Int(int i)
{
	switch (_state) {
	case State::Action:
		return _action->Int(i);
	case State::ValueOptions:
		return _valueOptions->Int(i);
	default:
		return false;
	}
}

bool ContentHandler::Uint(unsigned i)
{
	switch (_state) {
	case State::Position:
		_data.Position = static_cast<std::int32_t>(i);
		_state = State::Control;
		return true;
	case State::GroupControl:
		_data.GroupControl = static_cast<std::uint32_t>(i);
		_state = State::Control;
		return true;
	case State::GroupCondition:
	{
		bool groupConditionOK = _groupCondition->Uint(i);
		if (_groupCondition->Complete())
			_state = State::Control;
		return groupConditionOK;
	}
	case State::Action:
		return _action->Uint(i);
	case State::ValueOptions:
		return _valueOptions->Uint(i);
	default:
		return false;
	}
}

bool ContentHandler::Double(double d)
{
	switch (_state) {
	case State::Action:
		return _action->Double(d);
	case State::ValueOptions:
		return _valueOptions->Double(d);
	default:
		return false;
	}
}

bool ContentHandler::String(const Ch* str, SizeType length, bool copy)
{
	switch (_state) {
	case State::ID:
		_data.ID = str;
		_state = State::Control;
		return true;
	case State::Text:
		_data.Text = str;
		_state = State::Control;
		return true;
	case State::Help:
		_data.Help = str;
		_state = State::Control;
		return true;
	case State::Type:
		_data.Type = str;
		_state = State::Control;
		return true;
	case State::GroupBehavior:
		if (strcmp(str, "disable") == 0) {
			_data.GroupBehavior = Control::Behavior::Disable;
			_state = State::Control;
			return true;
		}
		else if (strcmp(str, "hide") == 0) {
			_data.GroupBehavior = Control::Behavior::Hide;
			_state = State::Control;
			return true;
		}
		else if (strcmp(str, "skip") == 0) {
			_data.GroupBehavior = Control::Behavior::Skip;
			_state = State::Control;
			return true;
		}
		else {
			return false;
		}
	case State::FormatString:
		_data.FormatString = str;
		_state = State::Control;
		return true;
	case State::Action:
		return _action->String(str, length, copy);
	case State::ValueOptions:
		return _valueOptions->String(str, length, copy);
	default:
		return false;
	}
}

bool ContentHandler::StartObject()
{
	switch (_state) {
	case State::Start:
		_state = State::Control;
		return true;
	case State::GroupCondition:
		return _groupCondition->StartObject();
	case State::Action:
		return _action->StartObject();
	case State::ValueOptions:
		return _valueOptions->StartObject();
	default:
		return false;
	}
}

bool ContentHandler::Key(const Ch* str, SizeType length, bool copy)
{
	switch (_state) {
	case State::Control:
		if (strcmp(str, "id") == 0) {
			_state = State::ID;
			return true;
		}
		else if (strcmp(str, "position") == 0) {
			_state = State::Position;
			return true;
		}
		else if (strcmp(str, "text") == 0) {
			_state = State::Text;
			return true;
		}
		else if (strcmp(str, "help") == 0) {
			_state = State::Help;
			return true;
		}
		else if (strcmp(str, "type") == 0) {
			_state = State::Type;
			return true;
		}
		else if (strcmp(str, "groupCondition") == 0) {
			_data.GroupCondition = std::make_shared<GroupConditionTree>();
			_groupCondition = std::make_unique<GroupConditionHandler>(_data.GroupCondition);
			_state = State::GroupCondition;
			return true;
		}
		else if (strcmp(str, "groupBehavior") == 0) {
			_state = State::GroupBehavior;
			return true;
		}
		else if (strcmp(str, "groupControl") == 0) {
			_state = State::GroupControl;
			return true;
		}
		else if (strcmp(str, "formatString") == 0) {
			_state = State::FormatString;
			return true;
		}
		else if (strcmp(str, "ignoreConflicts") == 0) {
			_state = State::IgnoreConflicts;
			return true;
		}
		else if (strcmp(str, "action") == 0) {
			_state = State::Action;
			_action = std::make_unique<ActionHandler>(&_data.Action, _form, _scriptName);
			return true;
		}
		else if (strcmp(str, "valueOptions") == 0) {
			_state = State::ValueOptions;
			_valueOptions = std::make_unique<ValueOptionsHandler>(
				&_data.ValueOptions,
				_modName,
				_data.ID,
				_form,
				_scriptName);
			return true;
		}
		else {
			return false;
		}
	case State::GroupCondition:
		return _groupCondition->Key(str, length, copy);
	case State::ValueOptions:
		return _valueOptions->Key(str, length, copy);
	case State::Action:
		return _action->Key(str, length, copy);
	default:
		return false;
	}
}

bool ContentHandler::EndObject(SizeType memberCount)
{
	switch (_state) {
	case State::Control:
	{
		std::shared_ptr<Control> control;
		if (_data.Type == "empty"s) {
			control = std::make_shared<EmptyControl>();
		}
		else if (_data.Type == "header"s) {
			control = std::make_shared<HeaderControl>();
		}
		else if (_data.Type == "text"s) {
			auto textOption = std::make_shared<TextControl>();
			textOption->Value = _data.ValueOptions.Value;
			textOption->ModName = _modName;
			textOption->SourceForm = _data.ValueOptions.SourceForm;
			textOption->ScriptName = _data.ValueOptions.ScriptName;
			textOption->PropertyName = _data.ValueOptions.PropertyName;
			control = textOption;
		}
		else if (_data.Type == "toggle"s) {
			auto toggleOption = std::make_shared<ToggleControl>();
			toggleOption->GroupControl = _data.GroupControl;
			toggleOption->ValueSource = _data.ValueOptions.ValueSource;
			control = toggleOption;
		}
		else if (_data.Type == "hiddenToggle"s) {
			auto hiddenToggleOption = std::make_shared<HiddenToggleControl>();
			hiddenToggleOption->GroupControl = _data.GroupControl;
			hiddenToggleOption->ValueSource = _data.ValueOptions.ValueSource;
			control = hiddenToggleOption;
		}
		else if (_data.Type == "slider"s) {
			auto sliderOption = std::make_shared<SliderControl>();
			sliderOption->Min = _data.ValueOptions.Min;
			sliderOption->Max = _data.ValueOptions.Max;
			sliderOption->Step = _data.ValueOptions.Step;
			sliderOption->FormatString = _data.FormatString;
			sliderOption->ValueSource = _data.ValueOptions.ValueSource;
			control = sliderOption;
		}
		else if (_data.Type == "stepper"s) {
			auto stepperOption = std::make_shared<StepperControl>();
			stepperOption->Options = _data.ValueOptions.Options;
			stepperOption->ValueSource = _data.ValueOptions.ValueSource;
			control = stepperOption;
		}
		else if (_data.Type == "menu"s) {
			auto menuOption = std::make_shared<MenuControl>();
			menuOption->ModName = _modName;
			menuOption->SourceForm = _data.ValueOptions.SourceForm;
			menuOption->ScriptName = _data.ValueOptions.ScriptName;
			menuOption->PropertyName = _data.ValueOptions.PropertyName;
			menuOption->Options = _data.ValueOptions.Options;
			menuOption->ShortNames = _data.ValueOptions.ShortNames;
			control = menuOption;
		}
		else if (_data.Type == "enum"s) {
			auto enumOption = std::make_shared<EnumControl>();
			enumOption->Options = _data.ValueOptions.Options;
			enumOption->ShortNames = _data.ValueOptions.ShortNames;
			enumOption->ValueSource = _data.ValueOptions.ValueSource;
			control = enumOption;
		}
		else if (_data.Type == "color"s) {
			auto colorOption = std::make_shared<ColorControl>();
			colorOption->ValueSource = _data.ValueOptions.ValueSource;
			control = colorOption;
		}
		else if (_data.Type == "keymap"s) {
			auto keymapOption = std::make_shared<KeyMapControl>();
			keymapOption->IgnoreConflicts = _data.IgnoreConflicts;
			keymapOption->ValueSource = _data.ValueOptions.ValueSource;
			control = keymapOption;
		}
		else if (_data.Type == "input"s) {
			auto inputOption = std::make_shared<InputControl>();
			inputOption->ModName = _modName;
			inputOption->SourceForm = _data.ValueOptions.SourceForm;
			inputOption->ScriptName = _data.ValueOptions.ScriptName;
			inputOption->PropertyName = _data.ValueOptions.PropertyName;
			control = inputOption;
		}

		if (control) {
			control->Position = _data.Position;
			control->ID = _data.ID;
			control->Text = _data.Text;
			control->Help = _data.Help;
			control->GroupCondition = _data.GroupCondition;
			control->GroupBehavior = _data.GroupBehavior;
			control->Action = _data.Action;
			_pageLayout->Controls.push_back(control);
		}
		else {
			return false;
		}

		_data = ControlData{};
		_state = State::Start;
		return true;
	}
	case State::GroupCondition:
	{
		bool groupConditionOK = _groupCondition->EndObject(memberCount);
		if (_groupCondition->Complete())
			_state = State::Control;
		return groupConditionOK;
	}
	case State::ValueOptions:
	{
		bool valueOptionsOK = _valueOptions->EndObject(memberCount);
		if (_valueOptions->Complete())
			_state = State::Control;
		return valueOptionsOK;
	}
	case State::Action:
	{
		bool actionOK = _action->EndObject(memberCount);
		if (_action->Complete())
			_state = State::Control;
		return actionOK;
	}
	default:
		return false;
	}
}

bool ContentHandler::StartArray()
{
	switch (_state) {
	case State::End:
		_state = State::Start;
		return true;
	case State::ValueOptions:
		return _valueOptions->StartArray();
	case State::Action:
		return _action->StartArray();
	default:
		return false;
	}
}

bool ContentHandler::EndArray(SizeType elementCount)
{
	switch (_state) {
	case State::Start:
		_state = State::End;
		return true;
	case State::ValueOptions:
		return _valueOptions->EndArray(elementCount);
	case State::Action:
		return _action->EndArray(elementCount);
	default:
		return false;
	}
}
