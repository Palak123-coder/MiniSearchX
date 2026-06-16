# Generates benchmark_data folder with 1000 sample documents for MiniSearchX

if (Test-Path "benchmark_data") {
    Remove-Item -Recurse -Force "benchmark_data"
}

mkdir benchmark_data

for ($i = 1; $i -le 1000; $i++) {
    "document $i search engine algorithms data structures multithreading synchronization mutex ranking tfidf google software engineering distributed systems performance indexing query processing machine learning artificial intelligence operating systems database networking backend api reliability debugging scalability " * 5 | Out-File -FilePath "benchmark_data\doc$i.txt" -Encoding utf8
}

Write-Output "Generated 1000 benchmark documents in benchmark_data/"