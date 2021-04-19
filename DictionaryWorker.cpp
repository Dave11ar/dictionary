#include "DictionaryWorker.h"

void DictionaryWorker::setInput(std::optional<std::string> value, uint64_t version)
{
    {
        std::lock_guard lg(inputMutex);
        input = value;
        inputVersion = version;
        notified = false;
    }

    notifyChanged.notify_one();
    inputChanged.notify_one();
}

Result DictionaryWorker::getOuput()
{
    std::lock_guard lg(inputMutex);
    notified = false;
    notifyChanged.notify_one();
    return output;
}

DictionaryWorker::DictionaryWorker(std::string pathToDictionary)
    : inputVersion(QUIT + 1),
      output(),
      workingThread([this] {threadProcces();}),
      suffixArray(std::move(pathToDictionary)) {
}

DictionaryWorker::~DictionaryWorker()
{
    inputVersion = QUIT;
    inputChanged.notify_one();
    workingThread.join();
}

void DictionaryWorker::storeResult(Result &&result)
{
    {
        std::lock_guard lg(inputMutex);
        output = std::move(result);
        emit ready();
        notified = true;
    }
    notifyChanged.notify_one();
}

void DictionaryWorker::threadProcces()
{
    std::mutex _dummyMutex;
    std::unique_lock _dummyUniqueLock(_dummyMutex);

    std::atomic<uint64_t> lastInputVersion = 0;

    while (true) {
        std::optional<std::string> input_copy;
        {
            std::unique_lock lg(inputMutex);
            inputChanged.wait(lg, [&]
            {
                return inputVersion != lastInputVersion;
            });

            lastInputVersion = uint64_t(inputVersion);
            if (lastInputVersion == QUIT) {
                break;
            }

            input_copy = input;
        }

        std::optional<std::string> result;
        if (input_copy) {
            array_t left, leftPos;
            array_t right, rightPos;


            left = -1;
            right = suffixArray.dictionaryLength;
            while (left + 1 < right) {
                if (inputVersion != lastInputVersion) {
                    break;
                }

                array_t mid = (left + right) / 2;

                int32_t check = suffixArray.check(mid, *input_copy);
                if (check == -1) {
                    left = mid;
                } else {
                    right = mid;
                }
            }
            leftPos = right;


            left = -1;
            right = suffixArray.dictionaryLength;
            while (left + 1 < right) {
                if (inputVersion != lastInputVersion) {
                    break;
                }

                array_t mid = (left + right) / 2;

                int32_t check = suffixArray.check(mid, *input_copy);
                if (check == 1) {
                    right = mid;
                } else {
                    left = mid;
                }
            }
            rightPos = left;



            for (array_t index = leftPos; index <= rightPos && inputVersion == lastInputVersion; index++) {
                notifyChanged.wait(_dummyUniqueLock, [&]
                {
                    return !notified;
                });

                storeResult(Result(suffixArray.getString(index), lastInputVersion));
            }
        }
    }
}
