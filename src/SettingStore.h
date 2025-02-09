#pragma once

class SettingStore final
{
	using Storage = RE::BSTHashMap<RE::BSFixedString, RE::Setting*>;

public:
	~SettingStore();
	SettingStore(const SettingStore&) = delete;
	SettingStore(SettingStore&&) = delete;
	SettingStore& operator=(const SettingStore&) = delete;
	SettingStore& operator=(SettingStore&&) = delete;

	static SettingStore& GetInstance();

	void ReadSettings();

	std::int32_t GetModSettingInt(std::string_view a_modName, std::string_view a_settingName);

	bool GetModSettingBool(std::string_view a_modName, std::string_view a_settingName);

	float GetModSettingFloat(std::string_view a_modName, std::string_view a_settingName);

	const char* GetModSettingString(std::string_view a_modName, std::string_view a_settingName);

	void SetModSettingInt(
		std::string_view a_modName,
		std::string_view a_settingName,
		std::int32_t a_newValue);

	void SetModSettingBool(
		std::string_view a_modName,
		std::string_view a_settingName,
		bool a_newValue);

	void SetModSettingFloat(
		std::string_view a_modName,
		std::string_view a_settingName,
		float a_newValue);

	void SetModSettingString(
		std::string_view a_modName,
		std::string_view a_settingName,
		std::string_view a_newValue);

	void ReloadDefault(std::string_view a_modName, std::string_view a_settingName);

	void ResetToDefault(std::string_view a_modName, std::string_view a_settingName);

	[[nodiscard]] RE::Setting* GetModSetting(
		std::string_view a_modName,
		std::string_view a_settingName);

	[[nodiscard]] RE::Setting* GetDefaultSetting(
		std::string_view a_modName,
		std::string_view a_settingName);

private:
	SettingStore();

	bool ReadINI(
		std::string_view a_modName,
		const std::filesystem::path& a_iniLocation,
		bool a_isDefault);

	void LoadDefaults();

	void LoadUserSettings();

	void RegisterModSetting(
		std::string_view a_modName,
		std::string_view a_settingName,
		std::string_view a_settingValue,
		Storage& a_settingStore);

	void CommitModSetting(std::string_view a_modName, RE::Setting* a_modSetting);

	[[nodiscard]] RE::BSFixedString GetKey(
		std::string_view a_modName,
		std::string_view a_settingName);

	Storage _defaults;
	Storage _settingStore;
	std::mutex _mutex;
	std::filesystem::path configPath{ "Data/MCM/Config"sv };
};
