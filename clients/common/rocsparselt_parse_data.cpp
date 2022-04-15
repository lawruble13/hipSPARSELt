/* ************************************************************************
 * Copyright (c) 2019-2022 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#include "rocsparselt_parse_data.hpp"
#include "rocsparselt_data.hpp"
#include "utility.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#ifdef WIN32
#ifdef __cpp_lib_filesystem
#include <filesystem>
#else
#include <experimental/filesystem>

namespace std
{
    namespace filesystem = experimental::filesystem;
}
#endif
#endif // WIN32

// Parse YAML data
static std::string rocsparselt_parse_yaml(const std::string& yaml)
{
    std::string tmp     = rocsparselt_tempname();
    auto        exepath = rocsparselt_exepath();
    auto        cmd     = exepath + "rocsparselt_gentest.py --template " + exepath
               + "rocsparselt_template.yaml -o " + tmp + " " + yaml;
    rocsparselt_cerr << cmd << std::endl;

#ifdef WIN32
    int status = std::system(cmd.c_str());
    if(status == -1)
        exit(EXIT_FAILURE);
#else
    int status = system(cmd.c_str());
    if(status == -1 || !WIFEXITED(status) || WEXITSTATUS(status))
        exit(EXIT_FAILURE);
#endif

    return tmp;
}

// Parse --data and --yaml command-line arguments
bool rocsparselt_parse_data(int& argc, char** argv, const std::string& default_file)
{
    std::string filename;
    char**      argv_p = argv + 1;
    bool        help = false, yaml = false;

    // Scan, process and remove any --yaml or --data options
    for(int i = 1; argv[i]; ++i)
    {
        if(!strcmp(argv[i], "--data") || !strcmp(argv[i], "--yaml"))
        {
            if(!strcmp(argv[i], "--yaml"))
            {
                yaml = true;
            }

            if(filename != "")
            {
                rocsparselt_cerr << "Only one of the --yaml and --data options may be specified"
                                 << std::endl;
                exit(EXIT_FAILURE);
            }

            if(!argv[i + 1] || !argv[i + 1][0])
            {
                rocsparselt_cerr << "The " << argv[i] << " option requires an argument"
                                 << std::endl;
                exit(EXIT_FAILURE);
            }
            filename = argv[++i];
        }
        else
        {
            *argv_p++ = argv[i];
            if(!help && (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")))
            {
                help = true;
                rocsparselt_cout << "\n"
                                 << argv[0] << " [ --data <path> | --yaml <path> ] <options> ...\n"
                                 << std::endl;
            }
        }
    }

    // argc and argv contain remaining options and non-option arguments
    *argv_p = nullptr;
    argc    = argv_p - argv;

    if(filename == "-")
        filename = "/dev/stdin";
    else if(filename == "")
        filename = default_file;

    if(yaml)
        filename = rocsparselt_parse_yaml(filename);

    if(filename != "")
    {
        RocSparseLt_TestData::set_filename(filename, yaml);
        return true;
    }

    return false;
}