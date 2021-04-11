#include "DictionaryWorker.h"

void DictionaryWorker::setInput(std::optional<std::string> value, uint64_t version)
{
    {
        std::lock_guard lg(mutex);
        input = value;
        inputVersion = version;
    }
    inputChanged.notify_all();
}

void DictionaryWorker::notify()
{
    {
        std::lock_guard lg(mutex);
        notified = false;
    }
    emit ready();
}

Result DictionaryWorker::getOuput()
{
    std::lock_guard lg(mutex);
    return output;
}

DictionaryWorker::DictionaryWorker(std::string pathToDictionary)
    : inputVersion(QUIT + 1),
      workingThread([this] {threadProcces();}),
      suffixArray(std::move(pathToDictionary)),
      output() {
}

DictionaryWorker::~DictionaryWorker()
{
    inputVersion = QUIT;
    inputChanged.notify_all();
    workingThread.join();
}


void DictionaryWorker::storeResult(Result &&result)
{
    std::lock_guard lg(mutex);
    output = std::move(result);

    if (!notified) {
        QMetaObject::invokeMethod(this, "notify");
        notified = true;
    }
}

void DictionaryWorker::threadProcces()
{
    uint64_t lastInputVersion = 0;

    while (true) {
        std::optional<std::string> input_copy;
        {
            std::unique_lock lg(mutex);
            inputChanged.wait(lg, [&]
            {
                return inputVersion != lastInputVersion;
            });
            lastInputVersion = inputVersion;
            if (lastInputVersion == QUIT) {
                break;
            }

            input_copy = input;
        }

        std::optional<std::string> result;
        if (input_copy) {
            array_t left_pos;
            array_t right_pos;

            int64_t left = -1;
            int64_t right = suffixArray.dictionaryLength;

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
            left_pos = right;

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
            right_pos = left;

            uint version;
            {
                std::lock_guard lg(mutex);
                version = inputVersion;
            }

            for (array_t index = left_pos; index <= right_pos; index++) {
                while (notified) {}

                if (inputVersion != lastInputVersion) {
                    break;
                }

                storeResult(Result(suffixArray.getString(index), version));
            }
        }
    }
}

