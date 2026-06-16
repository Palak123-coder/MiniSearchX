# MiniSearchX

MiniSearchX is a multithreaded C++ search engine that indexes local text documents and returns ranked search results using an inverted index and TF-IDF scoring.

The project demonstrates core software engineering concepts including data structures, algorithms, file processing, multithreading, synchronization, ranking, and performance benchmarking.

## Features

* Indexes `.txt` files from a local folder
* Tokenizes and normalizes text
* Builds an inverted index using hash maps
* Supports multi-word search queries
* Ranks documents using TF-IDF scoring
* Returns top search results using a priority queue
* Supports multithreaded document indexing
* Uses Windows synchronization primitives to protect shared index updates
* Measures indexing time and query latency

## Tech Stack

* C++
* STL
* Hash Maps
* Priority Queue
* File I/O
* TF-IDF Ranking
* Windows Threads
* Critical Sections for Synchronization

## How It Works

MiniSearchX reads all `.txt` files from a folder, tokenizes the content, and builds an inverted index.

```text
term -> document ID -> frequency
```

Example:

```text
systems -> doc1, doc3
learning -> doc2
threads -> doc3
```

When a user enters a query, MiniSearchX calculates TF-IDF scores for matching documents and returns the highest-ranked results.

## Multithreading Design

MiniSearchX divides the document list into chunks and assigns each chunk to a separate worker thread.

Each thread:

1. Reads assigned documents
2. Tokenizes text
3. Builds a local term-frequency map
4. Updates the shared inverted index inside a synchronized critical section

Only the shared index update is locked. File reading and tokenization happen outside the lock to improve parallelism.

## Benchmark Results

Benchmark dataset: 1,000 generated text documents
Vocabulary size: 1,032 unique terms

| Threads   | Indexing Time |
| --------- | ------------: |
| 1 thread  |        278 ms |
| 4 threads |         97 ms |
| 8 threads |         65 ms |

Using 8 threads reduced indexing time from 278 ms to 65 ms, achieving approximately 76.6% faster indexing compared to the single-threaded run.

## How to Compile

```bash
g++ -std=c++11 src/main.cpp -o minisearchx
```

## How to Run

```bash
.\minisearchx.exe data 4
```

For benchmark data:

```bash
.\minisearchx.exe benchmark_data 8
```

## Example Queries

```text
google systems
machine learning
threads synchronization
```

To exit:

```text
exit
```

## Sample Output

```text
MiniSearchX v2 started successfully.
Indexed 1000 documents in 65 ms using 8 threads.
Vocabulary size: 1032 unique terms.
```

## Future Improvements

* Add BM25 ranking
* Add phrase search
* Add AND/OR query support
* Add cross-platform filesystem support
* Add unit tests
* Refactor into modular header and source files
