#pragma once

#include "tt_strings.h"
#include "tt_messages.h"
#include <string>
#include <unordered_set>
#include <intrin.h>
#include <string_view>
#include <unordered_map>

namespace TT {
	std::string readAllBytes(const std::string_view filename);
	std::string readWithIncludes(const std::string_view filePath, std::unordered_set<std::string>& outDependencies);
	bool fileExists(const std::string_view filename);
    unsigned long long fileLastWriteTime(const std::string_view filename);

    struct BinaryReader {
        BinaryReader(const std::string_view filename);
		bool isValid() { return fp != nullptr; }
		
		// Returns nr of bytes read, if != target.count, EOF is reached.
		size_t readInto(std::string& target) const;
		size_t readInto(char* target, size_t size) const;
		// Asserts if nr of bytes read != t arget.count
		void ensureReadInto(std::string& target) const;
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

    struct BinaryWriter {
        BinaryWriter(const std::string_view filename);
        bool isValid() { return fp != nullptr; }

        template<typename T> void write(const T& value) const {
            std::fwrite(&value, sizeof(T), 1, fp);
        }

        void write(void* buffer, size_t size) const { std::fwrite(buffer, size, 1, fp); }

        void u8(unsigned char value) const { write<unsigned char>(value); }
        void u16(unsigned short value) const { write<unsigned short>(value); }
        void u32(unsigned int value) const { write<unsigned int>(value); }
        void u64(unsigned long long value) const { write<unsigned long long>(value); }

        void i8(char value) const { write<char>(value); }
        void i16(short value) const { write<short>(value); }
        void i32(int value) const { write<int>(value); }
        void i64(long long value) const { write<long long>(value); }

        void f32(float value) const { write<float>(value); }
        void f64(double value) const { write<double>(value); }

        // Write the size and then the contents of the string. T must be one of u8, u16, u32, u64.
        template<typename T> void str(const std::string& value) const {
            write<T>(value.size());
            std::fwrite(value.data(), value.size(), 1, fp);
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
