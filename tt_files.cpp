#pragma once

#include "tt_files.h"

namespace TT {
    std::string ReadAllBytes(const char* filename)
    {
        std::FILE* fp; fopen_s(&fp, filename, "rb");
        if (fp)
        {
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
}