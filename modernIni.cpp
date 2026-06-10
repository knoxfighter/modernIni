#include "modernIni.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <ranges>

void trim(std::string_view& sv) {
	// hardcoded list of whitespace characters
	std::string_view chars = " \r\t";
	auto rm_first = sv.find_first_not_of(chars);
	if (rm_first == std::string_view::npos) {
		sv = "";
		return;
	}
	sv.remove_prefix(rm_first);
	sv.remove_suffix(sv.length() - (sv.find_last_not_of(chars) + 1));
}

std::string modernIni::detail::escape(std::string_view str) {
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

size_t modernIni::detail::unescape(std::string& str) {
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

modernIni::Ini::Ini(Ini&& other) noexcept {
	type = other.type;
	key = std::move(other.key);
	data = std::move(other.data);
	parent = other.parent;

	// update the parent object of all children
	if (type == Type::Object) {
		auto& map = std::get<ObjectStorage>(data);
		for (auto& val : map | std::views::values) {
			val.parent = this;
		}
	}
}

modernIni::Ini& modernIni::Ini::operator=(Ini&& other) noexcept {
	type = other.type;
	key = std::move(other.key);
	data = std::move(other.data);
	parent = other.parent;

	// update the parent object of all children
	if (type == Type::Object) {
		auto& map = std::get<ObjectStorage>(data);
		for (auto& val : map | std::views::values) {
			val.parent = this;
		}
	}
	return *this;
}

modernIni::Result<bool> modernIni::Ini::hasValueChildren() const {
	if (type != Type::Object) {
		return std::unexpected(Error::WrongType);
	}
	return std::ranges::any_of(std::get<ObjectStorage>(data) | std::views::values, std::bind_front(std::equal_to{}, Type::Value), &Ini::type);
}

void modernIni::Ini::getParentPath(std::vector<std::string>& categories) const {
	// We don't need a check for type, parents will always be of type Object

	// We stop propagating if the key is empty
	if (key.empty()) {
		return;
	}

	if (parent != nullptr) {
		parent->getParentPath(categories);
	}
	categories.push_back(key);
}

std::string modernIni::Ini::getParentPath() const {
	std::vector<std::string> categories;
	getParentPath(categories);
	return categories
		   | std::views::transform([](const std::string& cat) { return std::format("[{}]", cat); })
		   | std::views::join
		   | std::ranges::to<std::string>();
}

modernIni::Result<void> modernIni::Ini::erase(const std::string& key) {
	if (type != Type::Object) {
		return std::unexpected(Error::WrongType);
	}
	std::get<ObjectStorage>(data).erase(key);

	return {};
}
modernIni::Result<bool> modernIni::Ini::contains(const std::string& key) const noexcept {
	if (type != Type::Object) {
		return std::unexpected(Error::WrongType);
	}
	return std::get<ObjectStorage>(data).contains(key);
}

modernIni::Ini& modernIni::Ini::operator[](std::string_view key) noexcept {
	setType(Type::Object);

	Ini ini(std::string(key), this);
	auto it = std::get<ObjectStorage>(data).insert_or_assign(std::string(key), std::move(ini));
	return it.first->second;
}

void modernIni::Ini::clear() noexcept {
	if (type == Type::Object) {
		std::get<ObjectStorage>(data).clear();
	} else if (type == Type::Value) {
		std::get<ValueStorage>(data).clear();
	}
}

void modernIni::Ini::setType(Type t) noexcept {
	// do nothing if already of the correct type
	if (type == t) {
		return;
	}
	type = t;
	switch (t) {
		case Type::Object:
			data = ObjectStorage{};
			break;
		case Type::Value:
			data = ValueStorage{};
			break;
	}
}

std::istream& modernIni::operator>>(std::istream& input, Ini& ini) {
	Ini* lastCategory = &ini;

	for (std::string line; std::getline(input, line);) {
		auto commentStart = detail::unescape(line);

		std::string_view lineView = line;

		lineView = lineView.substr(0, commentStart);
		trim(lineView);

		if (lineView.empty()) {
			continue;
		}

		// If trimmed line starts with `[` it is a category
		if (lineView.front() == '[') {
			// reset, so we are always the top most element and a new subcategory list can start
			lastCategory = &ini;

			size_t nextStart = 0;

			do {
				// nextStart always points at a '[', so we have to increment it.
				lineView.remove_prefix(nextStart + 1);

				auto end = lineView.find_first_of(']');
				if (end == std::string_view::npos) {
					// We simulate a missing closing bracket as if it was there
					end = lineView.size();
				}
				auto catName = lineView.substr(0, end);
				trim(catName);

				auto catNameStr = std::string(catName);
				lastCategory = &std::get<Ini::ObjectStorage>(lastCategory->data).try_emplace(catNameStr, std::move(catNameStr), lastCategory).first.operator*().second;

				lineView.remove_prefix(end);

				nextStart = lineView.find_first_of('[');
			} while (nextStart != std::string_view::npos);
		}
		// else this is a key-value pair
		else {
			auto firstEqual = lineView.find_first_of('=');
			// if no = is found, there is no value only a key.
			auto key = firstEqual == std::string_view::npos ? lineView : lineView.substr(0, firstEqual);
			auto value = firstEqual == std::string_view::npos ? "" : lineView.substr(firstEqual + 1);

			trim(key);
			trim(value);

			auto trimmedKey = std::string(key);
			auto trimmedValue = std::string(value);

			std::get<Ini::ObjectStorage>(lastCategory->data).try_emplace(trimmedKey, std::move(trimmedKey), std::move(trimmedValue), lastCategory);
		}
	}

	return input;
}

std::ostream& modernIni::operator<<(std::ostream& output, const Ini& ini) {
	switch (ini.type) {
		case Type::Object: {
			auto& children = std::get<Ini::ObjectStorage>(ini.data);

			// write out this categories key-value pairs
			for (const auto& element : children
											   | std::views::values
											   | std::views::filter([](const auto& element) { return element.type == Type::Value; })) {
				output << element;
			}

			// write out these categories child categories
			for (const auto& element : children
											   | std::views::values
											   | std::views::filter([](const auto& element) { return element.type == Type::Object; })) {
				if (auto hasValueElements = element.hasValueChildren(); hasValueElements && hasValueElements.value()) {
					output << '\n'
						   << detail::escape(element.getParentPath()) << '\n';
				}
				output << element;
			}
			break;
		}
		case Type::Value:
			output << detail::escape(ini.key) << " = " << detail::escape(std::get<Ini::ValueStorage>(ini.data)) << '\n';
			break;
	}
	return output;
}

modernIni::Result<void> modernIni::from_ini(const Ini& ini, std::string& value) {
	if (ini.type != Type::Value) {
		return std::unexpected(Error::WrongType);
	}
	value = std::get<Ini::ValueStorage>(ini.data);
	return {};
}

modernIni::Result<void> modernIni::from_ini(const Ini& ini, std::string_view& value) {
	if (ini.type != Type::Value) {
		return std::unexpected(Error::WrongType);
	}
	value = std::get<Ini::ValueStorage>(ini.data);
	return {};
}

modernIni::Result<void> modernIni::from_ini(const Ini& ini, bool& value) {
	// Type is checked in the get of std::string!

	auto str = ini.get<std::string>();
	if (!str) {
		return std::unexpected(str.error());
	}

	std::ranges::transform(str.value(), str.value().begin(), [](auto& c) {
		return std::tolower(c);
	});
	if (str == "true" || str == "on" || str == "1") {
		value = true;
		return {};
	}
	if (str == "false" || str == "off" || str == "0") {
		value = false;
		return {};
	}

	return std::unexpected(Error::InvalidValue);
}

modernIni::Result<void> modernIni::from_ini(const Ini& ini, char& value) {
	auto str = ini.get<std::string_view>();
	if (!str) {
		return std::unexpected(str.error());
	}
	if (str.value().size() != 1) {
		return std::unexpected(Error::InvalidValue);
	}
	value = str.value()[0];
	return {};
}

void modernIni::to_ini(Ini& ini, std::string_view value) {
	ini.setType(Type::Value);
	std::get<Ini::ValueStorage>(ini.data) = value;
}
