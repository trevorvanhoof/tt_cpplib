#pragma once

#include "tt_strings.h"
#include "tt_messages.h"
#include <string>
#include <unordered_set>
#include <intrin.h>

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

        bool swapEndianness = false;

        template<typename T> T read() const {
            T result;
            TT::assert(sizeof(T) == std::fread((char*)&result, 1, sizeof(T), fp));
            if(!swapEndianness)
                return result;
            switch(sizeof(T)) {
            case 2:
                {
                    unsigned short tmp = _byteswap_ushort(*reinterpret_cast<unsigned short*>(&result));
                    result = *reinterpret_cast<T*>(&tmp);
                }
                break;
            case 4:
                {
                    unsigned int tmp = _byteswap_ulong(*reinterpret_cast<unsigned int*>(&result));
                    result = *reinterpret_cast<T*>(&tmp);
                }
                break;
            case 8:
                {
                    unsigned long long tmp = _byteswap_uint64(*reinterpret_cast<unsigned long long*>(&result));
                    result = *reinterpret_cast<T*>(&tmp);
                }
                break;
            }
            return result;
        }

        unsigned char u8() const;
        unsigned short u16() const;
        unsigned int u32() const;
        unsigned long long u64() const;

        char i8() const;
        short i16() const;
        int i32() const;
        long long i64() const;

        float f32() const;
        double f64() const;

		// Read a size and then that many bytes into a string. T must be one of u8, u16, u32, u64.
        template<typename T> std::string str() const {
            return read(read<T>());
        }

	private:
		std::FILE* fp;
    };

	// Read a file, look for #include statements, can deal with c-style comments but not with other macros.
	std::string readWithIncludes(const std::string& filePath);

	// This more complicated version exposes a lot of the internal logic for use in file watching.
	std::string readWithIncludes(
		const std::string& filePath, // file to load
		std::unordered_set<std::string>& outDependencies, // filePath needs these file paths to get fully loaded; including self
		std::unordered_map<std::string, std::string>& cache, // avoid reading the same file twice, e.g. when it gets included in multiple places
		std::unordered_map<std::string, std::vector<std::string>>& fileDependents); // for each file, track which files rely on this file
}
