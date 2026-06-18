# Compiles and runs MiniSearchX unit tests

Write-Output "Compiling MiniSearchX unit tests..."

g++ -std=c++11 tests/test_minisearchx.cpp -o test_minisearchx

if ($LASTEXITCODE -ne 0) {
    Write-Output "Test compilation failed."
    exit 1
}

Write-Output "Running tests..."

.\test_minisearchx.exe

if ($LASTEXITCODE -ne 0) {
    Write-Output "Some tests failed."
    exit 1
}

Write-Output "All tests passed successfully."