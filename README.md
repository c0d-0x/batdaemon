# Cruxfilemond: Linux File System Monitoring Daemon
![Version](https://img.shields.io/badge/version-1.0-blue.svg) ![License](https://img.shields.io/badge/license-MIT-green) ![Status](https://img.shields.io/badge/status-active-brightgreen)

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
make
```

### Run the daemon with required permissions (root access required for fanotify):

```bash
sudo ./cruxfilemond /path/to/directory
```

Usage
To start monitoring a directory for file access and modification events:

```bash
sudo ./cruxfilemond /path/to/directory
```
The daemon will log all file access and modification events to the console or log file (if specified).

## Command-Line Options
-d : Run as a daemon in the background.
-v : Verbose mode for additional logging details.
Example:

```bash
sudo ./bin/cruxfilemond -d
```

## Configuration

This is done in the `./src/config.h`

```c

/* Replace with your USERNAME.
 *  libnotify uses D-Bus to communicate with the user's desktop session, which typically runs under a non-root user.
 *  To send notifications to the active user's desktop, we can set the DISPLAY and DBUS_SESSION_BUS_ADDRESS
 *  environment variables to match those of the active user. This allows the root process to interact with the
 *  active user's notification system by effectively resetting the environment to the user's session.
 ***/


#define USERNAME "username"
#define CONFIG_FILE "cf.config" /* Files or dirs do be watched in here- No spaces*/
#define LOG_FILE "cf.log"
#define LOCK_FILE "cf.lock"
#define CF_HOME_DIR


```

```> [!CAUTION]
+ The DISPLAY environment variable specifies the X display server where graphical applications should render their output.
+ The DBUS_SESSION_BUS_ADDRESS variable defines the address of the D-Bus session bus, which allows processes to communicate with each other within the user session.
```

## Logging

Cruxfilemond logs events such as file accesses and modifications. The log output includes:

- Timestamp: When the event occurred.
- Event Type: File read or write - [ACCESS or MODIFIED].
- File Path: Full path of the affected file.
- Process Info: PID and name of the process accessing the file.

## Custom Log Formats (Future)

Enhanced logging with additional information such as:

- File hash for integrity checks.
- Log Rotation (Future)
- Option for log rotation to avoid large log file growth.

## Signals

Cruxfilemond responds to the following signals:

- SIGHUP: Reloads the configuration. Planned for future releases to dynamically add or remove directories from the watchlist.
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
- libnotify

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
