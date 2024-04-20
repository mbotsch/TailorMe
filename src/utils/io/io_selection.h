#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static bool read_selection(const std::string& fn_selection, std::vector<int>& vertex_ids)
{
    std::ifstream index_file(fn_selection);

    if (!index_file)
    {
        printf("[ERROR] Cannot read %s for reading vertex selection\n", fn_selection.c_str());
        return false;
    }

    std::string line;
    while (std::getline(index_file, line))
    {
        int index;
        std::stringstream ss(line);
        ss >> index;

        if (!ss.fail())
        {
            vertex_ids.push_back(index);
        }
    }
    index_file.close();

    return true;
}


