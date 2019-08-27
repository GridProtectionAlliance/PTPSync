# PTPSync
Precision Time Protocol Synchronization Service for Windows
 
The [Precision Time Protocol](https://en.wikipedia.org/wiki/Precision_Time_Protocol) (PTP) defined in IEEE 1588 is a protocol used to synchronize clocks over a network.  It represents a major improvement over [NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol). On a local area network, it can achieve clock synchronization in the sub-microsecond range, making it ideal for time-sensitive Windows-hosted measurement systems like the [openPDC](https://github.com/GridProtectionAlliance/openPDC) and [substationSBG](https://github.com/GridProtectionAlliance/substationSBG).
 
Newer version of Windows, specifically Windows Server 2019 and post 2018 releases of Windows 10, already support PTP. You can validate that the version of Windows you are using supports PTP by looking for the provider DLL:
```
%windir%\system32\ptpprov.dll
```
 
If the PTP provider DLL exists on your system, [see official instructions for its configuration](https://github.com/microsoft/W32Time/tree/master/Precision%20Time%20Protocol).
 
For all other Windows operating environments, PTPSync is a Windows service that can be used to synchronize the operating system clock to within a few microseconds of precision on most local networks (see [Windows Time Precision](#windows-time-precision)).
 
This work is based on a port of [ptpd](https://github.com/ptpd/ptpd) developed by Jan Breuer that runs on Windows, see [NOTICE](NOTICE.txt). Updates to the Windows port of the project used by PTPSync can be found in the [local repository](https://github.com/GridProtectionAlliance/PTPSync/tree/master/Source/Applications/ptpd).
 
#### Windows Time Precision
 
Windows socket options do not currently support [SO_TIMESTAMPING](https://www.kernel.org/doc/Documentation/networking/timestamping.txt) and the PTP engine compiled for PTPSync was configured so that it would not be dependent upon externally installed libraries, e.g., PCAP. Consequently this affects the maximum possible time synchronization accuracy available when using this service. It is expected that properly configured deployments will see clock synchronization accuracy commonly within the range of a few microseconds, but results will vary based on a variety of environmental conditions. Regardless, an improvement in accuracy as compared to NTP is expected. You can use the [`w32tm /stripchart /computer:<other>`](https://docs.microsoft.com/en-us/windows-server/networking/windows-time-service/windows-time-service-tools-and-settings) command to measure time variance between the current machine and another.
 
> On systems running Windows older than Windows Server 2012 R2 / Windows 8, the API calls used to retrieve local clock time may only pickup time changes every 15 milliseconds, regardless of local clock accuracy.

## Installation
 
1) Download installer zip file `PTPSync.Installs.zip` from the [releases](https://github.com/GridProtectionAlliance/PTPSync/releases) page.
2) Open zip file and double click on `PTPSyncSetup.msi`.
3) Next through first page, accept [MIT license](LICENSE) on next page, and click next again.
4) On the `Custom Setup` page you can change the target install location if desired.
5) It is recommended to keep all production options listed on the `Custom Setup` page then click next.
6) On the `Service Account` page the default account for the PTPSync Windows service is `LocalSystem`. Should you choose another user for the service, the user will at least need enough rights to change the local clock and access the network. Click next.
7) On the `Company Information` page enter the company name hosting the PTPSync service and a short acronym, then click next.
8) Click install. When the installation completes, make sure "Launch Configuration Setup Utility" is checked and click finish.
9) The configuration setup utility for the service will now run. Click next from the first page.
> Note that the following steps assume this is a new installation, not an upgrade:
 
10) Select "I want to set up a new configuration." and click next.
11) Select "Database (suggested)" and click next.
12) On the `Set up a database` page, SQLite is the typical option. Once desired database is selected, click next.
> Note that the database is only used to hold configuration information for the PTP service. When using database other than SQLite, see [additional configuration steps](https://github.com/GridProtectionAlliance/openHistorian/wiki/Using-the-Configuration-Setup-Utility#setting-up-a-new-configuration).
 
13) The `Set up an SQLite database` page defines the desired location for the SQLite database file, it defaults to the `ConfigurationCache` sub-folder of the PTPSync service installation folder. When the desired database location has been selected, click next.
14) The `Define User Account Credentials` defines the initial administrative user for the PTPSync service. Click next when desired admin user has been selected. Additional users and groups can be configured later using the PTPSync Manager application.
15) Make sure "PTPSync Windows service" and "PTPSync Manager (local application)" are both checked and click next.
16) On the `Setup is ready` page, click next.
17) If the setup steps on the `Setup in progress` page succeed, click next.
18) Make sure "Start the PTPSync Service" and "Start the PTPSync Manager" are checked and click "Finish"
19) Installation is now complete, and the Windows PTPSync service will now be running.
20) The system will have chosen a local network interface to bind to for listening to PTP synchronization packets, but this may not be the desired interface. Continue with [Configuration](#configuration) steps below.
 
> You can leave current time synchronization, e.g., NTP, in place as a backup time sync in case PTP clock broadcast or service stops.

## Configuration
 
To configure PTP, run the PTPSync Manager application, which can be found in the Windows start menu. When the application starts, click the `Configure PTP` button on the home page:
 
![PTPSync Manager Home Screen](https://raw.githubusercontent.com/GridProtectionAlliance/PTPSync/master/Source/Documentation/PTPSyncMananger_Home.PNG)

On the configuration screen, make sure `PTPD!PROCESS` is selection in the adapter list, then click on the `Arguments` parameter:
 
![PTPSync Manager Config Screen](https://raw.githubusercontent.com/GridProtectionAlliance/PTPSync/master/Source/Documentation/PTPSyncMananger_Config.PNG)

TODO: Finish setup instructions (reference auto-created `NetworkInterfaces.txt` files)


## PTP Options

The PTPSync Windows service hosts and runs the `ptpd.exe` executable application. These are the command line options for the application:
```
Usage:  ptp.exe [OPTION]

ptp runs on UDP/IP, P2P mode by default

-?                show this page

-f FILE           send output to FILE
-T                set multicast time to live
-d                display stats
-D                display stats in .csv format
-P                display each packet
-R                record data about sync packets in a file

-x                do not reset the clock if off by more than one second
-O                do not reset the clock if offset is more than NUMBER nanoseconds
-M                do not accept delay values of more than NUMBER nanoseconds
-t                do not adjust the system clock
-a NUMBER,NUMBER  specify clock servo P and I attenuations
-w NUMBER         specify one way delay filter stiffness

-b {GUID}         bind PTP to network interface GUID
-u ADDRESS        also send uni-cast to ADDRESS
-e                run in ethernet mode (level2)
-h                run in End to End mode
-V                show verbose messages (includes debug)
-l NUMBER,NUMBER  specify inbound, outbound latency in nsec

-o NUMBER         specify current UTC offset
-i NUMBER         specify PTP domain number

-n NUMBER         specify announce interval in 2^NUMBER sec
-y NUMBER         specify sync interval in 2^NUMBER sec
-m NUMBER         specify max number of foreign master records

-g                run as slave only
-v NUMBER         specify system clock allen variance
-r NUMBER         specify system clock accuracy
-s NUMBER         specify system clock class
-p NUMBER         specify priority1 attribute
-q NUMBER         specify priority2 attribute

```
