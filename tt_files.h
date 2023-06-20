#pragma once

#include <string>

namespace TT {
	std::string readAllBytes(const ConstStringView& filename);
	std::string readWithIncludes(const ConstStringView& filePath, std::unordered_set<std::string>& outDependencies);
	bool exists(const ConstStringView& filename);

    struct BinaryReader {
        BinaryReader(const ConstStringView& filename);
		bool isValid() { return fp != nullptr; }
		
		// Returns nr of bytes read, if != target.count, EOF is reached.
		size_t readInto(const StringView& target) const;
		// Asserts if nr of bytes read != t arget.count
		void ensureReadInto(const StringView& target) const;
		// Reads given nr of bytes and returns resulting string,
		// string will be truncated if fewer bytes were available.
		std::string read(size_t size) const;
		// Read 4 bytes and return as u32.
        unsigned int u32() const;
		// Read 4 bytes and return as i32.
        int i32() const;
		// Read 4 bytes and return as f32.
        float f32() const;
		// Read a u32 size and then that many bytes into a string.
        std::string str() const;

	private:
		std::FILE* fp;
    };
}
