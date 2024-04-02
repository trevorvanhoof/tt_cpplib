#pragma once

#include <unordered_set>
#include <unordered_map>
#include "tt_messages.h"
#include "tt_files.h"
#include "tt_strings.h"
#include "windont.h"

namespace TT {
    std::string readAllBytes(const ConstStringView& filename) {
        std::FILE* fp; 
        fopen_s(&fp, filename.start, "rb");
        if (fp) {
            std::string contents;
            std::fseek(fp, 0, SEEK_END);
            contents.resize(std::ftell(fp));
            std::rewind(fp);
            std::fread(&contents[0], 1, contents.size(), fp);
            std::fclose(fp);
            return contents;
        }   
        error("Failed to open file for reading: '%s.'", filename);
        return "";
    }

    std::string readWithIncludes(const ConstStringView& filePath, std::unordered_set<std::string>& outDependencies) {
        // Read this file
        outDependencies.insert(filePath.start);
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

    bool exists(const ConstStringView& filename) {
        if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(filename.start) && GetLastError() == ERROR_FILE_NOT_FOUND)
            return false;
        return true;
    }

    BinaryReader::BinaryReader(const ConstStringView& filename) {
        fopen_s(&fp, filename.start, "rb");
        TT::assert(fp != nullptr, "Failed to open file for reading: '%s.'", filename);
    }

    size_t BinaryReader::readInto(const StringView& target) const {
        return std::fread(target.start, 1, target.count, fp);
    }

    void BinaryReader::ensureReadInto(const StringView& target) const {
        TT::assert(target.count == readInto(target));
    }

    std::string BinaryReader::read(size_t size) const {
        std::string contents;
        contents.resize(size);
        StringView view(contents);
        contents.resize(readInto(view));
        return contents;
    }

    unsigned int BinaryReader::u32() const {
        unsigned int result;
        TT::assert(sizeof(unsigned int) == std::fread((char*)&result, 1, sizeof(unsigned int), fp));
        return result;
    }

    int BinaryReader::i32() const {
        int result;
        TT::assert(sizeof(int) == std::fread((char*)&result, 1, sizeof(int), fp));
        return result;
    }

    std::string BinaryReader::str() const {
        return read(u32());
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
