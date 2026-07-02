
/*
 * Apple Bonjour dns_sd.h API definition for Windows CoNSync build
 */
#ifndef __DNS_SD_H
#define __DNS_SD_H

#include <stdint.h>
#include <winsock2.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque reference type for DNSService */
typedef struct _DNSServiceRef_t *DNSServiceRef;

/* TXTRecord reference type */
typedef union _TXTRecordRef_t {
    char PrivateData[16];
    char *ForceNaturalAlignment;
} TXTRecordRef;

/* Basic types */
typedef uint32_t DNSServiceFlags;
typedef int32_t  DNSServiceErrorType;
typedef uint32_t DNSServiceProtocol;

/* Error constants */
enum {
    kDNSServiceErr_NoError           = 0,
    kDNSServiceErr_Unknown           = -65537,
    kDNSServiceErr_NoSuchName        = -65538,
    kDNSServiceErr_NoMemory          = -65539,
    kDNSServiceErr_BadParam          = -65540,
    kDNSServiceErr_BadReference      = -65541,
    kDNSServiceErr_BadState          = -65542,
    kDNSServiceErr_BadFlags          = -65543,
    kDNSServiceErr_Unsupported       = -65544,
    kDNSServiceErr_NotInitialized    = -65545,
    kDNSServiceErr_AlreadyRegistered = -65547,
    kDNSServiceErr_NameConflict      = -65548,
    kDNSServiceErr_Invalid           = -65549,
    kDNSServiceErr_Firewall          = -65550,
    kDNSServiceErr_Incompatible      = -65551,
    kDNSServiceErr_BadInterfaceIndex = -65552,
    kDNSServiceErr_Refused           = -65553,
    kDNSServiceErr_NoSuchRecord      = -65554,
    kDNSServiceErr_NoAuth            = -65555,
    kDNSServiceErr_NoSuchKey         = -65556,
    kDNSServiceErr_NATTraversal      = -65557,
    kDNSServiceErr_DoubleNAT         = -65558,
    kDNSServiceErr_BadTime           = -65559,
    kDNSServiceErr_BadSig            = -65560,
    kDNSServiceErr_BadKey            = -65561,
    kDNSServiceErr_Transient         = -65562,
    kDNSServiceErr_ServiceNotRunning = -65563,
    kDNSServiceErr_NATPortMappingUnsupported = -65564,
    kDNSServiceErr_NATPortMappingDisabled    = -65565,
    kDNSServiceErr_NoRouter          = -65566,
    kDNSServiceErr_PollingMode       = -65567,
    kDNSServiceErr_Timeout           = -65568
};

/* Flag constants */
enum {
    kDNSServiceFlagsMoreComing          = 0x1,
    kDNSServiceFlagsAdd                 = 0x2,
    kDNSServiceFlagsDefault             = 0x4,
    kDNSServiceFlagsNoAutoRename        = 0x8,
    kDNSServiceFlagsShared              = 0x10,
    kDNSServiceFlagsUnique              = 0x20,
    kDNSServiceFlagsBrowseDomains       = 0x40,
    kDNSServiceFlagsRegistrationDomains = 0x80,
    kDNSServiceFlagsLongLivedQuery      = 0x100,
    kDNSServiceFlagsAllowRemoteQuery    = 0x200,
    kDNSServiceFlagsForceMulticast      = 0x400,
    kDNSServiceFlagsForce               = 0x800,
    kDNSServiceFlagsShareConnection     = 0x4000,
    kDNSServiceFlagsSuppressUnusable    = 0x8000,
    kDNSServiceFlagsTimeout             = 0x10000,
    kDNSServiceFlagsIncludeP2P          = 0x20000,
    kDNSServiceFlagsIncludeAWDL         = 0x40000
};

/* Protocol constants */
enum {
    kDNSServiceProtocol_IPv4 = 0x01,
    kDNSServiceProtocol_IPv6 = 0x02,
    kDNSServiceProtocol_UDP  = 0x10,
    kDNSServiceProtocol_TCP  = 0x20
};

#ifdef __cplusplus
}
#endif

#endif /* __DNS_SD_H */
