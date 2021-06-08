module;

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <regex>
#include <ranges>
#include <charconv>
#include <type_traits>
#include <format>

export module modernIni;

export namespace modernIni {
	class Ini;

	template<typename T>
	concept HasFromIni =
		requires(T& val, const Ini& ini) {
		from_ini(val, ini);
	};

	template<typename T>
	concept HasToIni =
		requires(const T& val, Ini& ini) {
		to_ini(val, ini);
	};

	template<typename T>
	concept IsFromChars =
		requires(char* s, T val) {
		std::from_chars(s, s, val);
	};

	enum class Type {
		Object,
		Value
	};

	class Ini {
		friend std::istream& operator>>(std::istream& input, Ini& ini);
		friend std::ostream& operator<<(std::ostream& output, const Ini& ini);

	private:
		Type type = Type::Value;
		std::string value;
		std::string key;
		std::map<std::string, Ini> subElements;
		Ini* parent = nullptr;

	public:
		Ini() {}

		Ini(std::string new_val) :
			value(new_val), type(Type::Value) { }

		Ini(std::string new_key, std::string new_val) : 
			key(new_key), value(new_val), type(Type::Value) { }

		Ini(std::string new_key, std::string new_val, Ini* new_parent) :
			key(new_key), value(new_val), type(Type::Value), parent(new_parent) { }

		Ini(std::map<std::string, Ini> new_sub_elements) :
			subElements(new_sub_elements) {
			type = Type::Object;
		}

		Ini(std::string new_key, std::map<std::string, Ini> new_sub_elements) :
			key(new_key), subElements(new_sub_elements) {
			type = Type::Object;
		}

		Ini(std::string new_key, std::map<std::string, Ini> new_sub_elements, Ini* new_parent) :
			key(new_key), subElements(new_sub_elements), parent(new_parent) {
			type = Type::Object;
		}

		template<HasToIni T>
		Ini(const T& val) {
			this->operator=(val);
		}
		template<typename T>
		requires std::is_integral_v<T> || std::is_floating_point_v<T>
		Ini(const T& val) {
			this->operator=(val);
		}

		bool hasValueElements() const {
			if (type == Type::Value)
				return false;

			for (const Ini& element : subElements | std::views::values) {
				if (element.type == Type::Value) {
					return true;
				}
			}

			return false;
		}

		inline bool isObject() const {
			return type == Type::Object;
		}

		inline bool isValue() const {
			return type == Type::Value;
		}

		bool has(const std::string& key) const {
			if (!isObject()) {
				return false;
			}
			return subElements.contains(key);
		}

		/**
		 * This produces a string in the ini file category format. e.g. `[cat][subcat][subsubcat]`
		 */
		std::string getCategories() const {
			std::stringstream ss;
			if (parent != nullptr) {
				ss << parent->getCategories();
			}
			if (!key.empty()) {
				ss << "[" << key << "]";
			}

			return ss.str();
		}

		template<HasFromIni T>
		void get_to(T& val) const {
			if (!isObject()) return;
			from_ini(val, *this);
		}

		template<IsFromChars T>
		void get_to(T& val) const {
			if (!isValue()) return;
			std::from_chars(value.data(), value.data() + value.size(), val);
		}

		void get_to(std::string& val) const {
			if (!isValue()) return;
			val = value;
		}

		void get_to(bool& val) const {
			if (!isValue()) return;
			std::string lowerVal = value;
			std::transform(lowerVal.begin(), lowerVal.end(), lowerVal.begin(), [](auto& c) {
				return std::tolower(c);
			});
			if (lowerVal == "true" || lowerVal == "on" || lowerVal == "1") {
				val = true;
			} else if (lowerVal == "false" || lowerVal == "off" || lowerVal == "0") {
				val = false;
			}
		}

		template<typename T>
		T get() const {
			T val = {};
			get_to(val);
			return std::move(val);
		}

		Ini& at(const std::string& key) {
			if (!isObject()) {
				throw std::out_of_range("Calle `at()` on non-object");
			}

			return subElements.at(key);
		}
		const Ini& at(const std::string& key) const {
			if (!isObject()) {
				throw std::out_of_range("Called `at()` on non-object");
			}

			return subElements.at(key);
		}

		Ini& operator[](const std::string& key) {
			type = Type::Object;
			Ini& element = subElements[key];
			element.parent = this;
			element.key = key;
			return element;
		}

		template<HasToIni T>
		void operator=(const T& val) {
			type = Type::Object;
			to_ini(val, *this);
		}

		template<typename T>
		requires std::is_integral_v<T> || std::is_floating_point_v<T>
		void operator=(const T& val) {
			type = Type::Value;
			value = std::format("{}", val);
		}

		void operator=(const std::string& val) {
			type = Type::Value;
			value = val;
		}

		bool operator==(const Ini& other) const {
			if (type != other.type || key != other.key) {
				return false;
			}

			switch (type)
			{
			case Type::Object:
				return subElements == other.subElements;
			case Type::Value:
				return value == other.value;
			default:
				break;
			}

			return false;
		}
	};

	// deserialize from stream
	std::istream& operator>>(std::istream& input, Ini& ini) {
		Ini* lastCategory = &ini;
		
		// global element always object
		ini.type = Type::Object;

		while (!input.eof()) {
			std::string line;
			std::getline(input, line);

			if (line.empty()) {
				//continue;
				continue;
			}
			else if (line.starts_with('[')) {
				lastCategory = &ini;
				// this is a category
				std::regex reg("\\[([^\\[\\]]+?)\\]");
				std::sregex_iterator next(line.begin(), line.end(), reg);
				std::sregex_iterator end;
				while (next != end) {
					std::smatch match = *next;
					lastCategory = &lastCategory->operator[](match[1]);
					lastCategory->type = Type::Object;
					next++;
				}
			}
			else {
				// key=value pair or empty
				size_t splitPos = line.find('=');
				std::string key = line.substr(0, splitPos);
				std::string value = line.substr(splitPos + 1);
				lastCategory->subElements.try_emplace(key, key, value, lastCategory);
			}
		}

		return input;
	}

	// serialize to stream
	std::ostream& operator<<(std::ostream& output, const Ini& ini) {
		switch (ini.type)
		{
		case Type::Object:
			// write out this categories key-value pairs
			for (const auto& element : ini.subElements | std::views::values) {
				if (element.type == Type::Value) {
					output << element;
				}
			}

			// write out this categories subcategories
			for (const auto& element : ini.subElements | std::views::values) {
				// check if object has any Value types elements
				if (element.type == Type::Object) {
					if (element.hasValueElements()) {
						output << std::endl << element.getCategories() << std::endl;
					}
					output << element;
				}
			}
			break;
		case Type::Value:
			output << ini.key << "=" << ini.value << std::endl;
			break;
		default:
			break;
		}
		return output;
	}
};