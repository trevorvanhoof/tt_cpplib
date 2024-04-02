#pragma once

#include "tt_strings.h"
#include "tt_messages.h"

namespace TT {
	size_t find(const ConstStringView& text, const ConstStringView& substr, size_t offset) {
		size_t end = text.count - substr.count;
		for (size_t i = offset; i < end; ++i) {
			if (memcmp(text.start + i, substr.start, substr.count) == 0)
				return i;
		}
		return -1;
	}

	std::vector<size_t> findAll(const ConstStringView& text, const ConstStringView& substr) {
		assert(substr.count != 0);
		std::vector<size_t> result;
		if (substr.count == 0) 
			return result;
		size_t cursor = 0;
		while (cursor < text.count - substr.count) {
			size_t next = find(text, substr, cursor);
			if (next == -1) 
				break;
			result.push_back(next);
			cursor = next + substr.count;
		}
		return result;
	}

	std::vector<std::string> split(std::string s, std::string delimiter) {
		size_t pos_start = 0, pos_end, delim_len = delimiter.length();
		std::string token;
		std::vector<std::string> res;

		while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
			token = s.substr(pos_start, pos_end - pos_start);
			pos_start = pos_end + delim_len;
			res.push_back(token);
		}

		res.push_back(s.substr(pos_start));
		return res;
	}
}
