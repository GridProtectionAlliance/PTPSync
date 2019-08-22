/*-
 * Copyright (c) 2010-2011 Jan Breuer 
 * Copyright (c) 2009-2011 George V. Neville-Neil, Steven Kreuzer, 
 *                         Martin Burnicki, Gael Mace, Alexandre Van Kempen
 * Copyright (c) 2005-2008 Kendall Correll, Aidan Williams
 *
 * All Rights Reserved
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file   net_dep.c
 * @date   Tue Jun 6 16:17:49 2011
 *
 * @brief  Functions to interact with the network sockets and NIC driver.
 *
 *
 */

#include "../ptpd.h"

/**
 * Shutdown platform dependent network stuff
 *
 * @param netPath
 *
 * @return TRUE if successful
 */
Boolean
netShutdownWSA2(NetPath * netPath)
{
    /* Close sockets */
    if (netPath->eventSock > 0)
        closesocket(netPath->eventSock);
    netPath->eventSock = -1;

    if (netPath->generalSock > 0)
        closesocket(netPath->generalSock);
    netPath->generalSock = -1;

    /* Close all event handles */
    if (netPath->eventHEvent) {
        WSACloseEvent(netPath->eventHEvent);
        netPath->eventHEvent = 0;
    }
    if (netPath->generalHEvent) {
        WSACloseEvent(netPath->generalHEvent);
        netPath->generalHEvent = 0;
    }
    /* Close WinSock */
    WSACleanup();

#if defined(TIMESTAMPING_PCAP)
    /* if using PCAP, close it */
    if(netPath->eventPcap) {
        pcap_close(netPath->eventPcap);
    }
#endif

    return TRUE;
}

/* Find the local network interface */
Boolean
netFindInterfaceWin32(NetPath * netPath, Octet * ifaceName, UInteger8 * communicationTechnology,
    Octet * uuid, struct in_addr * ifaceAddr)
{
    struct sockaddr_in * addr;

    Boolean result = FALSE;
    ULONG outBufLen = 0;
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
    DWORD dwRetVal = 0;

    ULONG family = AF_INET; /* AF_INET, AF_INET6, AF_UNSPEC */
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    ULONG Iterations = 0;


    outBufLen = WORKING_BUFFER_SIZE;

    do {

        pAddresses = (IP_ADAPTER_ADDRESSES *) HeapAlloc(GetProcessHeap(), 0, (outBufLen));
        if (pAddresses == NULL) {
            PERROR("Memory allocation for IP_ADAPTER_ADDRESSES");
            return 0;
        }

        dwRetVal =
        GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

        if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
            HeapFree(GetProcessHeap(), 0, pAddresses);
            pAddresses = NULL;
        } else {
            break;
        }

        Iterations++;

    } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

    if(NO_ERROR != dwRetVal) {
        SetLastError(dwRetVal);
        PERROR("GetAdaptersAddresses");
    }

    /* walk array */
    for(pCurrAddresses = pAddresses; pCurrAddresses != NULL; pCurrAddresses = pCurrAddresses->Next) {
        if(pCurrAddresses->IfType != IF_TYPE_ETHERNET_CSMACD && pCurrAddresses->IfType != IF_TYPE_IEEE80211) {
            continue;
        }

        if(NULL == pCurrAddresses->FirstUnicastAddress) {
            continue;
        }

        addr = (struct sockaddr_in *)pCurrAddresses->FirstUnicastAddress->Address.lpSockaddr;

        /* there is no other way to get interface names
         * we must show available names to user */
        INFO("found interface: %s\t(%S)\n", pCurrAddresses->AdapterName, pCurrAddresses->Description);

        if ((0==strncmp(ifaceName, pCurrAddresses->AdapterName, strlen(pCurrAddresses->AdapterName)))) {
            DBG("Interface %s selected\n", pCurrAddresses->AdapterName);
            *communicationTechnology = PTP_ETHER;
            strncpy(ifaceName, pCurrAddresses->AdapterName, IFACE_NAME_LENGTH);
            memcpy(uuid, pCurrAddresses->PhysicalAddress, min(pCurrAddresses->PhysicalAddressLength, PTP_UUID_LENGTH));
            *ifaceAddr = addr->sin_addr;
            result = TRUE;
        }
    }

    if(pAddresses) {
        HeapFree(GetProcessHeap(), 0, pAddresses);
    }

    if (ifaceName[0] == 0) {
        INFO("\n\nno interface specified, add -b parameter:\n\n"
              "   -b {GUID}         bind PTP to network interface GUID\n"
              "\nuse -? option for help\n"
        );
        result = FALSE;
    }

    return result;
}

/**
 * start platform dependent network initialization
 *
 * @param netPath
 *
 * @return TRUE if successful
 */
Boolean
netInitWSA2(NetPath * netPath)
{
    WSADATA data;
    WORD version;
    int ret = 0;

    // Winsock 2 required
    version = (MAKEWORD(2, 2));
    ret = WSAStartup(version, &data);

    if(ret != 0)
    {
        PERROR("netInitWSA2");
        return FALSE;
    }

    return TRUE;
}

#ifdef TIMESTAMPING_PCAP
Boolean
netInitPcap(NetPath * netPath, char * deviceName)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    bpf_u_int32 NetMask = 0xffffff;
    struct bpf_program fcode;
    char tmpStr[101];

    snprintf(tmpStr, 100, "rpcap://\\Device\\NPF_%s", deviceName);
    if ( NULL == (netPath->eventPcap = pcap_open_live(tmpStr, PACKET_SIZE, 0, 20, errbuf))) {
        ERROR("Error opening PCAP\n");
        return FALSE;
    }

    if (0 != pcap_setnonblock(netPath->eventPcap, 1, errbuf)) {
        ERROR("pcap_setnonblock: %s\n", errbuf);
        return FALSE;
    }

    snprintf(tmpStr, 100, "udp dst port %d", PTP_EVENT_PORT);
    if(pcap_compile(netPath->eventPcap, &fcode, tmpStr, 1, NetMask) < 0) {
        ERROR("Error compiling PCAP filter: wrong syntax.\n");
        return FALSE;
    }

    if(pcap_setfilter(netPath->eventPcap, &fcode)<0) {
        ERROR("Error setting PCAP filter\n");
        return FALSE;
    }

    return TRUE;
}

/**
 * initialize timestamping on event socket
 *
 * @param netPath
 * @param rtOpts
 *
 * @return TRUE if successful
 */
Boolean
netInitTimestampsPcap(NetPath *netPath, RunTimeOpts * rtOpts)
{
#if defined(TIMESTAMPING_PCAP)
    if(!netInitPcap(netPath, rtOpts->ifaceName)) {
        ERROR("failed to initialize PCAP\n");
        return FALSE;
    }
    DBG("using PCAP as timestamping method\n");
#endif

    return TRUE;
}


#endif

/**
 * Check if data have been received
 *
 * @param timeout
 * @param netPath
 *
 * @return result > 0: some message has been recieved,
 * result = 0: timeout or external interrupt (SIG_ALRM)
 * result < 0: error occured
 */
int
netSelectWSA2(TimeInternal * timeout, NetPath * netPath)
{
    int ret = 0;
    int n = 0;

    WSAEVENT pEvents[4];

    // initialize events
    if(!netPath->eventHEvent) {
        netPath->eventHEvent = WSACreateEvent();
        WSAEventSelect(netPath->eventSock, netPath->eventHEvent, FD_READ);

        netPath->generalHEvent = WSACreateEvent();
        WSAEventSelect(netPath->generalSock, netPath->generalHEvent, FD_READ);
    }

    pEvents[n++] = netPath->eventHEvent;

    pEvents[n++] = netPath->generalHEvent;

    if(NULL != TimerSelectInterrupt) {
        pEvents[n++] = TimerSelectInterrupt;
    }

#if defined(TIMESTAMPING_PCAP)
    pEvents[n++] = pcap_getevent(netPath->eventPcap);
#endif
    ret = WaitForMultipleObjects(n, pEvents, FALSE, INFINITE);

    if (ret >= WSA_WAIT_EVENT_0) {
        ret -= WSA_WAIT_EVENT_0;
        ret += 1;

        /* cleare appropriate event*/
        ResetEvent(pEvents[ret-1]);

        /* if the event is from timer, return value is 0 */
        if(pEvents[ret-1] == TimerSelectInterrupt) {
            ret = 0;
        }
    } else {
        ret = 0;
    }
    return ret;
}

#if defined(TIMESTAMPING_PCAP)
ssize_t
netRecvEventPcap(Octet *buf, TimeInternal *time, NetPath * netPath)
{
    ssize_t ret;

    struct pcap_pkthdr *header;
    const u_char *pkt_data;
    int payload_offset;

    ret = pcap_next_ex( netPath->eventPcap, &header, &pkt_data);
    if(ret <= 0) {
        return ret;
    }

    if(NULL != time) {
        time->seconds = header->ts.tv_sec;
        time->nanoseconds = header->ts.tv_usec * 1000;
    }

    /* we should not test, if it is realy UDP packet, because of PCAP filter settings do it elsewhere */
    payload_offset = 14 /* ethernet */ + (pkt_data[14] & 0xf) * 4 /* IP */ + 8 /* UDP */;
    ret = header->len - payload_offset;

    memcpy(buf, pkt_data + payload_offset, ret);

    return ret;
}
#endif

ssize_t
netRecvEventWSA2(Octet *buf, TimeInternal *time, NetPath * netPath)
{
    ssize_t ret;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

#if defined(TIMESTAMPING_GET_TIME)
    if(NULL != time) {
        /* we dont have better oportunity to recieve timestamp (if not using WinPcap) */
        getTime(time);
    }
#endif

    ret = recvfrom(netPath->eventSock, buf, PACKET_SIZE, MSG_PEEK, (struct sockaddr *)&addr, &addr_len);
    if(ret > 0) {
        ret = recvfrom(netPath->eventSock, buf, PACKET_SIZE, 0, (struct sockaddr *)&addr, &addr_len);
    } else {
        errno = EAGAIN;
        ret = 0;
    }
    if(ret <= 0) {
        if(errno == EAGAIN || errno == EINTR)
            return 0;
        return ret;
    }

    return ret;
}

/**
 *
 * store received data from network to "buf", timestamping
 * of general messages is not necessery
 *  * @param buf
 * @param time
 * @param netPath
 *
 * @return size of recieved packet
 */

ssize_t
netRecvGeneralWSA2(Octet * buf, NetPath * netPath)
{
    ssize_t ret;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    /* on windows platform is not MSG_DONTWAIT, using MSG_PEEK insted */
    ret = recvfrom(netPath->generalSock, buf, PACKET_SIZE, MSG_PEEK, (struct sockaddr *)&addr, &addr_len);
    if(ret > 0) {
        ret = recvfrom(netPath->generalSock, buf, PACKET_SIZE, 0, (struct sockaddr *)&addr, &addr_len);
    } else {
        errno = EAGAIN;
        ret = 0;
    }

    if (ret <= 0) {
        if (errno == EAGAIN || errno == EINTR)
            return 0;

        return ret;
    }
    return ret;
}