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
}
