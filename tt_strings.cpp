#pragma once

#include "tt_strings.h"
#include "tt_messages.h"

namespace TT {
	size_t find(const std::string_view text, const std::string_view substr, size_t offset) {
		size_t end = text.size() - substr.size();
		for (size_t i = offset; i < end; ++i) {
			if (memcmp(text.data() + i, substr.data(), substr.size()) == 0)
				return i;
		}
		return -1;
	}

	std::vector<size_t> findAll(const std::string_view text, const std::string_view substr) {
		assert(substr.size() != 0);
		std::vector<size_t> result;
		if (substr.size() == 0) 
			return result;
		size_t cursor = 0;
		while (cursor < text.size() - substr.size()) {
			size_t next = find(text, substr, cursor);
			if (next == -1) 
				break;
			result.push_back(next);
			cursor = next + substr.size();
		}
		return result;
	}

	std::vector<std::string> split(const std::string_view s, const std::string_view delimiter) {
		size_t pos_start = 0, pos_end, delim_len = delimiter.length();
		std::string token;
		std::vector<std::string> res;

		while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
			token = s.substr(pos_start, pos_end - pos_start);
			pos_start = pos_end + delim_len;
			res.push_back(token);
		}

		res.emplace_back(s.substr(pos_start));
		return res;
	}
}
