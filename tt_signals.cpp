#include "tt_signals.h"

namespace TT {
	Lifetime::Lifetime() {
		handle = nextHandle++;
		refCount[handle]++;
	}

	Lifetime::~Lifetime() {
		if (handle == -1) return;
		refCount[handle]--;
	}

	Lifetime::Lifetime(const Lifetime& other) {
		handle = other.handle;
		refCount[handle]++;
	}

	Lifetime::Lifetime(Lifetime&& other) noexcept {
		handle = other.handle;
		other.handle = -1;
	}

	Lifetime& Lifetime::operator=(const Lifetime& other) {
		handle = other.handle;
		refCount[handle]++;
		return *this;
	}

	Lifetime& Lifetime::operator=(Lifetime&& other) noexcept {
		handle = other.handle;
		other.handle = -1;
		return *this;
	}

	size_t Lifetime::nextHandle = 0;
	std::unordered_map<size_t, size_t> Lifetime::refCount;
}
