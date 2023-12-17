# DiffDaemon - track your changes

## Description
DiffDaemon is a linux daemon that save file system changes and restore previous states.

## Prerequisites

- Linux OS
- CMake 3.12+

## Installation
* Clone the repository:
```sh
git clone https://github.com/NikitaDzer/DiffDaemon.git
 ```

* Compile the project:
```sh
cmake -B build -D CMAKE_BUILD_TYPE=Debug
make -C build -j4
```

## Usage
```sh
~/DiffDaemon>./build/ddaemon -h
Diff Daemon (or DD)
Options:
	-h         Print help.
	-p <pid>   Specify process's pid to watch.
	-d         Use DD as daemon.
	-i         Use DD in the interactive mode.
~/DiffDaemon>./build/ddaemon -d                         # Run DD as daemon.
~/DiffDaemon>echo "It's a me,\n" >> mario.txt           # Create file with the initial text.
~/DiffDaemon>echo "Mario!" >> mario.txt                 # Change file content.
~/DiffDaemon>echo "diff mario.txt 1" > /tmp/dd_input    # Make diff with the previous (current - 1) state.
~/DiffDaemon>cat /tmp/dd_output                         # Print diff.
--- /tmp/dd_tmpfile
+++ /home/nikitos/DiffDaemon/mario.txt
@@ -1 +1,2 @@
 It's a me,\n
+Mario!
~/DiffDaemon>echo "close" > /tmp/dd_input # Shutdwon daemon.
```

## License
[MIT](https://choosealicense.com/licenses/mit/)
