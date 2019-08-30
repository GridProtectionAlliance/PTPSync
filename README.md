# PTPSync

<img align="right" src="https://github.com/GridProtectionAlliance/PTPSync/raw/master/Source/Documentation/Images/PTPSync-logo.png" width="150px" height="150px" />

Precision Time Protocol Synchronization Service for Windows

The [Precision Time Protocol](https://en.wikipedia.org/wiki/Precision_Time_Protocol) (PTP) defined in IEEE 1588 is a protocol used to synchronize clocks over a network.  It represents a major improvement over [NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol). On a local area network, it can achieve clock synchronization in the sub-microsecond range, making it ideal for time-sensitive Windows-hosted measurement systems like the [openPDC](https://github.com/GridProtectionAlliance/openPDC) and [substationSBG](https://github.com/GridProtectionAlliance/substationSBG).
 
Newer versions of Windows, specifically Windows Server 2019 and post 2018 releases of Windows 10, already support PTP. You can validate that the version of Windows you are using supports PTP by looking for the provider DLL:
```
%windir%\system32\ptpprov.dll
```
 
If the PTP provider DLL exists on your system, [see official instructions for its configuration](https://github.com/microsoft/W32Time/tree/master/Precision%20Time%20Protocol).
 
For all other Windows operating environments, PTPSync is a Windows service that can be used to synchronize the operating system clock to within a few microseconds of precision on most local networks (see [Windows Time Precision](#windows-time-precision)) when used with a [clock that supports PTP](#ptp-clock-vendors). 

_Need help?_ Search for answers or ask a question on the [PTPSync discussions board](https://discussions.gridprotectionalliance.org/c/gpa-products/Precision-Time-Protocol-Synchronization-Service-for-Windows).

## Installation
 
1) Download installer zip file `PTPSync.Installs.zip` from the [Releases](https://github.com/GridProtectionAlliance/PTPSync/releases) page.
2) Open zip file and double click on `PTPSyncSetup.msi`.
3) Next through first page, accept [MIT license](LICENSE) on next page, and click next again.
4) On the `Custom Setup` page you can change the target install location if desired.
5) It is recommended to keep all production options listed on the `Custom Setup` page then click next.
6) On the `Service Account` page the default account for the PTPSync Windows service is `LocalSystem`. Should you choose another user for the service, the user will at least need enough rights to change the local clock and access to the network. Click next to continue.
7) On the `Company Information` page enter the company name that is hosting the PTPSync service and a short acronym, then click next.
8) Click install. When the installation completes, make sure "Launch Configuration Setup Utility" is checked and click finish.
9) The configuration setup utility for the service will now run. Click next from the first page.
> Note that the following steps assume this is a new installation, not an upgrade:
 
10) Select "I want to set up a new configuration." and click next.
11) Select "Database (suggested)" and click next.
12) On the `Set up a database` page, SQLite is the typical option. Once desired database is selected, click next.
> Note that the database is only used to hold configuration information for the PTPSync service. When using database other than SQLite, see [additional configuration steps](https://github.com/GridProtectionAlliance/openHistorian/wiki/Using-the-Configuration-Setup-Utility#setting-up-a-new-configuration).
 
13) The `Set up a SQLite database` page defines the desired location for the SQLite database file, it defaults to the `ConfigurationCache` sub-folder of the PTPSync service installation folder. For non-default locations, service user will require read/write access. When the desired database location has been selected, click next.
14) On the `Define User Account Credentials` page, select the initial administrative user for the PTPSync service - installing user is the default. Click next when desired admin user has been selected. Additional users and groups can be configured later using the PTPSync Manager application.
15) Make sure "PTPSync Windows service" and "PTPSync Manager (local application)" are both checked and click next.
16) On the `Setup is ready` page, click next.
17) If the setup steps on the `Setup in progress` page succeed, click next.
18) Make sure "Start the PTPSync Service" and "Start the PTPSync Manager" are checked and click "Finish"
19) Installation is now complete, and the Windows PTPSync service will now be running.
20) The system will have chosen a local network interface to bind to for listening to PTP synchronization packets, but this may not be the desired interface. Continue with [Configuration](#configuration) steps below.
 
> You can leave current time synchronization, e.g., NTP, in place as a backup time sync in case the PTP clock broadcast or synchronization service stops.

## Configuration
 
To configure PTPSync service, run the PTPSync Manager application, which can be found in the Windows start menu.
> If you are continuing directly from installation steps above, manager application will already by runing.

When the application is running, click the `Configure PTPd` button on the home page:
 
![PTPSync Manager Home Screen](https://github.com/GridProtectionAlliance/PTPSync/raw/master/Source/Documentation/Images/PTPSyncManager_Home.PNG)

On the configuration screen, make sure `PTPD!PROCESS` is selected in the adapter list at the bottom of the page, then click on the `Arguments` parameter in the `Connection String` section:

![PTPSync Manager Config Screen](https://github.com/GridProtectionAlliance/PTPSync/raw/master/Source/Documentation/Images/PTPSyncManager_Config.PNG)

These arguments define the options to use for the `ptpd.exe` application instance that actually performs the time synchronization, see [PTP Options](#ptp-options) for full set.

The most important parameter here is the `-b` option that defines the GUID for the network interface to use for PTP traffic. The available network interface GUIDs for the local machine are enumerated in the `NetworkInterfaces.txt` file which can be found in the PTPSync installation folder, typically `C:\Program Files\PTPSync\NetworkInterfaces.txt`. This file is updated when the PTPSync Windows service is started. The contents of the file will be similar to the following:
```
Local Network Interfaces:

Ethernet 3 - Intel(R) Ethernet Connection (2) I218-V: {ED9D9DAA-C2AC-40DA-8AC6-218A769F0A44}
WiFi - Realtek 8822BE Wireless LAN 802.11ac PCI-E NIC: {67C2E6EC-3795-4B89-9A69-02E147503CD2}

```
To validate that the defined `-b` option is defined correctly, find the interface in this list that will receive traffic for the incoming PTP messages. At the end of the line that defines the proper interface is the network interface GUID, copy this value including the curly braces. Paste the GUID value, replacing existing one, after the `-b` option with a space in between and click the `Save` button.

> Always click the `Save` button after making any changes as this updates the record in the configuration database.

A helpful application to use while configuring the `ptpd` engine is the PTPSync Console application. Like the manager, this application can be found in the Windows start menu. Run this program now to continue with configuration steps:

![PTPSync Remote Console Screen](https://github.com/GridProtectionAlliance/PTPSync/raw/master/Source/Documentation/Images/PTPSyncManager_Console.PNG)

From the console application you can type `list /a` (or `ls /a`) to show the available adapters. For example, if the `PTPD!PROCESS` adapter has an ID of 2, you can then enter `ls 2` (or `ls PTPD!PROCESS`) to get detailed adapter status.

> If the console is noisy with status information you can enter the `quiet` command followed by enter to pause feedback, note that your keyboard input is being accepted even if it is interrupted by messages scrolling by, just keep typing. To restore normal status mode use the `resume` command. Type `help` for other available commands.

Making sure that console is still visible off to the side, pull the PTPSync Manager application back up and into focus. With the `PTPD!PROCESS` adapter still selected in the adapter list at the bottom of the page, click the `Initialize` button and confirm the initialize action by clicking `Yes`. This operation will restart the `ptpd.exe` application applying any updated arguments, e.g., the new `-b` argument as updated in the connection string.

> Make sure any changes are saved before initializing, the initialization process reads from the database configuration. You can also reinitialize the adapter from the PTPSync Console application using the `initialize PTPD!PROCESS` command.

If the PTP clock is sending broadcast messages, the local clock should now be synchronizing.

You can now adjust other parameters, such as the verbosity of the feedback. The `-V` option, which is on initially, puts the `ptpd` engine in highly verbose mode which includes debug messages. This option is good for validating that things are working, but should be turned off (by removing the option) for production. If you want a quick, less verbose, check of things in production you can use the `-d` option which will report each clock synchronization, however, this option can be a bit noisy too.

> Note that all feedback is logged locally to the `StatusLog.txt` file which can be found in the installation folder. The log file size is automatically curtailed per settings defined in the `PTPSync.exe.config` file. You can use the XML Configuration Editor, found in the PTPSync folder in the Windows start menu, to adjust any needed rarely changed configuration settings.

The other option that is applied by default is `-g`, this puts the `ptpd` engine in slave mode. Removing this option will put the engine in peer-to-peer mode which will allow the engine to be a peer time provider on the local network.

## PTP Options

The PTPSync Windows service hosts and runs the `ptpd.exe` executable application. These are the command line options for the application:
```
Usage:  ptpd.exe [OPTION]

ptpd runs on UDP/IP, P2P mode by default

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
---

_The `ptpd.exe` synchronization application used in PTPSync is based on a port of the [ptpd](https://github.com/ptpd/ptpd) project developed by Jan Breuer that runs on Windows, see [NOTICE](NOTICE.txt). Updates to the project used by PTPSync can be found in the [local repository](https://github.com/GridProtectionAlliance/PTPSync/tree/master/Source/Applications/ptpd)._

---

#### PTP Clock Vendors

Here are a few vendors that offer PTP hardware clock options:
* [Schweitzer Engineering Laboratories](https://selinc.com)
* [Arbiter Systems](https://www.arbiter.com)
* [EndRun Technologies](https://endruntechnologies.com)
* [Meinberg](https://endruntechnologies.com)
* [Orolia](https://www.orolia.com)
* [Microsemi](https://www.microsemi.com)

_This project and its authors can neither endorse or verify the applicability of any manufacturer's hardware clock._

---

#### Windows Time Precision
 
Windows socket options do not currently support [SO_TIMESTAMPING](https://www.kernel.org/doc/Documentation/networking/timestamping.txt) and the PTP engine compiled for PTPSync was configured so that it would not be dependent upon externally installed libraries, e.g., PCAP. Consequently this may affect the maximum possible time synchronization accuracy available when using this service. It is expected that properly configured deployments will see clock synchronization accuracy commonly within the range of a few microseconds or better, but results will vary based on a variety of environmental conditions. Regardless, an improvement in accuracy as compared to NTP is expected. You can use the [`w32tm /stripchart /computer:<other>`](https://docs.microsoft.com/en-us/windows-server/networking/windows-time-service/windows-time-service-tools-and-settings) command to measure time variance between the current machine and another.
 
> On systems running Windows older than Windows Server 2012 R2 / Windows 8, the API calls used to retrieve local clock time may only pickup time changes every 15 milliseconds, regardless of local clock accuracy.

---

This project was created with the Grid Solutions Framework Time-Series Library [Project Alpha](https://github.com/GridProtectionAlliance/projectalpha)

[![Project Alpha](https://github.com/GridProtectionAlliance/PTPSync/raw/master/Source/Documentation/Images/Project-Alpha-Logo_70.png)](https://github.com/GridProtectionAlliance/projectalpha)
