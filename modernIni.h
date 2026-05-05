#pragma once

#include <algorithm>
#include <expected>
#include <iostream>
#include <map>
#include <ranges>
#include <string>

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

	namespace detail {
		std::string escape(std::string_view str);

		// This returns the character that starts a comment
		size_t unescape(std::string& str);
	} // namespace detail

	class Ini {
	private:
		template<typename Self, typename T>
		using ReferenceResult = Result<std::reference_wrapper<std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, const T, T>>>;

	public:
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

		[[nodiscard]] Result<void> erase(const std::string& key);
		[[nodiscard]] Result<bool> contains(const std::string& key) const noexcept;
		Ini& operator[](const std::string& key) noexcept;

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

	}

	// TO_INI for STL
	// this serializes all int types, bool and floating points
	void to_ini(Ini& ini, std::string_view value);
	template<typename N>
	requires std::is_integral_v<N> or std::is_floating_point_v<N>
	void to_ini(Ini& ini, N value) {
		ini = std::format("{}", value);
	}
} // namespace modernIni
