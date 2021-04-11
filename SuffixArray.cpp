#include "SuffixArray.h"

SuffixArray::SuffixArray(std::string dictionaryFileName)
    : dictionaryFileName(dictionaryFileName),
      suffixArrayFileName("suffixArray.bin"),
      halfArrayFileName("halfArray.bin"),
      classesOldFileName("classesOld.bin"),
      classesNewFileName("classesNew.bin"),
      countFileName("count.bin")
{
    createFiles();

    if ((dictionaryFile = open(dictionaryFileName.data(), O_RDONLY)) == -1 ||
        (suffixArrayFile = open(suffixArrayFileName.data(), O_RDWR)) == -1  ||
        (halfArrayFile = open(halfArrayFileName.data(), O_RDWR)) == -1         ||
        (classesOldFile = open(classesOldFileName.data(), O_RDWR)) == -1   ||
        (classesNewFile = open(classesNewFileName.data(), O_RDWR)) == -1   ||
        (countFile = open(countFileName.data(), O_RDWR)) == -1)  {
        qDebug() << "Can not open files";
        exit(1);
    }


    dictionary = (char*)mmap(NULL, dictionaryLength * sizeof(char), PROT_READ, MAP_PRIVATE, dictionaryFile, 0);
    suffixArray = (array_t*)mmap(NULL, dictionaryLength * sizeof(array_t), PROT_READ | PROT_WRITE, MAP_PRIVATE, suffixArrayFile, 0);
    halfArray = (array_t*)mmap(NULL, dictionaryLength * sizeof(array_t), PROT_READ | PROT_WRITE, MAP_PRIVATE, halfArrayFile, 0);
    classesOld = (array_t*)mmap(NULL, dictionaryLength * sizeof(array_t), PROT_READ | PROT_WRITE, MAP_PRIVATE, classesOldFile, 0);
    classesNew = (array_t*)mmap(NULL, dictionaryLength * sizeof(array_t), PROT_READ | PROT_WRITE, MAP_PRIVATE, classesNewFile, 0);
    count = (array_t*)mmap(NULL, dictionaryLength * sizeof(array_t), PROT_READ | PROT_WRITE, MAP_PRIVATE, countFile, 0);


    buildSuffixArray();
}

int32_t SuffixArray::check(array_t pos, std::string &str)
{
    pos = get(suffixArray, pos);
    for (char c : str) {
        if (pos == dictionaryLength) {
            return 0;
        }

        char symbol = getFromDictionary<char, true>(pos++);
        if (symbol < c) {
            return -1;
        } else if (symbol > c) {
            return 1;
        }
    }

    return 0;
}

SuffixArray::~SuffixArray()
{
    munmap(dictionary, dictionaryLength * sizeof(char));
    munmap(suffixArray, dictionaryLength * sizeof(array_t));
    munmap(halfArray, dictionaryLength * sizeof(array_t));
    munmap(classesNew, dictionaryLength * sizeof(array_t));
    munmap(classesOld, dictionaryLength * sizeof(array_t));
    munmap(count, dictionaryLength * sizeof(array_t));

    close(dictionaryFile);
    close(suffixArrayFile);
    close(halfArrayFile);
    close(classesNewFile);
    close(classesOldFile);
    close(countFile);


    std::remove(halfArrayFileName.data());
    std::remove(classesOldFileName.data());
    std::remove(classesNewFileName.data());
    std::remove(countFileName.data());
}

template <typename T, bool character>
T SuffixArray::getFromDictionary(array_t index)
{
    char result;
    if (index == dictionaryLength - 1) {
        result = '\0';
    } else {
        result = *(dictionary + index);
    }

    if constexpr (character) {
        return result;
    } else {
        return static_cast<int64_t>(result) + (1 << (8 * sizeof(char)));
    }
}

array_t SuffixArray::get(array_t *file, array_t index)
{
    array_t result;
    memcpy(&result, file + index , sizeof(array_t));
    return result;
}

void SuffixArray::set(array_t *file, array_t index, array_t value)
{
    memcpy(file + index, &value, sizeof(array_t));
}

std::string SuffixArray::getString(array_t pos)
{
    pos = get(suffixArray, pos);

    array_t index = pos;

    std::string result;
    while (true) {
        char cur = getFromDictionary<char, true>(index--);
        if (cur == lineSeparator) {
            break;
        }
        result.push_back(cur);
        if (index == 0) {
            break;
        }
    }

    std::reverse(result.begin(), result.end());

    for (array_t index = pos + 1; index < dictionaryLength; index++) {
        char cur = getFromDictionary<char, true>(index);
        if (cur == lineSeparator || cur == '\0') {
            break;
        }
        result.push_back(cur);
    }

    return result;
}

void SuffixArray::createFiles()
{
    std::ifstream _dictionary(dictionaryFileName);
    std::ofstream _suffixArray;
    std::ofstream _halfArray;
    std::ofstream _classesOld;
    std::ofstream _classesNew;
    std::ofstream _count;

    _suffixArray.open(suffixArrayFileName, std::ios::binary | std::ios::out);
    _halfArray.open(halfArrayFileName.data(), std::ios::binary | std::ios::out);
    _classesOld.open(classesOldFileName.data(), std::ios::binary | std::ios::out);
    _classesNew.open(classesNewFileName.data(), std::ios::binary | std::ios::out);
    _count.open(countFileName.data(), std::ios::binary | std::ios::out);

    _dictionary.seekg(0, std::ios::end);
    dictionaryLength = _dictionary.tellg();
    dictionaryLength++;

    _suffixArray.seekp(dictionaryLength * sizeof(array_t));
    _halfArray.seekp(dictionaryLength * sizeof(array_t));
    _classesOld.seekp(dictionaryLength * sizeof(array_t));
    _classesNew.seekp(dictionaryLength * sizeof(array_t));
    _count.seekp(dictionaryLength * sizeof(array_t));

    _suffixArray << '\0';
    _halfArray << '\0';
    _classesOld << '\0';
    _classesNew << '\0';
    _count << '\0';

    _dictionary.close();
    _suffixArray.close();
    _halfArray.close();
    _classesOld.close();
    _classesNew.close();
    _count.close();
}

void SuffixArray::buildSuffixArray()
{
    array_t classes = 1;

    for (array_t index = 0; index < alphabet; index++) {
        set(count, index, 0);
    }

    for (array_t index = 0; index < dictionaryLength; index++) {
        array_t ind = getFromDictionary(index);
        set(count, ind, get(count, ind) + 1);
    }

    for (array_t index = 1; index < alphabet; index++) {
        set(count, index, get(count, index) + get(count, index - 1));
    }

    for (array_t index = 0; index < dictionaryLength; index++) {
        array_t symbol = getFromDictionary(index);
        array_t lastCount = get(count, symbol);
        set(count, symbol, --lastCount);

        set(suffixArray, lastCount, index);
    }

    set(classesOld, get(suffixArray, 0), 0);
    for (array_t index = 1; index < dictionaryLength; index++) {
        if (getFromDictionary(get(suffixArray, index)) !=
            getFromDictionary(get(suffixArray, index - 1))) {
            classes++;
        }

        set(classesOld, get(suffixArray, index), classes - 1);
    }


    for (array_t step = 0; (static_cast<array_t>(1) << step) < dictionaryLength; step++) {
        qDebug() << step;
        for (array_t index = 0; index < dictionaryLength; index++) {
            int64_t value = static_cast<int64_t>(get(suffixArray, index)) - (1 << step);
            set(halfArray, index, value + (value < 0 ? dictionaryLength : 0));
        }

        for (array_t index = 0; index < classes; index++) {
            set(count, index, 0);
        }

        for (array_t index = 0; index < dictionaryLength; index++) {
            array_t ind = get(classesOld, get(halfArray, index));
            set(count, ind, get(count, ind) + 1);
        }

        for (array_t index = 1; index < classes; index++) {
            set(count, index, get(count, index) + get(count, index - 1));
        }

        for (array_t index = dictionaryLength; index > 0; index--) {
            array_t correct_ind = index - 1;
            array_t countIndex = get(classesOld, get(halfArray, correct_ind));
            set(count, countIndex, get(count, countIndex) - 1);

            set(suffixArray, get(count, countIndex), get(halfArray, correct_ind));
        }

        set(classesNew, get(suffixArray, 0), 0);
        classes = 1;
        for (array_t index = 1; index < dictionaryLength; index++) {
            array_t midFir = (get(suffixArray, index) + (1 << step)) % dictionaryLength;
            array_t midSec = (get(suffixArray, index - 1) + (1 << step)) % dictionaryLength;

            if (get(classesOld, get(suffixArray, index)) != get(classesOld, get(suffixArray, index - 1)) ||
                get(classesOld, midFir) != get(classesOld, midSec)) {
                classes++;
            }

            set(classesNew, get(suffixArray, index), classes - 1);
        }


        for (array_t index = 0; index < dictionaryLength; index++) {
            set(classesOld, index, get(classesNew, index));
        }
    }
}
