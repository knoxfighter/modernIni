#include "modernIni.h"

bool modernIni::Ini::hasValueElements() const
{
	if (type == Type::Value)
		return false;

	for (const Ini& element : subElements | std::views::values) {
		if (element.type == Type::Value) {
			return true;
		}
	}

	return false;
}

bool modernIni::Ini::isObject() const
{
	return type == Type::Object;
}

bool modernIni::Ini::isValue() const
{
	return type == Type::Value;
}

bool modernIni::Ini::has(const std::string& key) const
{
	if (!isObject()) {
		return false;
	}
	return subElements.contains(key);
}

std::string modernIni::Ini::getCategories() const
{
	std::stringstream ss;
	if (parent != nullptr) {
		ss << parent->getCategories();
	}
	if (!key.empty()) {
		ss << "[" << key << "]";
	}

	return ss.str();
}

void modernIni::Ini::get_to(std::string& val) const
{
	if (!isValue()) return;
	val = value;
}

void modernIni::Ini::get_to(bool& val) const
{
	if (!isValue()) return;
	std::string lowerVal = value;
	std::ranges::transform(lowerVal, lowerVal.begin(), [](auto& c) {
		return std::tolower(c);
	});
	if (lowerVal == "true" || lowerVal == "on" || lowerVal == "1") {
		val = true;
	} else if (lowerVal == "false" || lowerVal == "off" || lowerVal == "0") {
		val = false;
	}
}

void modernIni::Ini::erase(const std::string& key)
{
	if (!isObject()) {
		throw std::out_of_range("Called `erase()` on non-object");
	}

	subElements.erase(key);
}

modernIni::Ini& modernIni::Ini::at(const std::string& key)
{
	if (!isObject()) {
		throw std::out_of_range("Called `at()` on non-object");
	}

	return subElements.at(key);
}

const modernIni::Ini& modernIni::Ini::at(const std::string& key) const
{
	if (!isObject()) {
		throw std::out_of_range("Called `at()` on non-object");
	}

	return subElements.at(key);
}

modernIni::Ini& modernIni::Ini::operator[](const std::string& key)
{
	type = Type::Object;
	Ini& element = subElements[key];
	element.parent = this;
	element.key = key;
	return element;
}

void modernIni::Ini::operator=(const std::string& val)
{
	type = Type::Value;
	value = val;
}

bool modernIni::Ini::operator==(const Ini& other) const
{
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

std::istream& modernIni::operator>>(std::istream& input, Ini& ini)
{
	Ini* lastCategory = &ini;
		
	// global element always object
	ini.type = Type::Object;

	while (!input.eof()) {
		std::string line;

		if (!std::getline(input, line) || line.empty()) {
			//continue;
			continue;
		}
		else if (line.find('=') != std::string::npos) {
			// key=value pair or empty
			size_t splitPos = line.find('=');
			std::string key = line.substr(0, splitPos);
			std::string value = line.substr(splitPos + 1);
			// strip spaces (leading/trailing)
			key = std::regex_replace(key, std::regex("^ +| +$|( ) +"), "$1");
			value = std::regex_replace(value, std::regex("^ +| +$|( ) +"), "$1");

			std::stringstream ss;
			bool decodeNext = false;
			for (auto letter : value) {
				if (decodeNext) {
					decodeNext = false;
					if (letter == '\\') {
						ss << letter;
					} else if (letter == 'n') {
						ss << '\n';
					}
				} else if (letter == '\\') {
					decodeNext = true;
				} else {
					ss << letter;
				}
			}
			value = ss.str();

			lastCategory->subElements.try_emplace(key, key, value, lastCategory);
		}
		else if (line.find('[') != std::string::npos) {
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
	}

	return input;
}

std::ostream& modernIni::operator<<(std::ostream& output, const Ini& ini)
{
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
	case Type::Value: {
		std::string value = ini.value;
		std::stringstream ss;
		for (auto letter : value) {
			if (letter == '\n') {
				ss << '\\' << 'n';
			} else if (letter == '\\') {
				ss << '\\' << '\\';
			} else {
				ss << letter;
			}
		}
		output << ini.key << "=" << ss.str() << std::endl;
		break;
	}
	default:
		break;
	}
	return output;
}
