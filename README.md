# Cruxfilemond

Cruxfilemond is a daemon process that monitors files and directories for changes using fanotify API. When a modification is detected, Cruxfilemond logs the event.
It provides:

- Configuration-based monitoring: Users can specify files and directories to watch in a configuration file.
- Process information: Gathers process information for file accesses.
- Logging (TODO): Plans to log file access events for further analysis.
- Single instance control: Ensures only one instance of the daemon runs at a time.

## Usage
```Bash
cruxfilemond [options]
```
### Options:

-c <config_file>: Specify the configuration file path.
-d: Run in foreground for debugging.

## How it Works

*Loads configuration:* Reads the configuration file and builds a list of paths to monitor.
*Creates fanotify instance:* Initializes a fanotify file descriptor with appropriate flags.
*Adds watches:* Adds watches for each configured path using fanotify_mark.
*Event loop:* Continuously reads events from the fanotify file descriptor.
*Event handling:* Processes events, retrieves process information, and optionally logs events.

## Dependencies
- Linux kernel with fanotify support
- Standard C libraries

## Signal Handling

Cruxfilemond can handle specific signals, including:

- **SIGHUP:** This signal triggers Cruxfilemond to reread its configuration file and potentially reload the monitored files/directories.
- **SIGTERM & SIGINT :** This signal instructs Cruxfilemond to terminate gracefully, cleaning up resources and exiting the process.
- **SIGUSR1:** Upon receiving this signal, Cruxfilemond dumps its log file to the standard output (stdout).
