#pragma once

#include "tt_strings.h"
#include "tt_messages.h"

namespace TT {
	size_t Find(const std::string& text, const std::string& substr, size_t offset) {
		size_t end = text.size();
		for (size_t i = offset; i < end; ++i) {
			if (memcmp(text.data() + i, substr.data(), substr.size()) == 0)
				return i;
		}
		return -1;
	}

	std::vector<size_t> FindAll(const std::string& text, const std::string& substr) {
		TT::Assert(substr.size() != 0);
		std::vector<size_t> result;
		if (substr.size() == 0) return result;
		size_t cursor = 0;
		while (cursor <= text.size() + substr.size()) {
			size_t next = Find(text, substr, cursor);
			if (next == -1) break;
			result.push_back(next);
			cursor = next + substr.size();
		}
		return result;
	}
}
