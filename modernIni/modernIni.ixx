module;

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <regex>
#include <ranges>

export module modernIni;

export namespace modernIni {
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

		Ini& operator[](const std::string& l) {
			type = Type::Object;
			Ini& element = subElements[l];
			element.parent = this;
			element.key = l;
			return element;
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