#ifndef DICTIONARYWORKER_H
#define DICTIONARYWORKER_H

#include <SuffixArray.h>
#include <thread>
#include <condition_variable>
#include <optional>
#include <string>
#include <QObject>

class MainWindow;

class Result {
public:
    Result(std::string &&word, uint64_t version)
        : word(std::move(word)), version(version) {}

    Result() {}

    std::string word;
    uint64_t version;
};


class DictionaryWorker : public QObject
{
    Q_OBJECT
public:
    void setInput(std::optional<std::string> value, uint64_t version);
    Result getOuput();

    DictionaryWorker(std::string pathToDictionary);
    ~DictionaryWorker();
signals:
    void ready();
private:
    void storeResult(Result &&result);
    void threadProcces();


    std::mutex mutex;
    std::atomic<uint64_t> inputVersion;
    std::condition_variable inputChanged;
    std::optional<std::string> input;
    std::thread workingThread;
    bool notified = false;

    SuffixArray suffixArray;
    Result output;
    static constexpr uint64_t QUIT = 0;
};

#endif // DICTIONARYWORKER_H
