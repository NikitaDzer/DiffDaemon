# DiffDaemon - track your changes

## Description
DiffDaemon is a linux daemon that save file system changes and restore previous states. <br>
Incremental backups are saved until the OS is restarted.

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
	-h         Print help message.
	-d         Use DD as daemon.
	-i         Use DD in the interactive mode.
	-p <pid>   Specify process <pid> to track its cwd (default = DD pid).

~/DiffDaemon>./build/ddaemon -d                         # Run DD as daemon.
~/DiffDaemon>echo "It is a me,\n" >> mario.txt          # Create file with the initial text.
~/DiffDaemon>echo "Mario!" >> mario.txt                 # Change file content.
~/DiffDaemon>echo "diff mario.txt 1" > /tmp/dd_input     # Make diff with the previous state.
~/DiffDaemon>cat /tmp/dd_output                         # Print diff.
--- /tmp/dd_tmpfile
+++ /home/nikitos/DiffDaemon/mario.txt
@@ -1 +1,2 @@
 It is a me,\n
+Mario!
~/DiffDaemon>echo "close" > /tmp/dd_input # Shutdwon daemon.
```

## License
[MIT](https://choosealicense.com/licenses/mit/)
