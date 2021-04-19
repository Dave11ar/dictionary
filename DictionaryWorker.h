#ifndef DICTIONARYWORKER_H
#define DICTIONARYWORKER_H

#include <SuffixArray.h>
#include <QObject>
#include <thread>
#include <condition_variable>
#include <optional>
#include <string>

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

    std::mutex inputMutex;

    std::atomic<uint64_t> inputVersion;
    std::atomic<bool> notified = false;

    std::condition_variable inputChanged;
    std::condition_variable notifyChanged;

    std::optional<std::string> input;
    Result output;

    std::thread workingThread;
    SuffixArray suffixArray;

    static constexpr uint64_t QUIT = 0;
};

#endif // DICTIONARYWORKER_H
