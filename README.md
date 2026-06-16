# MiniSearchX

MiniSearchX is a multithreaded C++ search engine that indexes local text documents and returns ranked search results using an inverted index and TF-IDF scoring.

This project demonstrates core software engineering concepts including data structures, algorithms, file processing, multithreading, synchronization, ranking, and performance benchmarking.

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
* Includes benchmark results for single-threaded and multithreaded indexing

## Tech Stack

* C++
* STL
* Hash Maps
* Priority Queue
* File I/O
* TF-IDF Ranking
* Windows Threads
* Critical Sections for Synchronization

## Project Structure

```text
MiniSearchX/
│
├── src/
│   └── main.cpp
│
├── data/
│   ├── doc1.txt
│   ├── doc2.txt
│   └── doc3.txt
│
├── scripts/
│   └── generate_benchmark_data.ps1
│
├── screenshots/
│   └── benchmark.png
│
├── README.md
└── .gitignore
```

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

When a user enters a search query, MiniSearchX calculates TF-IDF scores for matching documents and returns the highest-ranked results.

## Inverted Index

The inverted index stores each term with the documents in which it appears.

Instead of scanning every document for every query, the search engine directly checks the index to find matching documents.

This improves search efficiency and demonstrates the use of hash maps for fast lookup.

## TF-IDF Ranking

MiniSearchX ranks documents using TF-IDF scoring.

TF-IDF gives higher importance to terms that appear frequently in a specific document but are less common across the entire document collection.

This allows the search engine to return more relevant results instead of only checking whether a word exists in a document.

## Multithreading Design

MiniSearchX divides the document list into chunks and assigns each chunk to a separate worker thread.

Each thread:

1. Reads assigned documents
2. Tokenizes text
3. Builds a local term-frequency map
4. Updates the shared inverted index inside a synchronized critical section

Only the shared index update is locked. File reading and tokenization happen outside the lock to improve parallelism.

## Synchronization

The shared inverted index is protected using Windows Critical Sections.

This prevents multiple threads from writing to the shared data structure at the same time, avoiding race conditions and maintaining correctness during parallel indexing.

## Benchmark Results

Benchmark dataset: 1,000 generated text documents
Vocabulary size: 1,032 unique terms

| Threads   | Indexing Time |
| --------- | ------------: |
| 1 thread  |        278 ms |
| 4 threads |         97 ms |
| 8 threads |         65 ms |

Using 8 threads reduced indexing time from 278 ms to 65 ms, achieving approximately 76.6% faster indexing compared to the single-threaded run.

![Benchmark Output](screenshots/benchmark.png)

## Reproducing the Benchmark

Generate the benchmark dataset:

```powershell
.\scripts\generate_benchmark_data.ps1
```

If PowerShell blocks script execution, run:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\generate_benchmark_data.ps1
```

Compile the project:

```powershell
g++ -std=c++11 src/main.cpp -o minisearchx
```

Run with 1 thread:

```powershell
.\minisearchx.exe benchmark_data 1
```

Run with 4 threads:

```powershell
.\minisearchx.exe benchmark_data 4
```

Run with 8 threads:

```powershell
.\minisearchx.exe benchmark_data 8
```

## How to Compile

```powershell
g++ -std=c++11 src/main.cpp -o minisearchx
```

## How to Run

Run on the sample data folder:

```powershell
.\minisearchx.exe data 4
```

Run on the benchmark data folder:

```powershell
.\minisearchx.exe benchmark_data 8
```

## Example Queries

```text
google systems
machine learning
threads synchronization
```

To exit the search prompt:

```text
exit
```

## Sample Output

```text
MiniSearchX v2 started successfully.
Indexed 1000 documents in 65 ms using 8 threads.
Vocabulary size: 1032 unique terms.

search> google systems
Top results:
1. benchmark_data\doc999.txt | score: 0.0294118
2. benchmark_data\doc998.txt | score: 0.0294118
3. benchmark_data\doc997.txt | score: 0.0294118
Query latency: 0 microseconds.
```

## Key Concepts Demonstrated

* Inverted indexing
* TF-IDF ranking
* Top-K retrieval
* Hash maps
* Priority queues
* File processing
* Multithreaded indexing
* Synchronization using critical sections
* Benchmarking and performance comparison

## Current Limitations

* Windows-specific implementation due to use of Windows API and Critical Sections
* Supports `.txt` files only
* Does not yet support phrase search
* Does not yet support BM25 ranking
* No unit tests added yet

## Future Improvements

* Add BM25 ranking
* Add phrase search
* Add AND/OR query support
* Add cross-platform filesystem support
* Add unit tests
* Refactor into modular header and source files
* Add command-line options for top-K results
* Add larger and more realistic benchmark datasets
