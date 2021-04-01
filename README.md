# Cat Crawler
A cat picture crawler written in pure C.
Uses openSSL to establish SSL tunnel to get the https running.

# Build
```
make
```
Just that simple.

# The usage
```
./crawler.out
```
The picture it fetch is output to stdout.
File extension is not provided and need to be inspect by `file` command.

# Working Environment / Dependencies
This should only work in Linux. I don't think this can work in macOS X, and had no chance to run on Windows.
You need to install openSSL first inorder to compile and run this program.
