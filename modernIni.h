#pragma once

#include <iostream>
#include <map>
#include <ranges>
#include <string>

#ifdef TESTS
#include <gtest/gtest.h>

class ModernIniTestAccessor;
#endif


namespace modernIni {
	class Ini;

	namespace detail {

	}

	enum class Type {
		Object,
		Value,
	};

	class Ini {
	public:
		Ini() = default;

		Ini(std::string new_key, std::string new_val, Ini* new_parent) : type(Type::Value), key(std::move(new_key)), data(std::move(new_val)), parent(new_parent) {}

		Ini(Type new_type, std::string new_key, Ini* new_parent) : type(new_type), key(std::move(new_key)), parent(new_parent) {}

		// copy construct
		Ini(const Ini& other);
		Ini& operator=(const Ini& other);

		// move construct
		Ini(Ini&& other) noexcept;
		Ini& operator=(Ini&& other) noexcept;

		~Ini() = default;

		template<typename T>
		requires std::is_default_constructible_v<T> && std::is_move_constructible_v<T>
		T get() const {
			T val = {};
			from_ini(this, val);
			return std::move(val);
		}

		template<typename T>
		void get_to(T& val) const {
			from_ini(this, val);
		}

		template<typename T>
		Ini& operator=(const T& val) {
			to_ini(this, val);

			return *this;
		}

		[[nodiscard]] bool hasValueElements() const;
		void getCategories(std::vector<std::string>& categories) const;
		[[nodiscard]] std::string getCategories() const;

	private:
		using ObjectMap = std::map<std::string, Ini>;

		Type type = Type::Object;
		std::string key;
		// string for Type::Value
		// map for Type::Object
		// This is a map cause elements should be saved sorted by name.
		// We could change it to unordered_map.
		std::variant<std::string, ObjectMap> data = ObjectMap{};
		Ini* parent = nullptr;

		// friends
		friend std::istream& operator>>(std::istream& input, Ini& ini);
		friend std::ostream& operator<<(std::ostream& output, const Ini& ini);

		friend class ::ModernIniTestAccessor;
	};

	// deserialize from stream
	std::istream& operator>>(std::istream& input, Ini& ini);

	// serialize to stream
	std::ostream& operator<<(std::ostream& output, const Ini& ini);
} // namespace modernIni
