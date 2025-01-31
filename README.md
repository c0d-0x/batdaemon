# Cruxfilemond: Linux File System Monitoring Daemon
![Version](https://img.shields.io/badge/version-1.0.0-blue.svg) ![License](https://img.shields.io/badge/license-MIT-green) ![Status](https://img.shields.io/badge/status-active-brightgreen)

Cruxfilemond is a lightweight file system monitoring daemon built for Linux using the fanotify API. It tracks file access and modification events within specified directories, providing users with detailed logs of filesystem activity in real time.

## Features

Monitors file access and modification events in real time using fanotify.
Supports logging of basic file events (read, write) with timestamps.
Gracefully handles signals like SIGHUP (reload configuration) and SIGTERM (terminate daemon).
Simple configuration via a `config.h` header

## Upcoming Features

Future releases aim to add:

Support for nested directory monitoring.

- Enhanced logging with user-defined log formats and event types.
- Config file monitoring and reloading on the fly.

## Table of Contents

- Installation
- Usage
- Configuration
- Logging
- Signals
- Performance
- Contributing
- License

## Installation

Clone the repository:

```bash
git clone https://github.com/c0d-0x/cruxfilemond.git
```

### Build the project:

```bash
cd cruxfilemond
mkdir bin
mkdir .config ## for configuration
make
```

### Run the daemon with required permissions (root access required for fanotify):
- Usage: add a dir or a file to be watch in `cruxfilemond/.config/cf.conf` file
```bash
sudo ./cruxfilemond -d # to run in debug, interactive mode
```
The daemon will log all file access and modification events to the console or log file (if specified).

## Command-Line Options
-d : Run as a daemon in the background.<br>
-v : Verbose mode for additional logging details.<br>
Example:

```bash
sudo ./bin/cruxfilemond -d
```

## Configuration

This is done in the `./.config/cf.conf`


## Logging

Cruxfilemond logs events such as file accesses and modifications. The log output includes:

- Timestamp: When the event occurred.
- Event Type: File read or write - [ACCESS or MODIFIED].
- File Path: Full path of the affected file.
- Process Info: PID and name of the process accessing the file.

```json
{
"date":"Fri Jan 31 16:45:17 2025",
file":"/home/c0d_0x/Documents/pwn.college/workspace/filemond/src/inotify.c",
"process":"cat",
"event":"FILE ACCESSED",
"state":"(running)",
"umask":"0022",
"username":"c0d_0x"
}
```
- logs in json format.
## Signals

Cruxfilemond responds to the following signals:

- SIGTERM: Gracefully shuts down the daemon.

## To reload the daemon without restarting:

```bash
sudo ./bin/cruxfilemond_ipc -u # This is a wrapper to kill that sends SIGHUP, SIGTERM, and dumps cruxfilemond log_file
```

## How it Works

- **Loads configuration:** Reads the configuration file and builds a list of paths to monitor.
- **Creates fanotify instance:** Initializes a fanotify file descriptor with appropriate flags.
- **Adds watches:** Adds watches for each configured path using fanotify_mark.
- **Event loop:** Continuously reads events from the fanotify file descriptor.
- **Event handling:** Processes events, retrieves process information, and optionally logs events.

## Dependencies

- Linux kernel with fanotify support
- poll library
- Standard C libraries

## Performance

Cruxfilemond is optimized for lightweight monitoring of directories. For larger systems with many files, the following performance improvements are planned:

- Recursive directory monitoring to track nested directories.
- Batch event processing to reduce resource consumption under high load.
- Contributions

## Contributions are welcome! If you’d like to help improve Cruxfilemond, consider working on the following areas:

- Adding recursive directory monitoring.
- Implementing support for different file event types (creation, deletion, permission changes).
- Optimizing resource usage through epoll or multi-threading.
- Enhancing logging to support custom formats and integration with syslog or SIEM systems.
- Feel free to fork the repository, submit issues, or create pull requests.

## Future Work

Here’s a glimpse of upcoming features and enhancements:

- Nested directory monitoring.
- Configurable logging output.
- Memory and resource optimization.
- Web-based dashboard for real-time monitoring.

## Contact

For any questions, feel free to reach out through the repository's issues page or email: c0d-0x@proton.me
### NB: This project is currently under development.
