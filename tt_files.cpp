#pragma once

#include <unordered_set>
#include "tt_files.h"
#include "tt_strings.h"

namespace TT {
    std::string ReadAllBytes(const char* filename) {
        std::FILE* fp; fopen_s(&fp, filename, "rb");
        if (fp) {
            std::string contents;
            std::fseek(fp, 0, SEEK_END);
            contents.resize(std::ftell(fp));
            std::rewind(fp);
            std::fread(&contents[0], 1, contents.size(), fp);
            std::fclose(fp);
            return(contents);
        }
        throw(errno);
    }

    std::string ReadWithIncludes(const char* filename) {
        // TODO:
        // We need to track which files are in flight so we can assert on recursive includes.
        // To do so, we need to make paths absolute, which I don't remember how to do with the Windows API.
        // TODO: Skip #includes in comments
        std::string code = ReadAllBytes(filename);
        std::vector<size_t> offsets = FindAll(code, "#include \"");
        std::vector<size_t> ends;
        std::vector<std::string> includes;
        for (size_t offset : offsets) {
            size_t start = offset + 10;
            size_t end = code.find('\"', offset + 10);
            ends.push_back(end);
            std::string name = code.substr(start, end);
            includes.push_back(ReadWithIncludes(name.c_str()));
        }
        size_t cursor = 0;
        size_t size = 0;
        for (size_t i = 0; i < offsets.size(); ++i) {
            size += offsets[i] - cursor;
            size += includes[i].size();
            cursor = ends[i];
        }
        std::string result;
        result.resize(size);
        cursor = 0;
        size = 0;
        for (size_t i = 0; i < offsets.size(); ++i) {
            memcpy(&result[size], code.data() + cursor, offsets[i] - cursor);
            size += offsets[i] - cursor;
            memcpy(&result[size], includes[i].data(), includes[i].size());
            size += includes[i].size();
            cursor = ends[i];
        }
        return result;
    }
}
