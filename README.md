# MiniSearchX

MiniSearchX is a C++ search engine that indexes local text documents and returns ranked search results using an inverted index and TF-IDF scoring.

This project was built to understand the core engineering behind search systems, including tokenization, inverted indexing, ranking, hash maps, priority queues, and performance measurement.

## Features

- Indexes `.txt` files from a local folder
- Tokenizes and normalizes text
- Builds an inverted index using hash maps
- Supports multi-word search queries
- Ranks documents using TF-IDF scoring
- Returns top search results using priority queue
- Measures indexing time and query latency

## Tech Stack

- C++
- STL
- Hash Maps
- Priority Queue
- File I/O
- TF-IDF Ranking
- Windows API for file discovery

## How It Works

MiniSearchX reads all text files from a folder and breaks the content into lowercase searchable tokens.

It then builds an inverted index:

```text
term -> document ID -> frequency