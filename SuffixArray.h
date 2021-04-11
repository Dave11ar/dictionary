#ifndef SUFFIXARRAY_H
#define SUFFIXARRAY_H

#include <fstream>
#include <string>
#include <sstream>
#include <cstdio>
#include <QString>
#include <QDebug>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>

void *mmap(void *addr, size_t len, int prot, int flag, int filedes, off_t off);
int munmap(void *addr, size_t length);

typedef uint64_t array_t;

class SuffixArray
{
    static const char lineSeparator =
                #ifdef _WIN32
                    '\r\n';
                #else
                    '\n';
                #endif

    static const char fileSeparator =
                #ifdef _WIN32
                    '\\';
                #else
                    '/';
                #endif

    static constexpr array_t alphabet = (1 << (8 * sizeof(char) + 1));
public:
    array_t dictionaryLength;

    int32_t check(array_t pos, std::string &str);
    std::string getString(array_t pos);
    SuffixArray(std::string dictionaryFileName);
    ~SuffixArray();
private:
    array_t get(array_t *file, array_t index);

    template <typename T = array_t, bool character = false>
    T getFromDictionary(array_t index);
    void set(array_t *file, array_t index, array_t value);
    void createFiles();
    void buildSuffixArray();


    char *dictionary;
    array_t *suffixArray;
    array_t *halfArray;
    array_t *classesOld;
    array_t *classesNew;
    array_t *count;

    int64_t dictionaryFile;
    int64_t suffixArrayFile;
    int64_t halfArrayFile;
    int64_t classesOldFile;
    int64_t classesNewFile;
    int64_t countFile;

    std::string dictionaryFileName;
    std::string suffixArrayFileName;
    std::string halfArrayFileName;
    std::string classesOldFileName;
    std::string classesNewFileName;
    std::string countFileName;

    static const std::string prefixForFiles;
};


#endif // SUFFIXARRAY_H
