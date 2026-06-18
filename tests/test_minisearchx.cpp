#define UNIT_TEST

#include "../src/main.cpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int testsPassed = 0;
int testsFailed = 0;

void check(bool condition, const string& testName) {
    if (condition) {
        cout << "[PASS] " << testName << endl;
        testsPassed++;
    } else {
        cout << "[FAIL] " << testName << endl;
        testsFailed++;
    }
}

void createTestData() {
    system("if exist test_data rmdir /s /q test_data");
    system("mkdir test_data");

    ofstream doc1("test_data\\doc1.txt");
    doc1 << "Google builds search systems using algorithms and software engineering.";
    doc1.close();

    ofstream doc2("test_data\\doc2.txt");
    doc2 << "Machine learning helps build intelligent applications.";
    doc2.close();

    ofstream doc3("test_data\\doc3.txt");
    doc3 << "Operating systems use threads synchronization and mutexes.";
    doc3.close();
}

void testTokenizer() {
    vector<string> tokens = Tokenizer::tokenize("Google, SYSTEMS! 123");

    check(tokens.size() == 3, "Tokenizer returns correct number of tokens");
    check(tokens[0] == "google", "Tokenizer converts text to lowercase");
    check(tokens[1] == "systems", "Tokenizer removes punctuation");
    check(tokens[2] == "123", "Tokenizer keeps numeric tokens");
}

void testPhraseDetectionHelpers() {
    check(Tokenizer::isPhraseQuery("\"machine learning\"") == true, "Detects quoted phrase query");
    check(Tokenizer::isPhraseQuery("machine learning") == false, "Detects normal non-phrase query");

    string trimmed = Tokenizer::trimQuotes("\"machine learning\"");
    check(trimmed == "machine learning", "Removes quotes from phrase query");
}

void testIndexing() {
    SearchEngine engine;
    int threadsUsed = engine.indexFolder("test_data", 2);

    check(engine.getDocumentCount() == 3, "Indexes all test documents");
    check(engine.getVocabularySize() > 0, "Builds non-empty vocabulary");
    check(threadsUsed == 2, "Uses requested worker threads when possible");
}

void testTFIDFSearch() {
    SearchEngine engine;
    engine.indexFolder("test_data", 2);

    vector<pair<string, double>> results = engine.search("google systems", 3, "tfidf");

    check(!results.empty(), "TF-IDF search returns results");
    check(results[0].first.find("doc1.txt") != string::npos, "TF-IDF ranks most relevant document first");
}

void testPhraseSearch() {
    SearchEngine engine;
    engine.indexFolder("test_data", 2);

    vector<pair<string, double>> results = engine.search("\"machine learning\"", 3, "tfidf");

    check(!results.empty(), "Phrase search returns results");
    check(results[0].first.find("doc2.txt") != string::npos, "Phrase search finds exact phrase document");
}

void testBM25SearchAndTopK() {
    SearchEngine engine;
    engine.indexFolder("test_data", 2);

    vector<pair<string, double>> results = engine.search("threads synchronization", 2, "bm25");

    check(!results.empty(), "BM25 search returns results");
    check(results.size() <= 2, "Top-K limit is respected");
    check(results[0].first.find("doc3.txt") != string::npos, "BM25 ranks relevant document first");
}

int main() {
    cout << "Running MiniSearchX unit tests..." << endl;

    createTestData();

    testTokenizer();
    testPhraseDetectionHelpers();
    testIndexing();
    testTFIDFSearch();
    testPhraseSearch();
    testBM25SearchAndTopK();

    cout << endl;
    cout << "Tests passed: " << testsPassed << endl;
    cout << "Tests failed: " << testsFailed << endl;

    system("if exist test_data rmdir /s /q test_data");

    if (testsFailed > 0) {
        return 1;
    }

    return 0;
}