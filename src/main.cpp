#include <algorithm>
#include <chrono>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>

using namespace std;

class Tokenizer {
public:
    static vector<string> tokenize(const string& text) {
        vector<string> tokens;
        string word;

        for (char ch : text) {
            if (isalnum(static_cast<unsigned char>(ch))) {
                word += static_cast<char>(tolower(ch));
            } else {
                if (!word.empty()) {
                    tokens.push_back(word);
                    word.clear();
                }
            }
        }

        if (!word.empty()) {
            tokens.push_back(word);
        }

        return tokens;
    }

    static string trimQuotes(const string& query) {
        string result = query;

        while (!result.empty() && isspace(static_cast<unsigned char>(result.front()))) {
            result.erase(result.begin());
        }

        while (!result.empty() && isspace(static_cast<unsigned char>(result.back()))) {
            result.pop_back();
        }

        if (result.size() >= 2 && result.front() == '"' && result.back() == '"') {
            result = result.substr(1, result.size() - 2);
        }

        return result;
    }

    static bool isPhraseQuery(const string& query) {
        string result = query;

        while (!result.empty() && isspace(static_cast<unsigned char>(result.front()))) {
            result.erase(result.begin());
        }

        while (!result.empty() && isspace(static_cast<unsigned char>(result.back()))) {
            result.pop_back();
        }

        return result.size() >= 2 && result.front() == '"' && result.back() == '"';
    }
};

class SearchEngine {
private:
    vector<string> documents;
    vector<int> documentLengths;
    vector<vector<string>> documentTokens;

    // term -> docId -> frequency
    unordered_map<string, unordered_map<int, int>> invertedIndex;

    // Windows synchronization primitive
    CRITICAL_SECTION indexLock;

    double averageDocumentLength;

    struct WorkerArgs {
        SearchEngine* engine;
        const vector<string>* files;
        int start;
        int end;
    };

    vector<string> getTextFilesFromFolder(const string& folderPath) {
        vector<string> files;
        string searchPath = folderPath + "\\*.txt";

        WIN32_FIND_DATAA fileData;
        HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fileData);

        if (hFind == INVALID_HANDLE_VALUE) {
            cerr << "No .txt files found in folder: " << folderPath << endl;
            return files;
        }

        do {
            string fileName = fileData.cFileName;
            string fullPath = folderPath + "\\" + fileName;
            files.push_back(fullPath);
        } while (FindNextFileA(hFind, &fileData));

        FindClose(hFind);
        sort(files.begin(), files.end());
        return files;
    }

    void indexDocument(const string& filePath, int docId) {
        ifstream file(filePath);

        if (!file.is_open()) {
            cerr << "Could not open file: " << filePath << endl;
            return;
        }

        stringstream buffer;
        buffer << file.rdbuf();

        string content = buffer.str();
        vector<string> tokens = Tokenizer::tokenize(content);

        unordered_map<string, int> localTermFrequency;

        for (const string& token : tokens) {
            localTermFrequency[token]++;
        }

        documents[docId] = filePath;
        documentLengths[docId] = static_cast<int>(tokens.size());
        documentTokens[docId] = tokens;

        // Critical section: only shared invertedIndex update is locked.
        EnterCriticalSection(&indexLock);

        for (const auto& item : localTermFrequency) {
            const string& term = item.first;
            int frequency = item.second;
            invertedIndex[term][docId] = frequency;
        }

        LeaveCriticalSection(&indexLock);
    }

    void workerFunction(const vector<string>& files, int start, int end) {
        for (int i = start; i < end; i++) {
            indexDocument(files[i], i);
        }
    }

    static DWORD WINAPI threadEntryPoint(LPVOID param) {
        WorkerArgs* args = static_cast<WorkerArgs*>(param);
        args->engine->workerFunction(*(args->files), args->start, args->end);
        return 0;
    }

    bool containsPhrase(int docId, const vector<string>& phraseTokens) const {
        if (phraseTokens.empty()) {
            return false;
        }

        const vector<string>& tokens = documentTokens[docId];

        if (phraseTokens.size() > tokens.size()) {
            return false;
        }

        for (int i = 0; i <= static_cast<int>(tokens.size() - phraseTokens.size()); i++) {
            bool match = true;

            for (int j = 0; j < static_cast<int>(phraseTokens.size()); j++) {
                if (tokens[i + j] != phraseTokens[j]) {
                    match = false;
                    break;
                }
            }

            if (match) {
                return true;
            }
        }

        return false;
    }

public:
    SearchEngine() {
        InitializeCriticalSection(&indexLock);
        averageDocumentLength = 0.0;
    }

    ~SearchEngine() {
        DeleteCriticalSection(&indexLock);
    }

    int indexFolder(const string& folderPath, int threadCount) {
        vector<string> files = getTextFilesFromFolder(folderPath);

        int totalDocs = static_cast<int>(files.size());

        if (totalDocs == 0) {
            return 0;
        }

        if (threadCount <= 0) {
            threadCount = 1;
        }

        threadCount = min(threadCount, totalDocs);

        documents.resize(totalDocs);
        documentLengths.resize(totalDocs, 0);
        documentTokens.resize(totalDocs);

        vector<HANDLE> threadHandles;
        vector<WorkerArgs> workerArgs(threadCount);

        int chunkSize = (totalDocs + threadCount - 1) / threadCount;

        for (int t = 0; t < threadCount; t++) {
            int start = t * chunkSize;
            int end = min(start + chunkSize, totalDocs);

            if (start >= end) {
                continue;
            }

            workerArgs[t].engine = this;
            workerArgs[t].files = &files;
            workerArgs[t].start = start;
            workerArgs[t].end = end;

            HANDLE hThread = CreateThread(
                NULL,
                0,
                SearchEngine::threadEntryPoint,
                &workerArgs[t],
                0,
                NULL
            );

            if (hThread != NULL) {
                threadHandles.push_back(hThread);
            }
        }

        for (HANDLE hThread : threadHandles) {
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
        }

        long long totalLength = 0;

        for (int length : documentLengths) {
            totalLength += length;
        }

        averageDocumentLength = totalDocs > 0 ? static_cast<double>(totalLength) / totalDocs : 0.0;

        return static_cast<int>(threadHandles.size());
    }

    vector<pair<string, double>> search(const string& query, int topK, const string& rankingMode) {
        bool phraseMode = Tokenizer::isPhraseQuery(query);
        string cleanedQuery = Tokenizer::trimQuotes(query);
        vector<string> queryTokens = Tokenizer::tokenize(cleanedQuery);

        unordered_map<int, double> docScores;
        int totalDocs = static_cast<int>(documents.size());

        vector<bool> allowedDocs(totalDocs, true);

        if (phraseMode) {
            fill(allowedDocs.begin(), allowedDocs.end(), false);

            for (int docId = 0; docId < totalDocs; docId++) {
                if (containsPhrase(docId, queryTokens)) {
                    allowedDocs[docId] = true;
                }
            }
        }

        for (const string& term : queryTokens) {
            if (invertedIndex.find(term) == invertedIndex.end()) {
                continue;
            }

            int docsWithTerm = static_cast<int>(invertedIndex[term].size());

            for (const auto& item : invertedIndex[term]) {
                int docId = item.first;
                int termFrequency = item.second;

                if (!allowedDocs[docId] || documentLengths[docId] == 0) {
                    continue;
                }

                double score = 0.0;

                if (rankingMode == "bm25") {
                    double k1 = 1.5;
                    double b = 0.75;

                    double idf = log(1.0 + (totalDocs - docsWithTerm + 0.5) / (docsWithTerm + 0.5));
                    double numerator = termFrequency * (k1 + 1.0);
                    double denominator = termFrequency + k1 * (1.0 - b + b * (documentLengths[docId] / averageDocumentLength));

                    score = idf * (numerator / denominator);
                } else {
                    double idf = log((1.0 + totalDocs) / (1.0 + docsWithTerm)) + 1.0;
                    double tf = static_cast<double>(termFrequency) / documentLengths[docId];

                    score = tf * idf;
                }

                docScores[docId] += score;
            }
        }

        priority_queue<pair<double, int>> pq;

        for (const auto& item : docScores) {
            int docId = item.first;
            double score = item.second;
            pq.push({score, docId});
        }

        vector<pair<string, double>> results;

        while (!pq.empty() && static_cast<int>(results.size()) < topK) {
            double score = pq.top().first;
            int docId = pq.top().second;
            pq.pop();

            results.push_back({documents[docId], score});
        }

        return results;
    }

    int getDocumentCount() const {
        return static_cast<int>(documents.size());
    }

    int getVocabularySize() const {
        return static_cast<int>(invertedIndex.size());
    }

    double getAverageDocumentLength() const {
        return averageDocumentLength;
    }
};

int getDefaultThreadCount() {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);

    int cores = static_cast<int>(sysinfo.dwNumberOfProcessors);

    if (cores <= 0) {
        return 2;
    }

    return cores;
}

#ifndef UNIT_TEST
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: minisearchx.exe <folder_path> [thread_count] [top_k] [ranking_mode]" << endl;
        cout << "ranking_mode: tfidf or bm25" << endl;
        return 1;
    }

    string folderPath = argv[1];

    int threadCount = getDefaultThreadCount();
    int topK = 5;
    string rankingMode = "tfidf";

    if (argc >= 3) {
        threadCount = atoi(argv[2]);
    }

    if (argc >= 4) {
        topK = atoi(argv[3]);

        if (topK <= 0) {
            topK = 5;
        }
    }

    if (argc >= 5) {
        rankingMode = argv[4];

        if (rankingMode != "tfidf" && rankingMode != "bm25") {
            rankingMode = "tfidf";
        }
    }

    SearchEngine engine;

    auto start = chrono::high_resolution_clock::now();
    int threadsUsed = engine.indexFolder(folderPath, threadCount);
    auto end = chrono::high_resolution_clock::now();

    auto indexingTime = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    cout << "MiniSearchX v3 started successfully." << endl;
    cout << "Indexed " << engine.getDocumentCount() << " documents in "
         << indexingTime << " ms using " << threadsUsed << " threads." << endl;
    cout << "Vocabulary size: " << engine.getVocabularySize() << " unique terms." << endl;
    cout << "Average document length: " << engine.getAverageDocumentLength() << " tokens." << endl;
    cout << "Top-K results: " << topK << endl;
    cout << "Ranking mode: " << rankingMode << endl;
    cout << "Use quotes for phrase search. Example: \"machine learning\"" << endl;

    string query;

    while (true) {
        cout << "\nsearch> ";
        getline(cin, query);

        if (query == "exit") {
            cout << "Exiting MiniSearchX." << endl;
            break;
        }

        auto queryStart = chrono::high_resolution_clock::now();
        vector<pair<string, double>> results = engine.search(query, topK, rankingMode);
        auto queryEnd = chrono::high_resolution_clock::now();

        auto queryTime = chrono::duration_cast<chrono::microseconds>(queryEnd - queryStart).count();

        if (results.empty()) {
            cout << "No results found." << endl;
        } else {
            cout << "Top results:" << endl;

            for (int i = 0; i < static_cast<int>(results.size()); i++) {
                cout << i + 1 << ". " << results[i].first
                     << " | score: " << results[i].second << endl;
            }
        }

        cout << "Query latency: " << queryTime << " microseconds." << endl;
    }

    return 0;
}
#endif