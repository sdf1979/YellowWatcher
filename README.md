# YellowWatcher
On-line monitoring of managed locks and server calls of a 1C cluster (windows service BETA, expanding the functionality of the "Quality Manager Central" configuration)
Windows service for expanding the functionality of performance counters in the "Quality Manager Central" configuration. Collects and aggregates information from the technological log about managed locks (TLOCK, TDEADLOCK, TTIMEOUT), as well as server calls (CALL in the context of p: processName for rphost processes and in the context of IName for ragent and rmngr processes). Aggregated information is sent every minute via http to the QMC configuration and is presented there in the form of performance counters.

1. Installation:
  1.1 Configure the collection of the technological log for monitoring managed locks and server calls (an example of settings is the logcfg.xml file). Because monitoring requires only operational events, the duration of the collection of tAs is limited to one hour. The directory for monitoring must be separate and contain only those settings that are in the proposed archive file
  1.2 To get help in the console, run YellowWatcher.exe --help or YellowWatcher -H
  1.3 To install the service in the console, run YellowWatcher.exe --mode = install or YellowWatcher.exe -Minstall

2. Setting:
When installing the service, the file "settings.txt" will be created in the directory where the "YellowWatcher.exe" file is located, in which you must specify the settings required for the service to work.
  
  Setting example:  

  host = server1c  
  http_host = web_server  
  http_port = 80  
  http_target = /QMC/ws/InputStatistics.1cws  
  http_login = Incident  
  http_password = Incident  
  path_monitoring = C:\LOGS_MONITORING

  host - the name of the current server, used in forming the name of the performance counter in the "Quality Manager Central"
  http_host - the name of the web server on which the "Quality Manager Central" information base is published
  http_port - the publishing port of the "Quality Manager Central"
  http_target - the path to the "InputStatistics" IB web service CCC, http_login - username of infobase QMC (the user must have the roles "InputStatistics", "InputIncidentTickets")
  http-password - user password of infobase QMC
  path_monitoring - data directory of 1C technological log (must match the directory specified in logcfg .xml)

3. Launch:
Running as a service is done through the standard Service Management Console. To run as a console application, you need to execute "YellowWatcher.exe --mode = console" or "YellowWatcher.exe -Mconsole". When the application is running, a minimal log file "YellowWatcher.log" is kept by default, located in the directory of the executable file.

4. Removal:
To uninstall the service, run the command "YellowWatcher.exe --mode = uninstall" or "YellowWatcher.exe -Muninstall" in the console.
