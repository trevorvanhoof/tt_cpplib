#pragma once

#include <unordered_set>
#include <unordered_map>
#include "tt_messages.h"
#include "tt_files.h"
#include "tt_strings.h"
#include "windont.h"

namespace TT {
    std::string readAllBytes(const std::string_view filename) {
        std::FILE* fp; 
        fopen_s(&fp, filename.data(), "rb");
        if (fp) {
            std::string contents;
            std::fseek(fp, 0, SEEK_END);
            contents.resize(std::ftell(fp));
            std::rewind(fp);
            std::fread(&contents[0], 1, contents.size(), fp);
            std::fclose(fp);
            return contents;
        }   
        error("Failed to open file for reading: '%s.'", filename.data());
        return "";
    }

    std::string readWithIncludes(const std::string_view filePath, std::unordered_set<std::string>& outDependencies) {
        // Read this file
        outDependencies.insert(filePath.data());
        std::string code = readAllBytes(filePath);

        // Find include statements
        std::vector<std::string> sections;
        size_t cursor = 0;
        size_t cursor2 = 0;
        while (cursor < code.size()) {
            // Ignore block comments
            if (
                code[cursor] == '/' &&
                code[cursor + 1] == '*'
                ) {
                size_t end = code.find("*/", cursor + 2);
                cursor = end + 2;
            }
            // Ignore line comments
            else if (
                code[cursor] == '/' &&
                code[cursor + 1] == '/')
            {
                size_t start = cursor;
                cursor += 2;
                while (code[cursor] != '\n' && code[cursor] != '\r' && cursor < code.size())
                    ++cursor;
            }
            // Include
            else if (code.compare(cursor, 10, "#include \"") == 0) {
                // Find trailing quote
                size_t end = code.find('"', cursor + 10) + 1;
                // Get text between quotes
                std::string includeName = code.substr(cursor + 10, end - cursor - 11);

                // Add snippet between previous include and this include
                sections.push_back(code.substr(cursor2, cursor - cursor2));
                cursor = end;
                cursor2 = end;

                // Add snippet from include path
                std::unordered_set<std::string> dependencies;
                std::string incCode = readWithIncludes(includeName, dependencies);
                for (const std::string& dependency : dependencies) {
                    if(outDependencies.find(dependency) != outDependencies.end())
                        outDependencies.insert(dependency);
                }
                sections.emplace_back(incCode);
            } else {
                ++cursor;
            }
        }

        // Add trailing code & return as string
        sections.push_back(code.substr(cursor2));
        return join(sections, "");
    }

    bool fileExists(const std::string_view filename) {
        if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(filename.data()) && GetLastError() == ERROR_FILE_NOT_FOUND)
            return false;
        return true;
    }

    unsigned long long fileLastWriteTime(const std::string_view filePath) {
        WIN32_FILE_ATTRIBUTE_DATA info = {};
        if(!GetFileAttributesExA(filePath.data(), GetFileExInfoStandard, &info))
            return 0;
        return static_cast<unsigned long long>(info.ftLastWriteTime.dwHighDateTime) << 32 | static_cast<unsigned long long>(info.ftLastWriteTime.dwLowDateTime);
    }

    BinaryReader::BinaryReader(const std::string_view filename) {
        fopen_s(&fp, filename.data(), "rb");
        TT::assert(fp != nullptr, "Failed to open file for reading: '%s.'", filename);
    }

    size_t BinaryReader::readInto(std::string& target) const {
        return std::fread((void*)target.data(), 1, target.size(), fp);
    }

    size_t BinaryReader::readInto(char* target, size_t size) const {
        return std::fread((void*)target, 1, size, fp);
    }

    void BinaryReader::ensureReadInto(std::string& target) const {
        TT::assert(target.size() == readInto(target));
    }

    std::string BinaryReader::read(size_t size) const {
        std::string contents;
        contents.resize(size);
        assert(size == readInto(contents));
        return contents;
    }

    unsigned char BinaryReader::u8() const { return read<unsigned char>(); }
    unsigned short BinaryReader::u16() const { return read<unsigned short>(); }
    unsigned int BinaryReader::u32() const { return read<unsigned int>(); }
    unsigned long long BinaryReader::u64() const { return read<unsigned long long>(); }

    char BinaryReader::i8() const { return read<char>(); }
    short BinaryReader::i16() const { return read<short>(); }
    int BinaryReader::i32() const { return read<int>(); }
    long long BinaryReader::i64() const { return read<long long>(); }
    
    float BinaryReader::f32() const { return read<float>(); }
    double BinaryReader::f64() const { return read<double>(); }

    BinaryWriter::BinaryWriter(const std::string_view filename) {
        fopen_s(&fp, filename.data(), "wb");
        TT::assert(fp != nullptr, "Failed to open file for writing: '%s.'", filename);
    }

    std::string readWithIncludes(const std::string& filePath) {
        std::unordered_set<std::string> outDependencies;
        std::unordered_map<std::string, std::string> cache;
        std::unordered_map<std::string, std::vector<std::string>> fileDependents;
        return readWithIncludes(filePath, outDependencies, cache, fileDependents);
    }

    std::string readWithIncludes(
        const std::string& filePath, // file to load
        std::unordered_set<std::string>& outDependencies, // filePath needs these file paths to get fully loaded; including self
        std::unordered_map<std::string, std::string>& cache, // avoid reading the same file twice, e.g. when it gets included in multiple places
        std::unordered_map<std::string, std::vector<std::string>>& fileDependents) { // for each file, track which files rely on this file
        // Return the contents of this file if it is known
        if (cache.find(filePath) != cache.end())
            return cache.find(filePath)->second;

        // Read this file
        outDependencies.insert(filePath);

        fileDependents[filePath].push_back(filePath);

        std::string code = readAllBytes(filePath.c_str());

        std::vector<std::string> sections;
        size_t cursor = 0;
        size_t cursor2 = 0;
        while (cursor < code.size()) {
            if (
                code[cursor] == '/' &&
                code[cursor + 1] == '*'
                ) {
                size_t end = code.find("*/", cursor + 2);
                cursor = end + 2;
            }
            else if (
                code[cursor] == '/' &&
                code[cursor + 1] == '/')
            {
                size_t start = cursor;
                cursor += 2;
                while (code[cursor] != '\n' && code[cursor] != '\r' && cursor < code.size())
                    ++cursor;
            }
            else if (code.compare(cursor, 10, "#include \"") == 0) {
                size_t end = code.find('"', cursor + 10) + 1;
                std::string includeName = code.substr(cursor + 10, end - cursor - 11);

                // Add snippet between previous include and this include
                sections.push_back(code.substr(cursor2, cursor - cursor2));
                cursor = end;
                cursor2 = end;

                // Add snippet from include path
                std::unordered_set<std::string> dependencies;
                std::string incCode = readWithIncludes(includeName, dependencies, cache, fileDependents);
                for (const std::string& dependency : dependencies) {
                    fileDependents[dependency].push_back(filePath);
                    outDependencies.insert(dependency);
                }
                sections.emplace_back(incCode);
            }
            else {
                ++cursor;
            }
        }

        // Add trailing code
        sections.push_back(code.substr(cursor2));

        // Return code and dependencies
        code = join(sections, "");
        // info(code.data());

        // Cache
        cache[filePath] = code;
        return code;
    }

}
