#pragma once

#include <algorithm>
#include <expected>
#include <iostream>
#include <map>
#include <ranges>
#include <string>

#if __has_include(<magic_enum/magic_enum.hpp>)
#include <magic_enum/magic_enum.hpp>
#endif

#ifdef TESTS
#include <gtest/gtest.h>

class ModernIniTestAccessor;
#endif


namespace modernIni {
	enum class Type {
		Object,
		Value,
	};

	enum class Error {
		WrongType,
		KeyNotFound,
		InvalidValue,
	};
	template<typename T>
	using Result = std::expected<T, Error>;

	namespace customize {
		enum class IniEnumType {
			Default,
			Underlying,
			MagicEnum,
		};
		template<typename E>
		struct IniEnumTypeOverride {
			static constexpr auto type = IniEnumType::Default;
		};
	}

	namespace detail {
		std::string escape(std::string_view str);

		// This returns the character that starts a comment
		size_t unescape(std::string& str);

#if __has_include(<magic_enum/magic_enum.hpp>)
		constexpr bool MagicEnumAvailable = true;
#else
		constexpr bool MagicEnumAvailable = false;
#endif
		template<typename E>
		consteval customize::IniEnumType getEnumType() {
			auto t = customize::IniEnumTypeOverride<E>::type;
			// resolve Default
			if (t == customize::IniEnumType::Default) {
				t = MagicEnumAvailable ? customize::IniEnumType::MagicEnum : customize::IniEnumType::Underlying;
			}

			switch (t) {
				case customize::IniEnumType::Underlying:
					return customize::IniEnumType::Underlying;
				case customize::IniEnumType::MagicEnum:
					static_assert(MagicEnumAvailable, "MagicEnum is not available!");
					return customize::IniEnumType::MagicEnum;
				default:
					return customize::IniEnumType::Default;
			}
		}

		template<typename T>
		concept PairLike =
			requires { std::tuple_size<std::remove_cvref_t<T>>::value; } // This cannot be tuple_size_v, it causes a hard error
			&& std::tuple_size_v<std::remove_reference_t<T>> == 2;

		template<typename T>
		concept KeyValueRange =
			std::ranges::range<T>
			&& PairLike<std::ranges::range_value_t<T>>;

		template<typename T>
		concept StringLike =
			std::is_constructible_v<std::string, T>;

		template<typename T>
		concept Sequence =
			std::ranges::range<std::remove_cvref_t<T>>
			&& !StringLike<std::remove_cvref_t<T>>
			&& !KeyValueRange<std::remove_cvref_t<T>>;

		template<typename C, typename T>
		concept HasPushBack = requires(C c, T t) {
				c.push_back(t);
		};
		template<typename C, typename T>
		concept HasPushFront = requires(C c, T t) {
			c.push_front(t);
		} && !HasPushBack<C, T>;
		template<typename C, typename T>
		concept HasInsert = requires(C c, T t) {
			c.insert(t);
		};
		template<typename C, typename T>
		concept HasPush = requires(C c, T t) {
			c.push(t);
		};
		template<typename T>
		concept MapLike = requires {
			typename T::key_type;
			typename T::mapped_type;
		};
		template<typename T>
		concept SetLike = requires {
			typename T::value_type;
		} && !MapLike<T>;

		template<typename C, typename T>
		requires HasPushBack<C, T>
		void RunInsert(C& c, T&& t) {
			c.push_back(std::forward<T>(t));
		}
		template<typename C, typename T>
		requires HasPushFront<C, T>
		void RunInsert(C& c, T&& t) {
			c.push_front(std::forward<T>(t));
		}
		template<typename C, typename T>
		requires HasInsert<C, T>
		void RunInsert(C& c, T&& t) {
			c.insert(std::forward<T>(t));
		}
		template<typename C, typename T>
		requires HasPush<C, T>
		void RunInsert(C& c, T&& t) {
			c.push(std::forward<T>(t));
		}
	}

	class Ini {
	public:
		template<typename Self, typename T>
		using ReferenceResult = Result<std::reference_wrapper<std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, const T, T>>>;

		using ObjectStorage = std::map<std::string, Ini>;
		using ValueStorage = std::string;
		using Storage = std::variant<ObjectStorage, ValueStorage>;

		Ini() : data(ObjectStorage()) {}

		Ini(std::string new_key, std::string new_val, Ini* new_parent) : type(Type::Value), key(std::move(new_key)), data(std::move(new_val)), parent(new_parent) {}

		Ini(std::string new_key, Ini* new_parent) : type(Type::Object), key(std::move(new_key)), parent(new_parent) {}

		// copy construct
		Ini(const Ini& other) = delete;
		Ini& operator=(const Ini& other) = delete;

		// move construct
		Ini(Ini&& other) noexcept;
		Ini& operator=(Ini&& other) noexcept;

		// destruct
		~Ini() = default;

		/**
		 * Note: When getting a negative unsigned integer, the value will wrap.
		 */
		template<typename C>
		requires std::is_default_constructible_v<C> && std::is_move_constructible_v<C>
		[[nodiscard]] Result<C> get() const noexcept {
			C val = {};
			auto res = from_ini(*this, val);
			if (!res) {
				return std::unexpected(res.error());
			}
			return std::move(val);
		}

		/**
		 * Note: When getting a negative unsigned integer, the value will wrap.
		 */
		template<typename C>
		[[nodiscard]] Result<void> get_to(C&& val) const noexcept {
			return from_ini(*this, std::forward<C>(val));
		}

		template<typename C>
		Ini& operator=(C&& val) {
			to_ini(*this, std::forward<C>(val));

			return *this;
		}

		/**
		 * This sets the type of the Ini instance.
		 * Will reset data to empty.
		 * Does nothing if type is already t.
		 * @param t the new type that is set
		 * @return if setting the type was successful
		 */
		void setType(Type t) noexcept;

		[[nodiscard]] Type getType() const { return type; }

		/**
		 * Check if this object has value children.
		 * Will return Error if of the wrong type.
		 */
		[[nodiscard]] Result<bool> hasValueChildren() const;

		/**
		 * Get the parent path of this object.
		 * Mostly used on Objects and will contain the current key when used on Values.
		 * @param categories out param with the path
		 */
		void getParentPath(std::vector<std::string>& categories) const;

		/**
		 * Get the parent path is string in the form `[p1][p2]...`
		 */
		[[nodiscard]] std::string getParentPath() const;

		/* Object Map Wrapper */

		template<typename Self>
		[[nodiscard]] auto getChildren(this Self&& self) noexcept -> ReferenceResult<Self, ObjectStorage> {
			if (self.type != Type::Object) {
				return std::unexpected(Error::WrongType);
			}
			return std::get<ObjectStorage>(self.data);
		}

		auto begin() noexcept {
			switch (type) {
				case Type::Object:
					return std::get<ObjectStorage>(data).begin();
				default:
					return std::map<std::string, Ini>::iterator();
			}
		}
		auto end() noexcept {
			switch (type) {
				case Type::Object:
					return std::get<ObjectStorage>(data).end();
				default:
					return std::map<std::string, Ini>::iterator();
			}
		}
		auto begin() const noexcept {
			switch (type) {
				case Type::Object:
					return std::get<ObjectStorage>(data).begin();
				default:
					return std::map<std::string, Ini>::const_iterator();
			}
		}
		auto end() const noexcept {
			switch (type) {
				case Type::Object:
					return std::get<ObjectStorage>(data).end();
				default:
					return std::map<std::string, Ini>::const_iterator();
			}
		}

		/**
		 * Get the reference to a child of this Object, will fail if it is not an Object or if key does not exist.
		 */
		template<typename Self>
		[[nodiscard]] auto at(this Self&& self, const std::string& key) noexcept -> ReferenceResult<Self, Ini> {
			if (self.type != Type::Object) {
				return std::unexpected(Error::WrongType);
			}
			auto& object = std::get<ObjectStorage>(self.data);
			if (object.contains(key)) {
				return object.at(key);
			}
			return std::unexpected(Error::KeyNotFound);
		}

		/**
		 * Get the reference to a child of this Object, will fail if it is not an Object or if key does not exist.
		 *
		 * SAFETY: Even though value is called, the exception should never be thrown with enums and arithmetics (numbers)
		 */
		template<typename Self, typename T>
		requires std::is_enum_v<T> or std::is_arithmetic_v<T>
		[[nodiscard]] auto at(this Self&& self, T key) noexcept -> ReferenceResult<Self, Ini> {
			Ini ini;
			to_ini(ini, key);
			return self.at(ini.get<std::string_view>().value());
		}

		[[nodiscard]] Result<void> erase(const std::string& key);
		[[nodiscard]] Result<bool> contains(const std::string& key) const noexcept;
		Ini& operator[](std::string_view key) noexcept;

		/**
		 * Use the to_ini implementation to convert the key to a string and access the Ini object.
		 *
		 * SAFETY: Even though value is called, the exception should never be thrown with enums and arithmetics (numbers)
		 **/
		template<typename T>
		requires std::is_enum_v<T> or std::is_arithmetic_v<T>
		Ini& operator[](T key) noexcept {
			Ini ini;
			to_ini(ini, key);
			return (*this)[ini.get<std::string_view>().value()];
		}

	private:
		Type type = Type::Object;
		std::string key;
		// string for Type::Value
		// map for Type::Object
		// This is a map cause elements should be saved sorted by name.
		// We could change it to unordered_map.
		Storage data;
		Ini* parent = nullptr;

		// friends
		friend std::istream& operator>>(std::istream& input, Ini& ini);
		friend std::ostream& operator<<(std::ostream& output, const Ini& ini);
		friend Result<void> from_ini(const Ini& ini, std::string& value);
		friend Result<void> from_ini(const Ini& ini, std::string_view& value);
		friend void to_ini(Ini& ini, std::string_view value);

		friend class ::ModernIniTestAccessor;
	};

	/**
	 * Deserialize Ini from the input stream.
	 * If a key is defined multiple times under the same section, only the *first* value will be kept.
	 *
	 * @param input Input stream to read from
	 * @param ini Ini instance to deserialize into
	 * @return Reference to the input stream
	 */
	std::istream& operator>>(std::istream& input, Ini& ini);

	/**
	 * Serialize Ini to the output stream.
	 *
	 * @param output Output stream to write to
	 * @param ini Ini instance to serialize
	 * @return Reference to the output stream
	 */
	std::ostream& operator<<(std::ostream& output, const Ini& ini);

	// FROM_INI for STL
	Result<void> from_ini(const Ini& ini, std::string& value);

	// Lifetime is bound to the lifetime of this Ini instance.
	Result<void> from_ini(const Ini& ini, std::string_view& value);

	Result<void> from_ini(const Ini& ini, bool& value);

	Result<void> from_ini(const Ini& ini, char& value);

	/**
	 * Integer parsers: Expects the pattern identical to the one used by std::strtol in the default ("C") locale and the given non-zero numeric base, except that
	 */
	template<typename N>
	requires std::is_integral_v<N>
	[[nodiscard]] Result<void> from_ini(const Ini& ini, N& value) {
		// Type is checked in the get of std::string!
		auto str = ini.get<std::string_view>();
		if (!str) {
			return std::unexpected(str.error());
		}
		std::string_view view = str.value();

		// parse base
		int base = 10;
		bool negative = false;
		if (view.starts_with("-")) {
			view.remove_prefix(1);
			negative = true;
			if constexpr (std::is_unsigned_v<N>) {
				return std::unexpected(Error::InvalidValue);
			}
		}
		if (view.starts_with("0x") || view.starts_with("0X")) {
			base = 16;
			view.remove_prefix(2);
		} else if (view.starts_with("0o") || view.starts_with("0O")) {
			base = 8;
			view.remove_prefix(2);
		} else if (view.starts_with("0b") || view.starts_with("0B")) {
			base = 2;
			view.remove_prefix(2);
		}

		auto res = std::from_chars(view.data(), view.data() + view.size(), value, base);
		value = negative ? -value : value;
		if (res.ec == std::errc{}) {
			return {};
		}
		return std::unexpected(Error::InvalidValue);
	}

	/**
	 * Parses a floating point value from an Ini object.
	 * This function uses std::from_chars to parse the floating point value.
	 * Special values inf, nan and -inf are supported.
	 */
	template<typename N>
	requires std::is_floating_point_v<N>
	[[nodiscard]] Result<void> from_ini(const Ini& ini, N& value) {
		auto str = ini.get<std::string_view>();
		if (!str) {
			return std::unexpected(str.error());
		}
		std::string_view view = str.value();
		auto res = std::from_chars(view.data(), view.data() + view.size(), value);
		if (res.ec == std::errc{}) {
			return {};
		}
		return std::unexpected(Error::InvalidValue);
	}

	template<typename E>
	requires std::is_enum_v<E>
	[[nodiscard]] Result<void> from_ini(const Ini& ini, E& value) {
		constexpr auto type = detail::getEnumType<E>();
		// try magic enum first and fall back to underlying
		// we do this to check if we can match using the string directly.
		// If that doesn't work, we can read the number and use the value.
		// An identifier cannot start with a number (or only be a number).
		// Therefore, it is not possible to accidentally mistake one for another (except users do weird shit with their ini file manually).
		if constexpr (detail::MagicEnumAvailable) {
			auto str = ini.get<std::string_view>();
			if (!str) {
				return std::unexpected(str.error());
			}
			auto cast = magic_enum::enum_cast<E>(str.value());
			if (cast) {
				value = *cast;
				return {};
			}
		}
		auto num = ini.get<std::underlying_type_t<E>>();
		if (!num) {
			return std::unexpected(num.error());
		}
		value = static_cast<E>(num.value());
		return {};
		// return from_ini(ini, static_cast<int&>(value));
	}

	template<typename T>
	[[nodiscard]] Result<void> from_ini(const Ini& ini, std::optional<T>& value) {
		auto str = ini.get<std::string_view>();
		if (!str) {
			return std::unexpected(str.error());
		}
		if (str.value().empty()) {
			value = std::nullopt;
			return {};
		}
		auto getT = ini.get<T>();
		if (!getT) {
			return std::unexpected(getT.error());
		}
		value = getT.value();
		return {};
	}

	template<detail::MapLike T>
	[[nodiscard]] Result<void> from_ini(const Ini& ini, T& value) {
		if (ini.getType() != Type::Object) {
			return std::unexpected(Error::WrongType);
		}
		for (auto&& [key, iniVal] : ini) {
			// convert key to K
			Ini keyIni("", key, nullptr);
			auto keyRes = keyIni.get<typename T::key_type>();
			if (!keyRes) {
				return std::unexpected(keyRes.error());
			}

			// convert value to V
			auto valueRes = iniVal.get<typename T::mapped_type>();
			if (!valueRes) {
				return std::unexpected(valueRes.error());
			}
			// insert takes a pair(key, val)
			detail::RunInsert(value, std::make_pair(std::move(keyRes.value()), std::move(valueRes.value())));
		}
		return {};
	}

	template<detail::SetLike T>
	[[nodiscard]] Result<void> from_ini(const Ini& ini, T& value) {
		if (ini.getType() != Type::Object) {
			return std::unexpected(Error::WrongType);
		}

		for (auto&& val : ini | std::views::values) {
			auto getValue = val.get<typename T::value_type>();
			if (!getValue) {
				return std::unexpected(getValue.error());
			}
			detail::RunInsert(value, std::move(getValue.value()));
		}
		return {};
	}

	template<typename T, std::size_t Size>
	[[nodiscard]] Result<void> from_ini(const Ini& ini, std::array<T, Size>& value) {
		if (ini.getType() != Type::Object) {
			return std::unexpected(Error::WrongType);
		}
		Result<void> res = {};
		for (auto&& [i, val] : ini | std::views::values | std::views::take(Size) | std::views::enumerate) {
			auto getValue = val.get<T>();
			if (!getValue && getValue.has_value()) {
				res = std::unexpected(getValue.error());
				continue;
			}
			value[i] = std::move(getValue.value());
		}
		return res;
	}

	// TO_INI for STL
	// this serializes all int types, bool and floating points
	void to_ini(Ini& ini, std::string_view value);

	template<typename N>
	requires std::is_integral_v<N> or std::is_floating_point_v<N>
	void to_ini(Ini& ini, N value) {
		ini = std::format("{}", value);
	}


	template<typename E>
	requires std::is_enum_v<E>
	void to_ini(Ini& ini, E value) {
		constexpr auto type = detail::getEnumType<E>();
		if constexpr (type == customize::IniEnumType::Underlying) {
			ini = std::to_underlying(value);
		} else if constexpr (type == customize::IniEnumType::MagicEnum) {
			ini = magic_enum::enum_name(value);
		}
	}

	template<typename T>
	void to_ini(Ini& ini, const std::optional<T>& value) {
		if (value) {
			ini = *value;
		} else {
			ini = "";
		}
	}

	template<detail::KeyValueRange R>
	void to_ini(Ini& ini, const R& value) {
		ini.setType(Type::Object);

		for (auto&& [key, val] : value) {
			ini[key] = val;
		}
	}
	template<detail::Sequence T>
	requires (!detail::StringLike<T>)
	void to_ini(Ini& ini, const T& arr) {
		to_ini(ini, arr | std::views::enumerate);
	}
} // namespace modernIni
