# Cruxfilemond

Cruxfilemond is a daemon process that monitors files and directories for changes using fanotify API. When a modification is detected, Cruxfilemond logs the event.

## How it Works

1. **Check for Existing Instance:** It starts by verifying if another instance of Cruxfilemond is already running. This is achieved by checking for the presence of a lock file.
   - If no lock file exists, Cruxfilemond creates one and writes its process ID (PID) to the file. This establishes itself as the running instance and prevents the creation of multiples.
   - If a lock file is already present, Cruxfilemond exits to avoid conflicts.
2. **Configuration and Logging:** Cruxfilemond reads its configuration file, which contains file paths to monitor and potentially logging preferences.
3. **File Monitoring:** Cruxfilemond enters an infinite loop where it continuously monitors the configured files and directories for changes.
4. **Event Handling:** Upon detecting a file access or modification, Cruxfilemond logs the event to a designated log file.

## Signal Handling

Cruxfilemond can handle specific signals, including:

- **SIGHUP:** This signal triggers Cruxfilemond to reread its configuration file and potentially reload the monitored files/directories.
- **SIGTERM:** This signal instructs Cruxfilemond to terminate gracefully, cleaning up resources and exiting the process.
- **SIGUSR1:** Upon receiving this signal, Cruxfilemond dumps its log file to the standard output (stdout).
