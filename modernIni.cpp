#include "modernIni.h"

#include <algorithm>
#include <filesystem>
#include <ranges>

void trim(std::string_view& sv) {
	auto rm_first = sv.find_first_not_of(' ');
	if (rm_first == std::string_view::npos) {
		sv = "";
		return;
	}
	sv.remove_prefix(rm_first);
	sv.remove_suffix(sv.length() - (sv.find_last_not_of(' ') + 1));
}

std::string escape(std::string_view str) {
	std::string res;
	res.reserve(str.size());

	for (const char& c : str) {
		switch (c) {
			case '\\':
				res += "\\\\";
				break;
			case '\n':
				res += "\\n";
				break;
			case ';':
				res += "\\;";
				break;
			default:
				res += c;
				break;
		}
	}
	return res;
}

// This returns the character that starts a comment
size_t unescape(std::string& str) {
	size_t comment_start = std::string::npos;
	size_t write = 0;
	for (size_t i = 0; i < str.size(); ++i, ++write) {
		if (str[i] == '\\') {
			++i;
			switch (str[i]) {
				case 'n':
					str[write] = '\n';
					break;
				case '\\':
					str[write] = '\\';
					break;
				case ';':
					str[write] = ';';
					break;
				default:
					// Do not save any character
					break;
			}
		} else {
			str[write] = str[i];
			if (str[write] == ';') {
				comment_start = write;
			}
		}
	}
	str.resize(write);

	return comment_start;
}

modernIni::Ini::Ini(const Ini& other) : type(other.type),
										key(other.key),
										data(other.data),
										parent(other.parent) {
	// adjust parent of all children
	if (type == Type::Object) {
		auto& map = std::get<ObjectMap>(data);
		for (auto& val : map | std::views::values) {
			val.parent = this;
		}
	}
}

modernIni::Ini& modernIni::Ini::operator=(const Ini& other) {
	if (this == &other)
		return *this;
	type = other.type;
	key = other.key;
	data = other.data;
	parent = other.parent;

	// adjust parent of all children
	if (type == Type::Object) {
		auto& map = std::get<ObjectMap>(data);
		for (auto& val : map | std::views::values) {
			val.parent = this;
		}
	}
	return *this;
}

modernIni::Ini::Ini(Ini&& other) noexcept : type(other.type),
											key(std::move(other.key)),
											data(std::move(other.data)),
											parent(other.parent) {
	// adjust parent of all children
	if (type == Type::Object) {
		auto& map = std::get<ObjectMap>(data);
		for (auto& val : map | std::views::values) {
			val.parent = this;
		}
	}
}

modernIni::Ini& modernIni::Ini::operator=(Ini&& other) noexcept {
	if (this == &other)
		return *this;
	type = other.type;
	key = std::move(other.key);
	data = std::move(other.data);
	parent = other.parent;

	// adjust parent of all children
	if (type == Type::Object) {
		auto& map = std::get<ObjectMap>(data);
		for (auto& val : map | std::views::values) {
			val.parent = this;
		}
	}
	return *this;
}

bool modernIni::Ini::hasValueElements() const {
	auto children = std::get_if<ObjectMap>(&data);
	if (children == nullptr) {
		return false;
	}
	return std::ranges::any_of(*children | std::views::values, std::bind_front(std::equal_to{}, Type::Value), &Ini::type);
}

void modernIni::Ini::getCategories(std::vector<std::string>& categories) const {
	// We stop propagating if the key is empty
	if (key.empty()) {
		return;
	}

	if (parent != nullptr) {
		parent->getCategories(categories);
	}
	categories.push_back(key);
}

std::string modernIni::Ini::getCategories() const {
	std::vector<std::string> categories;
	getCategories(categories);
	return categories
		   | std::views::transform([](const std::string& cat) { return std::format("[{}]", cat); })
		   | std::views::join
		   | std::ranges::to<std::string>();
}

std::istream& modernIni::operator>>(std::istream& input, Ini& ini) {
	Ini* lastCategory = &ini;

	// global element always object
	ini.type = Type::Object;

	for (std::string line; std::getline(input, line);) {
		auto comment_start = unescape(line);

		std::string_view line_view = line;

		line_view = line_view.substr(0, comment_start);
		trim(line_view);

		if (line_view.empty()) {
			continue;
		}

		// If trimmed line starts with `[` it is a category
		if (line_view.front() == '[') {
			// reset, so we are always the top most element and a new subcategory list can start
			lastCategory = &ini;

			size_t nextStart = 0;

			do {
				// nextStart always points at a '[', so we have to increment it.
				line_view.remove_prefix(nextStart + 1);

				auto end = line_view.find_first_of(']');
				if (end == std::string_view::npos) {
					// This should be noexcept, therefore just log and exit here
					// throw std::runtime_error("Invalid category name");
					return input;
				}
				auto catName = line_view.substr(0, end);
				trim(catName);

				auto data = std::get_if<Ini::ObjectMap>(&lastCategory->data);
				if (!data) {
					// This should be noexcept, therefore just log and exit here
					// throw std::runtime_error("Invalid lastCategory");
					return input;
				}
				auto catNameStr = std::string(catName);
				lastCategory = &data->try_emplace(catNameStr, Type::Object, std::move(catNameStr), lastCategory).first->second;

				line_view.remove_prefix(end + 1);

				nextStart = line_view.find_first_of('[');
			} while (nextStart != std::string_view::npos);
		}
		// else this is a key-value pair
		else {
			auto key = line_view.substr(0, line_view.find_first_of('='));
			auto value = line_view.substr(line_view.find_first_of('=') + 1);

			trim(key);
			trim(value);

			auto trimmedKey = std::string(key);
			auto trimmedValue = std::string(value);

			auto data = std::get_if<Ini::ObjectMap>(&lastCategory->data);
			if (!data) {
				// This should be noexcept, therefore just log and exit here
				// throw std::runtime_error("Invalid lastCategory");
				return input;
			}
			data->try_emplace(trimmedKey, trimmedKey, trimmedValue, lastCategory);
		}
	}

	return input;
}
std::ostream& modernIni::operator<<(std::ostream& output, const Ini& ini) {
	switch (ini.type) {
		case Type::Object: {
			auto children = std::get_if<Ini::ObjectMap>(&ini.data);
			if (!children) {
				return output;
			}

			// write out this categories key-value pairs
			for (const auto& element : *children
											   | std::views::values
											   | std::views::filter([](const Ini& element) { return element.type == Type::Value; })) {
				output << element;
			}

			// write out these categories child categories
			for (const auto& element : *children
											   | std::views::values
											   | std::views::filter([](const Ini& element) { return element.type == Type::Object; })) {
				if (element.hasValueElements()) {
					output << '\n'
						   << escape(element.getCategories()) << '\n';
				}
				output << element;
			}

			break;
		}
		case Type::Value: {
			auto data = std::get_if<std::string>(&ini.data);
			if (!data) {
				return output;
			}

			output << escape(ini.key) << " = " << escape(*data) << '\n';
			break;
		}
	}

	return output;
}
