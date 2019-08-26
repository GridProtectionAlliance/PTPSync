# PTPSync
Precision Time Protocol Synchronization Service for Windows

The [Precision Time Protocol](https://en.wikipedia.org/wiki/Precision_Time_Protocol) (PTP) defined in IEEE 1588 is a protocol used to synchronize clocks over a network.  It represents a major improvement over [NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol). On a local area network, it can achieve clock synchnoization in the sub-microsecond range, making it ideal for Windows-hosted measurement systems like the openPDC and substationSBG. MS Windows Server 2019 supports PTP as does post 2018 releases of Windows 10.

For other Windows operating environments, PTPSync is a windows service that can be used to synchronize the operating system clock to within 1 microsecond on most local networks.
