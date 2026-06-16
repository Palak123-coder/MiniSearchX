#include <algorithm>
#include <chrono>
#include <cmath>
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
};

class SearchEngine {
private:
    vector<string> documents;

    // term -> docId -> frequency
    unordered_map<string, unordered_map<int, int>> invertedIndex;

    // docId -> total number of words
    unordered_map<int, int> documentLengths;

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
        return files;
    }

public:
    void indexFolder(const string& folderPath) {
        vector<string> files = getTextFilesFromFolder(folderPath);

        int docId = 0;

        for (const string& filePath : files) {
            indexDocument(filePath, docId);
            docId++;
        }
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

        documents.push_back(filePath);
        documentLengths[docId] = static_cast<int>(tokens.size());

        for (const string& token : tokens) {
            invertedIndex[token][docId]++;
        }
    }

    vector<pair<string, double>> search(const string& query, int topK = 5) {
        vector<string> queryTokens = Tokenizer::tokenize(query);

        unordered_map<int, double> docScores;
        int totalDocs = static_cast<int>(documents.size());

        for (const string& term : queryTokens) {
            if (invertedIndex.find(term) == invertedIndex.end()) {
                continue;
            }

            int docsWithTerm = static_cast<int>(invertedIndex[term].size());

            double idf = log((1.0 + totalDocs) / (1.0 + docsWithTerm)) + 1.0;

            for (const auto& item : invertedIndex[term]) {
                int docId = item.first;
                int termFrequency = item.second;

                double tf = static_cast<double>(termFrequency) / documentLengths[docId];
                double score = tf * idf;

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
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: minisearchx.exe <folder_path>" << endl;
        return 1;
    }

    string folderPath = argv[1];

    SearchEngine engine;

    auto start = chrono::high_resolution_clock::now();
    engine.indexFolder(folderPath);
    auto end = chrono::high_resolution_clock::now();

    auto indexingTime = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    cout << "MiniSearchX started successfully." << endl;
    cout << "Indexed " << engine.getDocumentCount() << " documents in "
         << indexingTime << " ms." << endl;
    cout << "Vocabulary size: " << engine.getVocabularySize() << " unique terms." << endl;

    string query;

    while (true) {
        cout << "\nsearch> ";
        getline(cin, query);

        if (query == "exit") {
            cout << "Exiting MiniSearchX." << endl;
            break;
        }

        auto queryStart = chrono::high_resolution_clock::now();
        vector<pair<string, double>> results = engine.search(query);
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