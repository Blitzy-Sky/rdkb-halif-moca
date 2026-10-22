/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/

/**********************************************************************

    module: moca_hal.h

        For CCSP Component:  MoCA_Provisioning_and_management

    ---------------------------------------------------------------

    description:

        This header file gives the function call prototypes and
        structure definitions used for the RDK-Broadband
        MoCA hardware abstraction layer

    ---------------------------------------------------------------

    environment:

        This HAL layer is intended to support MoCA drivers
        through an open API.  The current implementation created
        below this HAL supports MoCA.
        Changes may be needed to support other MoCA enviornments.

         @file moca_hal.h
         @brief RDK-Broadband MoCA Hardware Abstraction Layer
         This header file defines function prototypes, structures, and constants used
         for interfacing with the MoCA hardware through a standardized API.
         @details The current implementation is tailored for a specific MoCA environment. The
         MoCA specification version it targets is not stated by this interface; the `MixedMode`
         member of `moca_cfg_t`, admitting 1.0/1.1/2.0 operation, is its only version reference.
         @component MoCA_Provisioning_and_management

    ---------------------------------------------------------------

    author:

        Cisco

**********************************************************************/

#ifndef __MOCA_HAL_H__
#define __MOCA_HAL_H__

/*
 * Common scalar type and status aliases.
 *
 * This interface defines its own ULONG, BOOL, CHAR, UCHAR, INT and UINT aliases, and its
 * own TRUE, FALSE, ENABLE, STATUS_SUCCESS, STATUS_FAILURE and STATUS_NOT_AVAILABLE
 * values, rather than taking them from a shared RDK-B header. Each definition is guarded
 * by #ifndef, so a translation unit that has already defined a name keeps its own
 * definition and this header adopts it. Two consequences bind a caller.
 *
 * First, the widths are the platform's, not this interface's: ULONG is unsigned long and
 * INT is int, so their ranges follow the target's data model. Where a member or a return
 * value has a narrower valid range, that range is stated on the member or the function
 * and not here.
 *
 * Second, because the guards let an earlier definition win, a caller that defines any of
 * these names differently before including this header changes the meaning of every
 * declaration below without any diagnostic. In particular STATUS_SUCCESS, STATUS_FAILURE
 * and STATUS_NOT_AVAILABLE are the return vocabulary of eighteen of the functions here,
 * so redefining one of them silently redefines what a successful call looks like. A
 * caller should let this header supply them.
 */

#ifndef ULONG
#define ULONG unsigned long
#endif

#ifndef BOOL
#define BOOL  unsigned char   /*!< Alias for the byte-wide boolean used across RDK-B. It is a whole byte and not a single bit, so a `BOOL` member is compared against TRUE or FALSE rather than tested for any non-zero value. */
#endif

#ifndef CHAR
#define CHAR  char   /*!< Alias for `char`. Used for the text members of the structures below - interface name, firmware version, protocol version and MAC address text. */
#endif

#ifndef UCHAR
#define UCHAR unsigned char   /*!< Alias for `unsigned char`. Used for the frequency and taboo bit masks, for raw MAC address octets, and for the per-subcarrier arrays of moca_scmod_stat_t. */
#endif

#ifndef INT
#define INT   int   /*!< Alias for `int`. The return type of 16 of the 21 declarations here, and the type of the signed power levels and node identifiers in the structures below. */
#endif

#ifndef UINT
#define UINT  unsigned int   /*!< Alias for `unsigned int`. Used for the node identifier, channel and reporting-node bitmask members of moca_aca_cfg_t. */
#endif

#ifndef TRUE
#define TRUE     1   /*!< The value a BOOL member carries when the condition it reports holds. */
#endif

#ifndef FALSE
#define FALSE    0   /*!< The value a BOOL member carries when the condition it reports does not hold. */
#endif

#ifndef ENABLE
#define ENABLE   1   /*!< Enabled state, numerically equal to TRUE. No declaration in this header takes or returns it; it is provided for callers that express enablement separately from a boolean. */
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS     0   /*!< The operation succeeded and any output buffer the call was given has been populated. Returned by all 18 status-returning functions in this header. */
#endif

#ifndef STATUS_FAILURE
#define STATUS_FAILURE     -1   /*!< The operation failed. This interface attaches no reason to it, so a caller can retry, fall back or report, but cannot branch on a cause; the vendor log named under `Logging and debugging requirements` in the repository specification is where the reason has to be found. */
#endif

#ifndef STATUS_NOT_AVAILABLE
#define STATUS_NOT_AVAILABLE     -2   /*!< A requested value or capability is not available. Declared by this interface for that purpose; no declaration below documents returning it, so a caller treats it as a member of the failure class rather than as an expected outcome of a particular call. */
#endif

/**
 * @defgroup MOCA_HAL MoCA Hardware Abstraction Layer (HAL)
 *
 * This group encompasses functions, data types, and constants that provide an abstraction layer for interacting with
 * MoCA hardware devices. The MoCA HAL facilitates communication between higher-level software components and the
 * underlying MoCA driver, enabling operations like configuration, status retrieval, and statistics collection.
 *
 * @ingroup Network_Interfaces
 */

/**
 * @defgroup MOCA_HAL_TYPES MoCA HAL Data Types
 *
 * This subgroup defines the data structures used within the MoCA HAL to represent MoCA configurations, statistics,
 * and other relevant information.
 *
 * @ingroup MOCA_HAL
 */

/**
 * @defgroup MOCA_HAL_APIS MoCA HAL APIs
 *
 * This subgroup contains the function prototypes for the MoCA HAL API, which allows applications to control and
 * monitor MoCA devices through a standardized interface.
 *
 * @ingroup MOCA_HAL
 */

/**
 * @addtogroup MOCA_HAL_TYPES
 * @{
 */

/**
 * @brief Maximum number of CPE (Customer Premises Equipment) devices in a MoCA network.
 */
#define kMoca_MaxCpeList 256

/**
 * @brief Maximum number of MoCA nodes allowed in a network.
 */
#define kMoca_MaxMocaNodes 16

/**
 * @brief Number of padding bytes added to a 6-byte MAC address to make it 18 bytes long. 
 *        This is required on platforms that handle MAC addresses as 18 bytes, while RDKB uses 6-byte MACs.
 */
#define MAC_PADDING 12

#define STATUS_INPROGRESS -1             /**< Status code: ACA process is in progress */
#define STATUS_NO_NODE -2                /**< Status code: Specified MoCA node does not exist */
#define STATUS_INVALID_PROBE -3          /**< Status code: HAL API called with invalid probe type */
#define STATUS_INVALID_CHAN -4           /**< Status code: HAL API called with invalid channel */

/*
 * These four ACA status codes are preprocessor macros rather than members of an
 * enumeration, and that is not a cosmetic distinction. Because they are untyped integer
 * constants sharing the return type of every other call, the compiler cannot tell a
 * caller that it has compared an ACA return value against a non-ACA code, and two pairs
 * of them collide numerically with the general codes above: STATUS_INPROGRESS is -1, the
 * same value as STATUS_FAILURE, and STATUS_NO_NODE is -2, the same value as
 * STATUS_NOT_AVAILABLE. A caller therefore cannot distinguish "an ACA run is already in
 * progress" from "the call failed" by inspecting the return value alone, and must read
 * moca_getIfAcaStatus() and the stat member of moca_aca_stat_t instead. The colliding
 * values, and what a caller does about them, are set out under `Internal Error Handling`
 * in the repository specification.
 */

/**********************************************************************
                ENUMERATION DEFINITIONS
**********************************************************************/
#ifndef MOCA_VAR

/**
 * @brief States a MoCA interface may report through moca_dynamic_info_t.Status.
 *
 * The enumerators are given explicit values running from 1 to 7, so **0 is not a defined
 * state**: a caller must not read a zeroed structure as a valid status, and this
 * enumeration reserves no value for "not yet read". IF_STATUS_Unknown is the only member
 * that stands for absent information, and it is a value the implementation reports rather
 * than a default the caller may assume.
 *
 * **This enumeration does not describe a state machine.** It defines the values the Status
 * member may hold and nothing more: this interface states no legal transition, no ordering
 * between the states and no event that causes a change, so a caller reads the current value
 * and must not infer a sequence from it. `State Diagram` in the repository specification
 * in the HAL specification records the same position.
 *
 * @note This type exists only where the `MOCA_VAR` macro is **not** defined, as does
 *       `moca_dynamic_info_t`, the only structure that uses it. See the warning on that
 *       type for what a `MOCA_VAR` build actually does.
 * @see moca_dynamic_info_t
 */
typedef enum
{
    IF_STATUS_Up = 1,                /**< Interface is up and operational */
    IF_STATUS_Down = 2,              /**< Interface is down (not operational) */
    IF_STATUS_Unknown = 3,           /**< Interface status is unknown */
    IF_STATUS_Dormant = 4,           /**< Interface is dormant (temporarily inactive) */
    IF_STATUS_NotPresent = 5,        /**< Interface is not physically present */
    IF_STATUS_LowerLayerDown = 6,    /**< Lower layer of the interface is down */
    IF_STATUS_Error = 7             /**< Interface is in an error state */
} moca_if_status_t;

#endif

/**
 * @brief Probe type an ACA run uses to assess a channel.
 *
 * Selected through the Type member of moca_aca_cfg_t. The two members differ in whether
 * the assessed node transmits: a quiet probe measures the channel while the node is
 * silent, and an EVM probe transmits a known signal so that its error vector magnitude
 * can be measured. A value outside this enumeration is reported by
 * moca_setIfAcaConfig() as STATUS_INVALID_PROBE.
 *
 * @note The spelling of PROBE_QUITE is the interface's own; it denotes a quiet probe.
 * @see moca_aca_cfg_t
 * @see moca_setIfAcaConfig
 */
typedef enum
{
    PROBE_QUITE = 0, /**< Quiet probe (no signal transmission) */
    PROBE_EVM   = 1   /**< EVM (Error Vector Magnitude) probe (transmits signal to measure signal quality) */
}PROBE_TYPE;

#if 0
/*
 * Excluded from every build by the enclosing #if 0, and named by no declaration in this
 * header, so it is not part of the interface. A caller reads an ACA outcome from the stat
 * member of moca_aca_stat_t, which is a plain INT.
 */
typedef enum
{
    STAT_SUCCESS =0,         /**< ACA Status is Success. */
    STAT_FAIL_BADCHANNEL =1, /**< ACA Status is Badchannel. */
    STAT_FAIL_NOEVMPROBE =2, /**< ACA Status is NoEVMProbe. */
    STAT_FAIL =3,            /**< ACA Status is fail. */
    STAT_INPROGRESS =4       /**< ACA Status is Inprogress. */
}ACA_STATUS;
#endif
/**********************************************************************
                STRUCTURE DEFINITIONS
**********************************************************************/
/**
 * @brief Configuration parameters for a MoCA interface.
 */
typedef struct
{
    ULONG InstanceNumber;      /*!< Identifies which MoCA interface the rest of this structure describes. Domain: 0 where the platform carries a single MoCA interface, 1 to 256 where it carries several. This is the only place this interface states a range for an interface index, so it is also the range a caller applies to the ifIndex argument the calls here take. No value is reserved to mean "no interface", and no return code is reserved for an index outside the range. */

    CHAR Alias[64];            /*!< Vendor-chosen display name for the interface, held as text in a fixed array of 64 CHAR. This interface documents the member as carrying up to 64 characters, which is the whole declared extent and leaves no byte for a terminator, so **no terminator is established for this member**: a caller bounds every read by the 64-byte extent instead of scanning for one, and writes no more than 64 bytes when it supplies the member through moca_SetIfConfig(). No character set, no default value and no uniqueness rule is stated, and no value is reserved to mean "unset". */
    BOOL bEnabled;             /*!< Administrative enable state requested for the interface: TRUE puts it into service, FALSE takes it out. `BOOL` is `unsigned char` here, so all 256 byte values are representable, but this interface defines only TRUE (1) and FALSE (0) and states nothing about any other value. This is the requested setting; the operational state that results is reported by Status in moca_dynamic_info_t. */
    BOOL bPreferredNC;         /*!< Whether this node asks to be preferred when a Network Coordinator is chosen: TRUE requests the preference, FALSE does not. TRUE or FALSE only, as for every BOOL member here. It expresses a preference and not an outcome - this interface does not state that setting it makes the node coordinator, and the node actually coordinating is reported by NetworkCoordinator in moca_dynamic_info_t. */
    BOOL PrivacyEnabledSetting;/*!< Whether link privacy is to be enabled: TRUE enables it and makes KeyPassphrase meaningful, FALSE disables it. TRUE or FALSE only. This is the requested setting; PrivacyEnabled in moca_dynamic_info_t reports the privacy state of the network the node has actually joined, which this interface does not state must agree with it. */

    UCHAR FreqCurrentMaskSetting[128];    /*!< Frequencies the node may use when forming a network, as a bitmask of 128 UCHAR - 1024 bits, each element a single unsigned byte in the range 0 to 255. **The mapping from bit position to frequency is vendor-specific and is not stated by this interface**, and neither is the bit or byte order, so a caller can neither construct nor interpret the mask from this header alone. moca_FreqMaskToValue() converts a mask to the frequency value it denotes, and its own contract records that the number of bytes it reads is not established. No value is reserved to mean "no frequency selected". */
    CHAR KeyPassphrase[18];    /*!< Link privacy password, meaningful only while PrivacyEnabledSetting is TRUE, held as text in a fixed array of 18 CHAR. This interface documents the member as carrying up to 18 characters, which is the whole declared extent and leaves no byte for a terminator, so **no terminator is established for this member**: a caller bounds every read by the 18-byte extent and writes no more than 18 bytes when it supplies the member through moca_SetIfConfig(). No minimum length, character set or strength rule is stated, and no value is reserved to mean "no passphrase". The value is a secret; see the warning on moca_GetIfConfig(), which reads it back. */

    INT TxPowerLimit;          /*!< Maximum allowed transmission power. INT is int, whose width is platform-dependent - at least 16 bits by the C standard and 32 bits on every data model RDK-B targets - so the representable range is that of the target's int. This interface states no narrower valid range and no unit for the limit. */
    ULONG AutoPowerControlPhyRate;   /*!< Target PHY rate for automatic power control. ULONG is unsigned long, whose width is platform-dependent - at least 32 bits, and 64 bits on an LP64 target - so the representable range is 0 to at least (2^32)-1. This interface states no narrower valid range and no unit for the rate. */
    ULONG BeaconPowerLimit;    /*!< Maximum allowed transmission power for beacons. ULONG is unsigned long, whose width is platform-dependent - at least 32 bits, and 64 bits on an LP64 target - so the representable range is 0 to at least (2^32)-1. This interface states no narrower valid range and no unit. */

    ULONG MaxIngressBWThreshold; /*!< Maximum ingress bandwidth threshold; MaxIngressBWThresholdReached in moca_dynamic_info_t reports when it has been reached. ULONG is unsigned long, whose width is platform-dependent - at least 32 bits, and 64 bits on an LP64 target - so the representable range is 0 to at least (2^32)-1. This interface states no narrower valid range and no unit. */
    ULONG MaxEgressBWThreshold;  /*!< Maximum egress bandwidth threshold; MaxEgressBWThresholdReached in moca_dynamic_info_t reports when it has been reached. ULONG is unsigned long, whose width is platform-dependent - at least 32 bits, and 64 bits on an LP64 target - so the representable range is 0 to at least (2^32)-1. This interface states no narrower valid range and no unit. */

    BOOL Reset;                /*!< Requests that the interface's configuration parameters return to their defaults: TRUE requests the reset, FALSE requests nothing. TRUE or FALSE only. This interface does not state which members the reset restores, what their default values are, whether the reset completes before the moca_SetIfConfig() call carrying it returns, or whether the member reads back as TRUE afterwards. */
    BOOL MixedMode;            /*!< Whether the node may operate on a network mixing MoCA generations - 1.0, 1.1 and 2.0 nodes together: TRUE admits mixed operation, FALSE does not. TRUE or FALSE only. This is the only place this interface names a MoCA specification version, and it does not state which version applies when the member is FALSE. */
    BOOL ChannelScanning;      /*!< Whether the node scans for a beacon across the frequencies selected by ChannelScanMask (TRUE) or stays on a single frequency (FALSE). TRUE or FALSE only. This interface states neither which frequency the single-frequency case uses nor how long a scan may take; the frequency actually in use is reported by CurrentOperFreq in moca_dynamic_info_t. */
    BOOL AutoPowerControlEnable;   /*!< Whether the node adjusts its transmit power automatically to hold the rate in AutoPowerControlPhyRate (TRUE) or leaves the power as configured (FALSE). TRUE or FALSE only. This interface states no algorithm, no step size and no settling time for the adjustment. */

    BOOL EnableTabooBit;       /*!< Whether the frequencies selected by NodeTabooMask are honoured as forbidden (TRUE) or ignored (FALSE). TRUE or FALSE only. This interface does not state what happens when the member is TRUE and the mask selects nothing, nor whether the taboo set binds the local node alone or the network. */
    UCHAR NodeTabooMask[128];   /*!< Frequencies the node must avoid, honoured only while EnableTabooBit is TRUE, as a bitmask of 128 UCHAR - 1024 bits, each element 0 to 255. As with every mask in this header the bit-to-frequency mapping is vendor-specific and unstated, so a caller neither constructs nor interprets it from this header; moca_FreqMaskToValue() is the only interpretation aid, and no value is reserved to mean "nothing taboo". */
    UCHAR ChannelScanMask[128]; /*!< Frequencies the node scans for a beacon while ChannelScanning is TRUE, as a bitmask of 128 UCHAR - 1024 bits, each element 0 to 255, with the same vendor-specific and unstated bit-to-frequency mapping. This interface states neither the scan order nor what happens if the mask selects no frequency. */
} moca_cfg_t;

/**
 * @brief Static information about a MoCA interface.
 */
typedef struct
{
    CHAR Name[64];                      /*!< Name by which the platform identifies this MoCA interface, for example "moca0" or "nmoca0", held as text in a fixed array of 64 CHAR. This interface documents the member as carrying up to 64 characters, which is the whole declared extent and leaves no byte for a terminator, so **no terminator is established for this member**: a caller bounds every read by the 64-byte extent. It is read-only static information, and this interface states no relationship between the name and the ifIndex argument its calls take, so a caller cannot derive one from the other. */
    UCHAR MacAddress[6 + MAC_PADDING];   /*!< MAC address of the local node, as 6 + MAC_PADDING = 18 raw bytes, not text. The first six bytes are the six address octets in network order, most significant octet first; the remaining MAC_PADDING (12) bytes exist only because some platforms handle a MAC address as 18 bytes, and this interface does not specify their content. A caller reads the six octets and formats them itself; it must not read this member as a printable string, and there is no NUL terminator to stop at. */
    CHAR FirmwareVersion[64];            /*!< Version of the vendor firmware running on the MoCA device, held as free-form text in a fixed array of 64 CHAR. This interface documents the member as carrying up to 64 characters, which is the whole declared extent and leaves no byte for a terminator, so **no terminator is established for this member**: a caller bounds every read by the 64-byte extent. No format, ordering or comparison rule is stated, so a caller must not parse it into numeric fields or order two values. It reports the vendor firmware, not a revision of this interface, which declares no version macro at all. */
    ULONG MaxBitRate;                   /*!< Maximum PHY rate the local node supports. ULONG is unsigned long, whose width is platform-dependent - at least 32 bits, and 64 bits on an LP64 target - so the representable range is 0 to at least (2^32)-1. This interface states no narrower valid range and no unit for the rate. */
    CHAR HighestVersion[64];            /*!< Highest MoCA protocol version the local node is capable of, expressed as text such as "1.0", "1.1" or "2.0" in a fixed array of 64 CHAR. This interface documents the member as carrying up to 64 characters, which is the whole declared extent and leaves no byte for a terminator, so **no terminator is established for this member**: a caller bounds every read by the 64-byte extent. The permitted values are not enumerated here; MixedMode in moca_cfg_t, which admits 1.0/1.1/2.0 operation, is the only other version reference this interface makes. Compare CurrentVersion in moca_dynamic_info_t, which reports the version actually in use. */

    UCHAR FreqCapabilityMask[8];       /*!< Frequencies the local node is capable of using, as a bitmask of 8 UCHAR - 64 bits, each element 0 to 255, with the vendor-specific and unstated bit-to-frequency mapping every mask in this header shares. It is eight bytes wide where the moca_cfg_t masks are 128, so the two are not interchangeable as arguments: moca_FreqMaskToValue() takes a bare pointer and states that the number of bytes it reads is not established, which makes passing this member to it a bounds question the caller must settle with the vendor implementation. Read-only capability information. */
    UCHAR NetworkTabooMask[128];       /*!< Frequencies the local node reports as not supported, as a bitmask of 128 UCHAR - 1024 bits, each element 0 to 255, with the same vendor-specific and unstated bit-to-frequency mapping. Despite the name it is read from the local node by moca_IfGetStaticInfo(), and this interface does not state how it relates to the writable NodeTabooMask in moca_cfg_t. */

    ULONG TxBcastPowerReduction;       /*!< Beacon backoff, in dB. ULONG is unsigned long, whose width is platform-dependent - at least 32 bits, and 64 bits on an LP64 target - so the representable range is 0 to at least (2^32)-1. This interface states no narrower valid range, and because the type is unsigned the value expresses the magnitude of the reduction rather than a signed offset. */
    BOOL QAM256Capable;                /*!< Whether the node can use QAM-256 modulation: TRUE if it can, FALSE if it cannot. TRUE or FALSE only. It reports a fixed property of the hardware rather than a setting - this interface exposes no way to turn the capability on or off. */
    BOOL PacketAggregationCapability;  /*!< Whether the node can carry several Ethernet packets in one MoCA transmission: TRUE if it can, FALSE if it cannot. TRUE or FALSE only, and again a fixed property rather than a setting. Where it is TRUE, the totals in moca_aggregate_counters_t and the ExtAggrAverage members of moca_stats_t are the counters that describe the aggregation. */
} moca_static_info_t;

#ifndef MOCA_VAR
/**
 * @brief Dynamic information about a MoCA interface.
 *
 * @warning **This type is excluded when `MOCA_VAR` is defined, but the function that
 *          returns it, moca_IfGetDynamicInfo(), is not — so a translation unit that
 *          defines `MOCA_VAR` and includes this header does not compile.** The
 *          declaration at the end of this header names `moca_dynamic_info_t` outside any
 *          guard, so with `MOCA_VAR` defined the compiler reports an unknown type name
 *          there and the whole header fails, taking every other declaration in it with
 *          it. The consequence is not that one API is unavailable in that build: the
 *          `MOCA_VAR` variant of this header is ill-formed, so a caller cannot use this
 *          type, or any other declaration this header makes, from a translation unit that
 *          defines `MOCA_VAR`. `Platform or Product Customization` in the repository
 *          specification states the same limitation.
 * @see moca_IfGetDynamicInfo
 */
typedef struct
{
    moca_if_status_t Status;             /*!< Operational state of the local interface, as one of the seven moca_if_status_t enumerators, whose values run from 1 to 7 - so 0 is not a defined state and no enumerator stands for "not yet read". **This interface does not state which transitions are legal or in what order they occur**, so a caller reads the current value and must not infer a sequence from it; `State Diagram` in the repository specification records the same. IF_STATUS_Unknown is the only value that stands for absent information. */
    ULONG LastChange;                   /*!< Time at which the link status last changed, in seconds since the epoch. ULONG is unsigned long, so the value is representable from 0 to at least (2^32)-1 and cannot be negative. This interface does not state which clock the value comes from, whether it survives a reboot, or what the member holds where the status has never changed, and it reserves no value for that case - so a caller must not difference it against its own clock until it has established the source with the vendor implementation. */
    ULONG MaxIngressBW;                 /*!< Highest ingress bandwidth the interface has seen, in bits per second. ULONG, so 0 to at least (2^32)-1. It is a high-water mark and not an instantaneous rate: this interface states neither the interval it covers nor any way to reset it, so a caller cannot tell how old the maximum is. MaxIngressBWThresholdReached below reports whether it has crossed the MaxIngressBWThreshold configured through moca_cfg_t. */
    ULONG MaxEgressBW;                  /*!< Highest egress bandwidth the interface has seen, in bits per second. ULONG, so 0 to at least (2^32)-1, a high-water mark with no stated interval and no stated reset, exactly as for MaxIngressBW. MaxEgressBWThresholdReached below reports whether it has crossed the MaxEgressBWThreshold configured through moca_cfg_t. */
    CHAR CurrentVersion[64];            /*!< MoCA protocol version the node is currently operating with, expressed as text such as "1.0", "1.1" or "2.0" in a fixed array of 64 CHAR. This interface neither enumerates the permitted values nor states a termination rule for the member, so **no terminator is established** and a caller bounds every read by the 64-byte extent. What the member holds while the node has joined no network is not specified, and no value is reserved for that case. Compare HighestVersion in moca_static_info_t, which reports capability rather than the version in use. */
    ULONG NetworkCoordinator;          /*!< Identifier of the node currently acting as Network Coordinator. Domain: 0 to 7 on a MoCA 1.0 network and 0 to 15 on a 1.1 or 2.0 network, which is consistent with kMoca_MaxMocaNodes (16). **No value is reserved to mean "no coordinator elected"**, so a caller cannot distinguish an unknown coordinator from node 0 by this member alone; the coordinator's address is in NetworkCoordinatorMACAddress below. */
    ULONG NodeID;                      /*!< Identifier the local node holds on the network it has joined. Domain: 0 to 7 on a MoCA 1.0 network and 0 to 15 on a 1.1 or 2.0 network. No value is reserved to mean "not joined", so a caller that needs to know whether the node is on a network reads Status rather than this member. */
    ULONG BackupNC;                    /*!< Identifier of the node standing by to take over as Network Coordinator. Domain: 0 to 7 on a MoCA 1.0 network and 0 to 15 on a 1.1 or 2.0 network. No value is reserved to mean "no backup elected", and this interface does not state that a backup is always present, so a caller must not treat the value as a live node without cross-checking the associated-device list. */
    BOOL PrivacyEnabled;               /*!< Whether link privacy is in force on the network the node has joined: TRUE if it is, FALSE if it is not. TRUE or FALSE only. It reports the network's state, which this interface does not state must equal the local request in PrivacyEnabledSetting of moca_cfg_t. */
    UCHAR FreqCurrentMask[8];           /*!< Frequencies currently in use, as a bitmask of 8 UCHAR - 64 bits, each element 0 to 255, with the vendor-specific and unstated bit-to-frequency mapping every mask in this header shares. It is eight bytes where the moca_cfg_t masks are 128, so the two are not interchangeable; moca_FreqMaskToValue() is the only interpretation aid and does not establish the width it reads. */
    ULONG CurrentOperFreq;              /*!< Frequency the node is operating on, in the vendor's own representation. **This interface states no unit and no range for the value** - it describes it only as vendor-specific - so a caller must not read it as Hz, MHz or a channel index until it has established the representation with the vendor implementation. ULONG, so 0 to at least (2^32)-1, and no value is reserved to mean "not operating". */
    ULONG LastOperFreq;                 /*!< Frequency the node operated on before the current one, in the same unstated vendor representation as CurrentOperFreq and with no unit or range stated. This interface does not state what the member holds before the first change of frequency, and reserves no value for that case. */
    ULONG TxBcastRate;                 /*!< Rate at which the node transmits broadcast traffic. **This interface states no unit for this member**: moca_mesh_table_t documents its rates in Mbps and moca_associated_device_t calls its equivalent a PHY rate, but neither statement is made here, so the unit must be established with the vendor implementation. ULONG, so 0 to at least (2^32)-1. */
    BOOL MaxIngressBWThresholdReached;  /*!< Whether ingress traffic has reached the MaxIngressBWThreshold configured through moca_cfg_t: TRUE if it has, FALSE if it has not. TRUE or FALSE only. This interface does not state when the flag clears, whether it latches until something resets it, or over what interval the comparison is made. */
    BOOL MaxEgressBWThresholdReached;   /*!< Whether egress traffic has reached the MaxEgressBWThreshold configured through moca_cfg_t: TRUE if it has, FALSE if it has not. TRUE or FALSE only, with the same unstated clearing, latching and interval behaviour as the ingress flag above. */
    ULONG NumberOfConnectedClients;     /*!< Number of other nodes the local node is connected to on the MoCA network. This interface documents the range as 0 to 256, which does not agree with kMoca_MaxMocaNodes (16), its own published maximum for the nodes a network may hold; the two cannot both bound this population and this interface does not reconcile them, so a caller validates against the smaller figure and treats a larger value as unexpected rather than as fact. It states neither whether the local node is included nor how the count relates to the CPE list that kMoca_MaxCpeList (256) bounds. */
    CHAR NetworkCoordinatorMACAddress[18]; /*!< MAC address of the node currently acting as Network Coordinator, in the printable colon-separated form AA:BB:CC:DD:EE:FF, which occupies 17 of the 18 declared CHAR. Unlike the raw-octet MAC members of moca_static_info_t and moca_associated_device_t, this one is text - but **this interface does not state that the eighteenth byte holds a terminator**, so a caller bounds every read by the 18-byte extent and must not treat the member as a terminated string. What it holds before a coordinator is known, and whether the remaining byte is written at all, are not specified. The coordinator's node identifier is in NetworkCoordinator above. */
    ULONG LinkUpTime;                   /*!< Time the node has been connected to the MoCA network with at least one other node present, in seconds. ULONG, so 0 to at least (2^32)-1. This interface does not state whether the value restarts when the last peer leaves and rejoins, whether it wraps or saturates at the type's maximum, or what it holds while the node is alone on the coax, and it reserves no value for that case. */
} moca_dynamic_info_t;

#endif

/**
 * @brief Network-layer traffic counters for a MoCA interface, as read by moca_IfGetStats().
 *
 * Every member is a `ULONG`, so each is representable from 0 to at least (2^32)-1 and none
 * can be negative. Four properties a caller needs in order to use these as counters are
 * **not stated by this interface**, and their absence applies to every member below: what
 * resets them, whether they wrap or saturate on reaching the type's maximum, over what
 * interval they accumulate, and whether the members are sampled atomically with respect to
 * one another. A caller computing a rate must therefore tolerate a reading lower than the
 * previous one instead of assuming monotonic growth, and must not assume that two members of
 * one read describe the same instant. Counts are of whole frames except where a member is
 * documented in bytes.
 */
typedef struct
{
    ULONG BytesSent;              /*!< Bytes the interface has transmitted. This interface does not state whether MoCA framing overhead is included, or whether control traffic is counted alongside payload. */
    ULONG BytesReceived;          /*!< Bytes the interface has received, with the same unstated treatment of framing overhead and control traffic as BytesSent. */
    ULONG PacketsSent;            /*!< Frames the interface has transmitted, counted as whole frames rather than bytes. The unicast, multicast and broadcast members below count subsets of the same traffic, but this interface does not state that they sum to this total, so a caller must not derive one from the others. */
    ULONG PacketsReceived;        /*!< Frames the interface has received, counted as whole frames, with the same unstated relationship to the per-address-type members below. */
    ULONG ErrorsSent;             /*!< Transmitted frames the interface recorded as errored. **What counts as an error is not stated**, so a caller must not read the value as a particular fault class, and this interface does not state whether an errored frame is also counted in PacketsSent. */
    ULONG ErrorsReceived;         /*!< Received frames the interface recorded as errored, with the error condition equally unstated and the same open question about double counting against PacketsReceived. */
    ULONG UnicastPacketsSent;     /*!< Frames transmitted to a single destination address. */
    ULONG UnicastPacketsReceived; /*!< Frames received that were addressed to this node alone. */
    ULONG DiscardPacketsSent;     /*!< Frames dropped on the transmit path instead of being sent. The reasons for a discard, and whether a discarded frame is also counted in ErrorsSent, are not stated. */
    ULONG DiscardPacketsReceived; /*!< Frames dropped on the receive path instead of being delivered, with the reasons and the overlap with ErrorsReceived equally unstated. */
    ULONG MulticastPacketsSent;   /*!< Frames transmitted to a multicast address. */
    ULONG MulticastPacketsReceived; /*!< Frames received that were addressed to a multicast group. */
    ULONG BroadcastPacketsSent;   /*!< Frames transmitted to the broadcast address. */
    ULONG BroadcastPacketsReceived; /*!< Frames received that were addressed to the broadcast address. */
    ULONG UnknownProtoPacketsReceived; /*!< Received frames whose upper-layer protocol the interface did not recognise; they are counted here instead of being classified, and this interface does not state whether they are also counted in PacketsReceived. */
    ULONG ExtAggrAverageTx;       /*!< Average number of frames the interface aggregated into one transmission, in the vendor's own scaling. **Neither the unit nor the averaging window is stated** - this interface describes the value only as vendor-specific - so a caller must establish both with the vendor implementation. The capability itself is reported by PacketAggregationCapability in moca_static_info_t. */
    ULONG ExtAggrAverageRx;       /*!< Average number of frames the interface found aggregated in one reception, in the same unstated vendor scaling and with the same unstated averaging window as ExtAggrAverageTx. */
} moca_stats_t;

/**
 * @brief MoCA MAC-layer control-message counters, as read by moca_IfGetExtCounter().
 *
 * These count MoCA control traffic rather than user data, and every member counts messages
 * **received** by the local node; this interface declares no transmit counterpart. Each is a
 * `ULONG`, representable from 0 to at least (2^32)-1, and as with moca_stats_t this interface
 * states nothing about what resets these counters, whether they wrap or saturate at the
 * type's maximum, over what interval they accumulate, or whether the members are sampled
 * atomically with respect to one another.
 */
typedef struct
{
    ULONG Map;   /*!< MAP messages received - the MoCA Access Protocol messages by which the Network Coordinator publishes the transmission schedule. */
    ULONG Rsrv;  /*!< Reservation request messages received, by which nodes ask the coordinator for a transmission opportunity. */
    ULONG Lc;    /*!< Link control messages received. This interface does not enumerate which link control message types are counted. */
    ULONG Adm;   /*!< Admission request messages received, by which a node asks to join the network. */
    ULONG Probe; /*!< Probe messages received - the channel-assessment traffic whose type an ACA run selects through PROBE_TYPE in moca_aca_cfg_t. This interface does not state whether probes from an ACA run and probes from ordinary network maintenance are counted separately. */
    ULONG Async; /*!< Asynchronous beacon messages received. */
} moca_mac_counters_t;

/**
 * @brief Aggregate MoCA payload data unit totals, as read by moca_IfGetExtAggrCounter().
 *
 * Both members count payload data units and exclude MoCA control packets, so they are not
 * comparable with the control-message counters in moca_mac_counters_t and are not a subset of
 * the frame counts in moca_stats_t. Each is a `ULONG`, representable from 0 to at least
 * (2^32)-1, and this interface again states nothing about resets, wrapping or saturation, the
 * accumulation interval, or atomic sampling between the two members.
 */
typedef struct
{
    ULONG Tx;  /*!< Payload data units transmitted, MoCA control packets excluded. */
    ULONG Rx;  /*!< Payload data units received, MoCA control packets excluded. */
} moca_aggregate_counters_t;

/**
 * @brief Represents a MoCA Customer Premises Equipment (CPE) node.
 */
typedef struct
{
    CHAR mac_addr[6];  /*!< MAC address of the CPE node, as exactly six raw address octets in network order, most significant octet first. It is not text: the printable form AA:BB:CC:DD:EE:FF is 17 characters and cannot be held in six bytes, there is no NUL terminator, and CHAR is signed on some platforms so an octet above 0x7F reads as a negative value unless the caller converts it to unsigned first. A caller reads the six octets and formats them itself. */
} moca_cpe_t;

/**
 * @brief Information about a device associated with the MoCA network.
 */
typedef struct
{
    UCHAR MACAddress[6 + MAC_PADDING];   /*!< MAC address of the associated device, as 6 + MAC_PADDING = 18 raw bytes, not text. The first six bytes are the six address octets in network order, most significant octet first; the remaining MAC_PADDING (12) bytes exist only because some platforms handle a MAC address as 18 bytes, and this interface does not specify their content. A caller reads the six octets and formats them itself; it must not read this member as a printable string, and there is no NUL terminator to stop at. */
    ULONG NodeID;                      /*!< Identifier this device holds on the MoCA network. This interface describes the value as vendor-specific and states no range for it here, unlike the node identifiers in moca_dynamic_info_t which it bounds at 0 to 15, so a caller treats it as an opaque key and does not assume that 0 is invalid. */
    BOOL PreferredNC;                  /*!< Whether this device asks to be preferred when a Network Coordinator is chosen: TRUE if it does, FALSE if it does not. TRUE or FALSE only, and a preference rather than an outcome - the node actually coordinating is reported by NetworkCoordinator in moca_dynamic_info_t. */
    CHAR HighestVersion[64];            /*!< Highest MoCA protocol version the associated device is capable of, expressed as text such as "1.0", "1.1" or "2.0" in a fixed array of 64 CHAR. This interface neither enumerates the permitted values nor states a termination rule for the member, so **no terminator is established** and a caller bounds every read by the 64-byte extent. It describes the remote device, not the local node, whose equivalent is HighestVersion in moca_static_info_t. */
    ULONG PHYTxRate;                   /*!< PHY rate at which this device transmits. **This interface states no unit for the value**: moca_mesh_table_t documents its rates in Mbps, but that statement is not made here, so the unit must be established with the vendor implementation. ULONG, so 0 to at least (2^32)-1, and no value is reserved to mean "no rate established". */
    ULONG PHYRxRate;                   /*!< PHY rate at which this device receives, with no unit stated for the same reason as PHYTxRate and no value reserved for an unestablished rate. */
    ULONG TxPowerControlReduction;     /*!< Reduction this device applies to its transmit power, in dB. ULONG is unsigned, so the member carries the magnitude of the reduction rather than a signed offset, and this interface states no maximum for it. */
    INT RxPowerLevel;                  /*!< Received power this device measures, in dBm. INT is signed, so the negative values normal for dBm at a receiver are representable. This interface states no range and reserves no value for "not measured". */
    ULONG TxBcastRate;                 /*!< PHY rate at which this device transmits broadcast traffic, with no unit stated by this interface for the same reason as PHYTxRate. */
    INT RxBcastPowerLevel;             /*!< Received power this device measures on broadcast traffic, in dBm, signed and with no stated range exactly as for RxPowerLevel. */
    ULONG TxPackets;                   /*!< Frames this device has transmitted, counted as whole frames. As with the counters in moca_stats_t, this interface states nothing about what resets the value, whether it wraps or saturates at the type's maximum, or over what interval it accumulates, so a caller computing a rate must tolerate a reading lower than the previous one. */
    ULONG RxPackets;                   /*!< Frames this device has received, counted as whole frames, with the same unstated reset, wrap and interval behaviour as TxPackets. */
    ULONG RxErroredAndMissedPackets;   /*!< Frames this device received in error together with frames it did not receive at all. The two conditions share one counter and this interface offers no way to separate them, states no definition of either, and states no reset, wrap or interval behaviour. */
    BOOL QAM256Capable;                /*!< Whether this device can use QAM-256 modulation: TRUE if it can, FALSE if it cannot. TRUE or FALSE only, describing the remote device rather than the local node, whose equivalent is QAM256Capable in moca_static_info_t. */
    BOOL PacketAggregationCapability;  /*!< Whether this device can carry several Ethernet packets in one MoCA transmission: TRUE if it can, FALSE if it cannot. TRUE or FALSE only, again describing the remote device. */
    ULONG RxSNR;                       /*!< Signal-to-noise ratio this device measures on the signal it receives. **This interface states neither a unit nor a scaling for the value**, and because ULONG is unsigned the member cannot express a negative ratio, so a caller must not read it as dB until it has established the representation with the vendor implementation. */
    BOOL Active;                      /*!< Whether this device is currently present on the network: TRUE if it is, FALSE if it is not. TRUE or FALSE only. It is also the member that carries the event in a notification delivered through moca_associatedDevice_callback - TRUE for an activation, FALSE for a deactivation. */
    ULONG RxBcastRate;                 /*!< PHY rate at which this device receives broadcast traffic, with no unit stated for the same reason as PHYRxRate. */
    ULONG NumberOfClients;             /*!< Client devices reachable behind this node. This interface states no range for the count and does not state that it corresponds to the entries moca_GetMocaCPEs() reports, so a caller must not use it to size that array - kMoca_MaxCpeList (256) is the sizing rule that call states. No value is reserved to mean "unknown". */
} moca_associated_device_t;

#ifndef MOCA_VAR
/**
 * @brief Represents an entry in the MoCA mesh PHY rate table.
 *
 * This table stores the unicast transmit PHY rates between all pairs of nodes in the MoCA network.
 */
typedef struct
{
    ULONG RxNodeID;     /*!< Identifier of the receiving node of the pair this entry describes, and with TxNodeID the pair of values that locate the entry in the table. This interface states no range for it here; kMoca_MaxMocaNodes (16) is the only bound it publishes on the nodes a network may hold, and no value is reserved to mean "unknown". */
    ULONG TxNodeID;     /*!< Identifier of the transmitting node of the pair this entry describes, with the same unstated range and absent reserved value as RxNodeID. The pair is ordered, so an entry for (Tx=a, Rx=b) is a different entry from (Tx=b, Rx=a) and this interface does not state that the two carry equal rates. */
    ULONG TxRate;       /*!< Unicast PHY rate from TxNodeID to RxNodeID, in Mbps. ULONG, so 0 to at least (2^32)-1. This interface states neither an averaging interval nor a value reserved for a pair between which no rate has been established, so a caller must not read 0 as "no link". */
    ULONG TxRateNper;   /*!< PHY rate from TxNodeID to RxNodeID for NPER - Network Performance Enhancement Rate - traffic, in Mbps, on a MoCA 2.x network. **What the member holds on a network that is not 2.x is not stated**, and no value is reserved for that case, so a caller checks the network version through CurrentVersion in moca_dynamic_info_t before reading it. */
    ULONG TxRateVlper;  /*!< PHY rate from TxNodeID to RxNodeID for VLPER - Very Low Packet Error Rate - traffic, in Mbps, on a MoCA 2.x network, with the same unstated behaviour on a pre-2.x network and the same absent reserved value as TxRateNper. */
} moca_mesh_table_t;
#endif

/**
 * @brief Represents an entry in the MoCA interface flow statistics table.
 *
 * This table provides statistics about ingress PQoS (Priority Quality of Service) flows in a MoCA interface.
 */
typedef struct
{
    ULONG FlowID;              /*!< Identifier the MoCA network has assigned to this flow. It is opaque: this interface states no range, no allocation rule and no reserved value, so a caller uses it only to correlate one read of the flow table with the next. */
    ULONG IngressNodeID;       /*!< Identifier of the node at which the flow enters the MoCA network. This interface states no range for it here, unlike the node identifiers in moca_dynamic_info_t, and reserves no value for "unknown". */
    ULONG EgressNodeID;        /*!< Identifier of the node at which the flow leaves the MoCA network, with no range stated and no reserved value, as for IngressNodeID. */
    ULONG FlowTimeLeft;        /*!< Lease time still to run on this flow. **This interface states no unit for this member**, although it documents the initial LeaseTime below in seconds, so a caller must not assume the two share a unit until it has established that with the vendor implementation. No value is reserved to mean "expired". */
    char DestinationMACAddress[18];   /*!< Destination MAC address of the Ethernet packets carried by this PQoS flow, in the printable colon-separated form AA:BB:CC:DD:EE:FF, which occupies 17 of the 18 declared char. Like moca_dynamic_info_t.NetworkCoordinatorMACAddress, and unlike the raw-octet MAC members elsewhere in this header, this one is text - but **this interface does not state that the eighteenth byte holds a terminator**, so a caller bounds every read by the 18-byte extent and must not treat the member as a terminated string. Note that char here is the plain type rather than this header's CHAR alias, and that it is signed on some platforms. */
    ULONG PacketSize;          /*!< Number of MoCA aggregated frames carried in the flow, which is what this interface describes the member as holding even though its name suggests a size in bytes. **No unit is stated for either reading**, so a caller must establish which is meant with the vendor implementation before acting on the value. */
    ULONG PeakDataRate;        /*!< Peak data rate of the flow, in bits per second. ULONG, so 0 to at least (2^32)-1. This interface states no interval over which the peak is measured. */
    ULONG BurstSize;           /*!< Burst size of the flow, in bytes. ULONG, so 0 to at least (2^32)-1, and no maximum is stated. */
    ULONG FlowTag;            /*!< Tag the requesting application attached to the flow. This interface assigns it no meaning, states no range and reserves no value, so it is opaque to the HAL and interpreted only by whatever set it. */
    ULONG LeaseTime;           /*!< Lease time the flow was granted, in seconds. This interface does not state what the member holds for a flow that carries no lease, and reserves no value for that case; FlowTimeLeft above reports what remains, in a unit this interface does not state. */
} moca_flow_table_t;

/**
 * @brief Type of the handler a caller registers for MoCA associated-device events.
 *
 * A function of this type is invoked when a MoCA client is activated or deactivated on the
 * network. That trigger is the whole of what this interface states about when a handler
 * runs. **Nothing here establishes the context of the call**: this interface does not state
 * which thread invokes the handler, whether two invocations may overlap, whether they are
 * ordered with respect to each other, or whether an invocation may occur while one of the
 * caller's own calls into this interface is still in progress. A handler must therefore be
 * written to tolerate all of those readings - it serialises access to any state it shares
 * with the rest of the caller, and it must not assume that it holds a lock the caller took,
 * or that it runs between the caller's calls rather than inside one.
 *
 * @param[in] ifIndex Index of the MoCA interface on which the event occurred. This interface
 *                    does not state that the value is one of the indices the caller has
 *                    itself queried, so a handler checks it against the interfaces it knows
 *                    before using it as a key.
 * @param[in] moca_dev Address of a `moca_associated_device_t` describing the device the event
 *                     concerns. Its `Active` member carries the event itself: TRUE for an
 *                     activation, FALSE for a deactivation; the remaining members carry the
 *                     node's identity and link characteristics. **The lifetime of the pointee
 *                     is not specified.** This interface neither states that the record stays
 *                     valid for the duration of the handler call, nor that it is released when
 *                     the handler returns, and it names no owner and no release function. A
 *                     handler therefore copies whatever it needs out of the record before it
 *                     does anything else, keeps no copy of the pointer, and does not free the
 *                     storage. Whether the same storage is reused for a later event is not
 *                     stated either, so two invocations must not be assumed to address
 *                     distinct records.
 *
 * @return INT - the status the handler reports for its own handling of the event. **This
 *         interface does not state what an implementation does with the value**, defines no
 *         vocabulary for it, and defines no retry, replay or de-registration behaviour keyed
 *         to it. A handler cannot use it to request redelivery, and a caller must not depend
 *         on any consequence of returning one value rather than another; returning
 *         STATUS_SUCCESS where the handler has nothing to report is the least surprising
 *         choice.
 *
 * @see moca_associatedDevice_callback_register
 * @see moca_associated_device_t
 */
typedef INT (*moca_associatedDevice_callback)(ULONG ifIndex, moca_associated_device_t *moca_dev);

/**
 * @brief A node's preferred-Network-Coordinator information.
 *
 * @note **No declaration in this header takes or returns this type**, so nothing in this
 *       interface produces or consumes a value of it. It is documented because it is public
 *       and a caller will find it, not because a call populates it; a caller that needs the
 *       same information for the nodes on its network reads `PreferredNC` and `NodeID` from
 *       the moca_associated_device_t entries that moca_GetAssociatedDevices() returns.
 */
typedef struct moca_assoc_pnc_info
{
    ULONG mocaNodeIndex;      /*!< Identifier of the node this record describes. This interface states no range for it and reserves no value for "unknown". */
    BOOL  mocaNodePreferredNC; /*!< Whether the node asks to be preferred when a Network Coordinator is chosen: TRUE if it does, FALSE if it does not. TRUE or FALSE only, and a preference rather than an outcome. */
    ULONG mocaNodeMocaversion; /*!< MoCA version the node supports, encoded as the major and minor version run together: by this interface's own examples 10 for MoCA 1.0, 11 for 1.1 and 20 for 2.0. The full set of values is not enumerated here and no value is reserved for "unknown". The same information appears as text, in a different form, in the HighestVersion and CurrentVersion members elsewhere in this header. */
} moca_assoc_pnc_info_t;

/**
 * @brief Subcarrier modulation statistics for a MoCA link between two nodes.
 */
typedef struct
{
    INT TxNode;    /*!< Identifier of the transmitting node of the link these statistics describe. INT is signed, so negative values are representable, but this interface states no range for the member and reserves no value for "not applicable". */
    INT RxNode;    /*!< Identifier of the receiving node of the link these statistics describe, signed and with no stated range or reserved value, as for TxNode. */
    INT Channel;   /*!< Channel the NPER and VLPER figures below were calculated on, which this interface identifies only as the primary or the secondary channel. **It does not state which numeric value denotes which channel**, nor any other value the member may take, so a caller must establish the encoding with the vendor implementation before acting on it. */

    UCHAR Mod[512];   /*!< Modulation scheme in use on each subcarrier of Channel, one UCHAR per subcarrier. The array holds 512 elements and each element is a single unsigned byte, so its value range is 0 to 255; UCHAR is unsigned char (see the definition at the top of this header). The meaning of a given value, and how many of the 512 elements carry a meaningful one for a particular channel, are vendor-specific and are not stated by this interface. */
    UCHAR Nper[512];  /*!< NPER (Network Performance Enhancement Rate) bit loading on each subcarrier of Channel, one UCHAR per subcarrier. The array holds 512 elements and each element is a single unsigned byte, so its value range is 0 to 255. The scaling of a value, and how many of the 512 elements carry a meaningful one, are vendor-specific and are not stated by this interface. */
    UCHAR Vlper[512]; /*!< VLPER (Very Low Packet Error Rate) bit loading on each subcarrier of Channel, one UCHAR per subcarrier. The array holds 512 elements and each element is a single unsigned byte, so its value range is 0 to 255. The scaling of a value, and how many of the 512 elements carry a meaningful one, are vendor-specific and are not stated by this interface. */
} moca_scmod_stat_t;

/**
 * @brief Configuration parameters for initiating an ACA (Automatic Channel Adaptation) test.
 */
typedef struct
{
    UINT NodeID;      /*!< Node the run assesses. UINT is unsigned int, so 0 to at least (2^32)-1 is representable, while kMoca_MaxMocaNodes (16) bounds the nodes a network may hold; moca_setIfAcaConfig() reports STATUS_NO_NODE for a node that does not exist, which is the only validation this interface describes. No value is reserved to mean "every node" - ReportNodes below selects the reporting set. */
    PROBE_TYPE Type;  /*!< Probe the run uses, as one of the two PROBE_TYPE enumerators: PROBE_QUITE (0) measures the channel while the assessed node stays silent, PROBE_EVM (1) has it transmit a known signal so that error vector magnitude can be measured. A value outside the enumeration is reported by moca_setIfAcaConfig() as STATUS_INVALID_PROBE. */
    UINT Channel;    /*!< Channel the run assesses. **This interface states no unit, numbering or range for the value**; moca_setIfAcaConfig() reports STATUS_INVALID_CHAN where the implementation does not accept it, which is the only validation this interface describes, so a caller must establish the numbering with the vendor implementation. */
    UINT ReportNodes; /*!< Which nodes are included in the assessment, as a bitmask whose least significant bit selects node 0. UINT is at least 32 bits wide, so it can select more positions than the kMoca_MaxMocaNodes (16) a network may hold; this interface does not state what an implementation does with a bit above that limit, and reserves no value to mean "all nodes". */
    BOOL ACAStart;    /*!< Whether the moca_setIfAcaConfig() call carrying this structure starts a run (TRUE) or only records the configuration for a later start (FALSE). TRUE or FALSE only, and the distinction is the one documented on that function. */
} moca_aca_cfg_t;

/**
 * @brief Status and results of an ACA (Automatic Channel Adaptation) test.
 */
typedef struct
{
    moca_aca_cfg_t acaCfg;      /*!< Configuration the reported run was performed with, as a copy of the moca_aca_cfg_t a caller passed to moca_setIfAcaConfig(). This interface does not state what the member holds where no run has yet been requested on the interface. */
    INT stat;                  /*!< Outcome of the run, as a plain INT taking 0 for success, 1 for a bad channel, 2 for a missing EVM probe, 3 for failure and 4 for a run still in progress. Those five values are the whole of what this interface defines for the member: it names no enumerated type for them, so the compiler cannot check a comparison, and it neither defines nor reserves a sixth value. The member is also the only way to separate a run in progress from a failed request, because the STATUS_INPROGRESS and STATUS_FAILURE return codes share the value -1. */
    INT RxPower;               /*!< Total received power measured over the run, in dBm. INT is signed, so the negative values normal for dBm at a receiver are representable; no range is stated and no value is reserved for "not measured". */
    INT ACAPowProfile[512];     /*!< Received power per channel measured by the run, in dBm, as 512 signed INT elements one per channel. **How many of the 512 elements carry a measurement is not stated**, nor is the mapping from element index to channel, and no value marks an element that was not measured; the profile is meaningful only once ACATrapCompleted below is TRUE. */
    BOOL ACATrapCompleted;     /*!< Whether the power profile above is complete and may be read: TRUE when it is ready, FALSE otherwise. TRUE or FALSE only. This interface states neither how long a run takes nor how often a caller should re-read the status while waiting, so a caller polls moca_getIfAcaStatus() at an interval it chooses itself. */
} moca_aca_stat_t;

/** @} */  //END OF GROUP MOCA_HAL_TYPES

/**
 * @addtogroup MOCA_HAL_APIS
 * @{
 */

/**
 * @brief Installs the caller's handler for MoCA associated-device join and leave events.
 *
 * Registers a function that the implementation invokes whenever a MoCA client is
 * activated or deactivated on the network, so that a caller can learn of membership
 * changes without polling. It is the only asynchronous notification in this interface;
 * everything else is a synchronous read or write. Registration is optional: a caller that
 * polls `moca_GetNumAssociatedDevices()` and `moca_GetAssociatedDevices()` need never
 * call it.
 *
 * **Most of the registration contract is not stated by this interface**, and the paragraphs
 * below record that rather than filling the gaps in. What this interface does state is the
 * trigger above and the type of the argument. It does not state what an implementation does
 * with the pointer, what a second call does, what a null pointer means, or from where a
 * delivery arrives, so a caller must program for the widest reading of each.
 *
 * @param[in] callback_proc Pointer to the caller's handler, of type
 *                          `moca_associatedDevice_callback`. **Retention is not specified**:
 *                          this interface does not state whether the implementation stores
 *                          the pointer, for how long, or whether it ever drops it. Because
 *                          it also declares no de-registration call and no way to query what
 *                          is registered, a caller has no point at which it can establish
 *                          that delivery has ceased, and must therefore keep the handler
 *                          callable - and every object the handler reaches through allocated
 *                          and not moved - for as long as the process may receive events.
 *                          Registering a handler that lives in a module which is later
 *                          unloaded, or that reaches storage the caller later frees or
 *                          relocates, leaves an implementation holding a pointer it may
 *                          still call. **Replacement is not specified**: whether a second
 *                          call replaces the first handler, is kept alongside it, or is
 *                          rejected is not stated, and neither is how many handlers this
 *                          interface retains. **A null pointer is not specified either**:
 *                          there is no de-registration call, and whether NULL clears a
 *                          previously registered handler or is simply stored and later
 *                          dereferenced is not stated, so a caller must not use NULL to
 *                          unregister.
 *
 * @pre None. This interface declares no initialization or teardown call, so registration
 *      may be performed at any point; `Initialization and Startup` in the repository
 *      specification places it before the first
 *      `moca_IfGetDynamicInfo()` in the normal bring-up order, but does not require it.
 * @post Not specified. **This function returns `void`, so it reports no outcome**: a caller
 *       cannot tell from it whether the handler was accepted, and this interface offers no
 *       other way to query what is registered, so a failed registration is reported by
 *       nothing and the absence of events is its only symptom. Nor does this interface state
 *       that the handler is invoked at all, or that an event which occurred before the call
 *       is delivered after it. A caller that must not miss a membership change reconciles
 *       against `moca_GetNumAssociatedDevices()` and `moca_GetAssociatedDevices()` instead of
 *       treating the notification stream as complete.
 *
 * @note Blocking: this interface states nothing specific to this call - neither that it is
 *       cheap local bookkeeping, nor that it waits for anything. The only statement that
 *       bears on it is the module-wide non-blocking requirement in `Blocking calls` in the
 *       repository specification, which covers every function here
 *       and which this declaration neither extends nor excepts.
 * @note Thread safety: this interface is not thread safe, and `Threading Model` in the
 *       repository specification places serialisation on the calling module. The delivery
 *       side is not specified at all: which thread invokes the handler, whether two
 *       invocations may overlap, whether they are ordered, and whether one may arrive while
 *       a call of the caller's own is still in progress are all unstated. A caller must
 *       therefore serialise its own state against both paths, its own calls and its handler
 *       alike, and must not assume that registering from one thread makes deliveries arrive
 *       on it. The lifetime of the record a delivery carries is unspecified in the same way;
 *       `moca_associatedDevice_callback` states what a handler must copy before it returns.
 *
 * @warning There is no de-registration call. Once a handler has been passed to this
 *          function, this interface provides no way to withdraw it, so the caller carries
 *          the retention duty described on `callback_proc` for the remaining life of the
 *          process.
 *
 * @see moca_associatedDevice_callback
 * @see moca_associated_device_t
 * @see moca_GetAssociatedDevices
 */
void moca_associatedDevice_callback_register(moca_associatedDevice_callback callback_proc); 

/**********************************************************************************
 *
 *  MoCA Subsystem level function prototypes
 *
**********************************************************************************/

/*
 * The return-code set below is deliberately coarse, and a caller must treat it as open
 * rather than closed.
 *
 * Eighteen of the twenty-one functions declared here report a status drawn from
 * STATUS_SUCCESS, STATUS_FAILURE and STATUS_NOT_AVAILABLE, plus the four ACA-specific
 * codes. STATUS_FAILURE carries no reason: the interface does not partition a failure by
 * cause, so a caller cannot branch on one and can only retry, fall back, or report it.
 * Nothing in this header states that those codes are the whole of what an implementation
 * may return.
 *
 * The consequence for a caller is a forward-compatibility rule: treat any value other
 * than STATUS_SUCCESS as a failure of the class documented on the function, rather than
 * comparing against the currently defined codes and assuming an unrecognised value cannot
 * occur. `Internal Error Handling` in the repository specification
 * in the HAL specification states the same rule and lists the two pairs of codes that
 * collide numerically.
 */

/**
 * @brief Reads the configuration currently in force on a MoCA interface into a
 *        caller-supplied structure.
 *
 * Populates the caller's `moca_cfg_t` with the enable, privacy, power, bandwidth-threshold
 * and frequency-mask settings the interface is running with. `Initialization and Startup`
 * in the repository specification makes this the read half of
 * the read-modify-write sequence a caller must use with `moca_SetIfConfig()`: because that
 * call takes the whole structure, a caller reads the configuration back first so that the
 * members it does not intend to change are preserved.
 *
 * @param[in] ifIndex Index of the MoCA interface to query.
 *                    `moca_cfg_t.InstanceNumber` states the range this interface uses for
 *                    an interface index: 0 where a single MoCA interface is present, 1 to
 *                    256 where several are. This interface does not state what an
 *                    implementation does with an index outside that range, and no return
 *                    code is reserved for it.
 * @param[out] pmoca_config Pointer to a caller-allocated `moca_cfg_t` that the
 *                          implementation fills in. It must be non-NULL. The caller both
 *                          allocates and releases the storage - the `Memory Model` topic
 *                          of the repository specification places every output buffer on
 *                          the caller and states that this interface declares no allocator
 *                          and no matching release function. Nothing here establishes that
 *                          the implementation retains the pointer beyond the call. The
 *                          fixed arrays inside the structure bound what may be read from
 *                          it: the `Alias` and `KeyPassphrase` text arrays are 64 and 18
 *                          bytes with no terminator established, and the three mask
 *                          members are 128 vendor-specific bytes each.
 *
 * @pre This interface declares no initialization or teardown call, so it imposes no call
 *      ordering and opens no session; `moca_HardwareEquipped()` is the only readiness
 *      check available, as `Initialization and Startup` in the repository specification
 *      in the HAL specification states. The pointer argument must address
 *      caller-allocated storage of the declared type. This interface does not state what
 *      an implementation does with a null pointer, so passing one is not covered by this
 *      contract and is not reported through a distinct return code.
 * @post On `STATUS_SUCCESS` the structure holds the configuration in force. On
 *       `STATUS_FAILURE` this interface does not state whether any member was written, so
 *       the caller must treat the whole structure as unusable and must not use it as the
 *       basis of a subsequent `moca_SetIfConfig()`.
 *
 * @returns INT - the outcome of the read, reported synchronously as `Internal Error
 *          Handling` in the repository specification requires.
 * @retval STATUS_SUCCESS - The configuration was read.
 * @retval STATUS_FAILURE - The operation failed. This interface does not distinguish the
 *         reason, so the client can only retry, fall back, or report the failure; it
 *         cannot branch on a cause, and it treats any non-zero value as a failure of this
 *         class because the code set is open.
 *
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification makes
 *       its non-blocking rule a steady-state one - a ready implementation must not suspend
 *       the caller's main thread - and excepts the start-up window before the vendor
 *       subsystem is ready, where a call may not return promptly and a caller bounds it.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process, so a vendor
 *       implementation guards its own state.
 *
 * @warning `KeyPassphrase` carries the link privacy password. It is read back through this
 *          call, so a caller must not log the structure wholesale or copy it into
 *          diagnostic output; `Logging and debugging requirements` in the repository
 *          specification governs what may be logged.
 *
 * @see moca_SetIfConfig
 * @see moca_cfg_t
 */
INT moca_GetIfConfig(ULONG ifIndex, moca_cfg_t *pmoca_config);

/**
 * @brief Applies a complete configuration to a MoCA interface.
 *
 * Writes the enable, privacy, power, bandwidth-threshold and frequency-mask settings the
 * caller has supplied. The call takes the **whole** structure rather than a delta, so
 * every member is applied and a member the caller did not deliberately set is applied with
 * whatever value it happens to hold. `Initialization and Startup` in the repository
 * specification therefore requires a caller to read the
 * configuration with `moca_GetIfConfig()` first and modify the members it intends to
 * change.
 *
 * @param[in] ifIndex Index of the MoCA interface to configure.
 *                    `moca_cfg_t.InstanceNumber` states the range this interface uses for
 *                    an interface index: 0 where a single MoCA interface is present, 1 to
 *                    256 where several are. This interface does not state what an
 *                    implementation does with an index outside that range, and no return
 *                    code is reserved for it.
 * @param[in] pmoca_config Pointer to a caller-allocated `moca_cfg_t` holding the
 *                         configuration to apply. It must be non-NULL and every member
 *                         must be set, for the reason above. The parameter is an input
 *                         even though the pointer is not const-qualified; nothing in this
 *                         interface states that the implementation writes through it, and
 *                         a caller should not expect values to be read back here. The
 *                         caller owns the storage, and this interface does not specify
 *                         whether the implementation retains the pointer, so a caller
 *                         keeps it valid. Member constraints the interface does state:
 *                         `Alias` is at most 64 bytes and `KeyPassphrase` at most 18,
 *                         `TxPowerLimit` is a signed `INT`, the three 128-byte masks are
 *                         vendor-specific, and `Reset` set to `TRUE` returns the
 *                         configuration parameters to their defaults. **This interface
 *                         states no valid range for any of the numeric members and no
 *                         validation rule**, so which combinations an implementation
 *                         accepts is not specified and a rejected value is reported only
 *                         as `STATUS_FAILURE`.
 *
 * @pre This interface declares no initialization or teardown call, so it imposes no call
 *      ordering and opens no session; `moca_HardwareEquipped()` is the only readiness
 *      check available, as `Initialization and Startup` in the repository specification
 *      in the HAL specification states. The pointer argument must address
 *      caller-allocated storage of the declared type. This interface does not state what
 *      an implementation does with a null pointer, so passing one is not covered by this
 *      contract and is not reported through a distinct return code.
 * @post On `STATUS_SUCCESS` the configuration has been applied. **On `STATUS_FAILURE` this
 *       interface does not state whether the configuration was applied in part**, so a
 *       caller cannot assume the interface was left as it was: it should read the
 *       configuration back with `moca_GetIfConfig()` to establish the actual state rather
 *       than assuming either outcome. Nothing is persisted - `Persistence Model` in the
 *       repository specification places persistence on the caller.
 *
 * @returns INT - the outcome of the write, reported synchronously as `Internal Error
 *          Handling` in the repository specification requires.
 * @retval STATUS_SUCCESS - The configuration was applied.
 * @retval STATUS_FAILURE - The operation failed. This interface does not distinguish the
 *         reason, so the client can only retry, fall back, or report the failure; it
 *         cannot branch on a cause, and it treats any non-zero value as a failure of this
 *         class because the code set is open.
 *
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification makes
 *       its non-blocking rule a steady-state one - a ready implementation must not suspend
 *       the caller's main thread - and excepts the start-up window before the vendor
 *       subsystem is ready, where a call may not return promptly and a caller bounds it.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process, so a vendor
 *       implementation guards its own state.
 *       Two callers writing configuration concurrently is the case this matters most for,
 *       since each supplies a whole structure and the later write overwrites the earlier
 *       one in full.
 *
 * @warning Setting `Reset` to `TRUE` discards the current configuration and restores
 *          defaults. A caller performing a read-modify-write must clear it in the
 *          structure it read back unless a reset is intended.
 *
 * @see moca_GetIfConfig
 * @see moca_cfg_t
 */
INT moca_SetIfConfig(ULONG ifIndex, moca_cfg_t *pmoca_config);

/**
 * @brief Reads the live state of a MoCA interface and the network it has joined into a
 *        caller-supplied structure.
 *
 * Populates the caller's `moca_dynamic_info_t` with the values that change while the
 * interface is up: link status and the timestamp of its last change, the MoCA version in
 * use, the network coordinator and backup coordinator node IDs and the local node ID, the
 * current and previous operating frequencies, the ingress and egress bandwidth maxima and
 * their threshold flags, the connected client count and the link uptime. It is the
 * counterpart of `moca_IfGetStaticInfo()`, which reads the values that do not change.
 *
 * @param[in] ifIndex Index of the MoCA interface to query.
 *                    `moca_cfg_t.InstanceNumber` states the range this interface uses for
 *                    an interface index: 0 where a single MoCA interface is present, 1 to
 *                    256 where several are. This interface does not state what an
 *                    implementation does with an index outside that range, and no return
 *                    code is reserved for it.
 * @param[out] pmoca_dynamic_info Pointer to a caller-allocated `moca_dynamic_info_t` that
 *                                the implementation fills in. It must be non-NULL. The
 *                                caller both allocates and releases the storage - the
 *                                `Memory Model` topic of the repository specification
 *                                in the HAL specification places every output buffer on
 *                                the caller and states that this interface declares no
 *                                allocator and no matching release function. Nothing here
 *                                establishes that the implementation retains the pointer
 *                                beyond the call. Two members bound what may be read:
 *                                `CurrentVersion` is a 64-byte text array and
 *                                `NetworkCoordinatorMACAddress` an 18-byte array holding
 *                                the 17-character printable MAC form, neither with a
 *                                terminator established, so both are read within extent.
 *                                `Status` is a `moca_if_status_t`, whose transitions this
 *                                interface does not specify; `State Diagram` in the
 *                                repository specification states that a caller must read
 *                                the current value and must not assume an ordering.
 *
 * @pre This interface declares no initialization or teardown call, so it imposes no call
 *      ordering and opens no session; `moca_HardwareEquipped()` is the only readiness
 *      check available, as `Initialization and Startup` in the repository specification
 *      in the HAL specification states. The pointer argument must address
 *      caller-allocated storage of the declared type. This interface does not state what
 *      an implementation does with a null pointer, so passing one is not covered by this
 *      contract and is not reported through a distinct return code.
 * @post On `STATUS_SUCCESS` the structure holds the interface's live state at the moment of
 *       the read, and it may be stale by the time the caller inspects it. On
 *       `STATUS_FAILURE` this interface does not state whether any member was written, so
 *       the whole structure must be treated as unusable.
 *
 * @returns INT - the outcome of the read, reported synchronously as `Internal Error
 *          Handling` in the repository specification requires.
 * @retval STATUS_SUCCESS - The dynamic information was read.
 * @retval STATUS_FAILURE - The operation failed. This interface does not distinguish the
 *         reason, so the client can only retry, fall back, or report the failure; it
 *         cannot branch on a cause, and it treats any non-zero value as a failure of this
 *         class because the code set is open.
 *
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification makes
 *       its non-blocking rule a steady-state one - a ready implementation must not suspend
 *       the caller's main thread - and excepts the start-up window before the vendor
 *       subsystem is ready, where a call may not return promptly and a caller bounds it.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process, so a vendor
 *       implementation guards its own state.
 *
 * @warning **This declaration is not guarded, but the `moca_dynamic_info_t` type it names
 *          is: the type is defined inside `#ifndef MOCA_VAR`. A translation unit that
 *          defines `MOCA_VAR` and includes this header therefore does not compile at all -
 *          the compiler reaches this line, finds an undeclared type, and rejects the
 *          header, taking every other declaration in it down with it.** The consequence is
 *          not that this one API is unavailable in a `MOCA_VAR` build: that variant of the
 *          header is ill-formed, and guarding the caller's own dynamic-information path
 *          under `#ifndef MOCA_VAR` does not avoid it, because the failure happens inside
 *          the header before any of the caller's code is seen. `Platform or Product
 *          Customization` in the repository specification states
 *          the same limitation.
 *
 * @see moca_dynamic_info_t
 * @see moca_IfGetStaticInfo
 * @see moca_if_status_t
 */
INT moca_IfGetDynamicInfo(ULONG ifIndex, moca_dynamic_info_t *pmoca_dynamic_info);

/**
 * @brief Reads the fixed identity and capabilities of a MoCA interface into a
 *        caller-supplied structure.
 *
 * Populates the caller's `moca_static_info_t` with the values that do not change while the
 * interface is up: the interface name, the local node's MAC address, the firmware version,
 * the maximum PHY rate and highest supported MoCA version, the frequency capability and
 * network taboo masks, the beacon backoff and the QAM-256 and packet-aggregation capability
 * flags. `Initialization and Startup` in the repository specification
 * in the HAL specification places this immediately after the `moca_HardwareEquipped()`
 * readiness check in a caller's normal bring-up order.
 *
 * @param[in] ifIndex Index of the MoCA interface to query.
 *                    `moca_cfg_t.InstanceNumber` states the range this interface uses for
 *                    an interface index: 0 where a single MoCA interface is present, 1 to
 *                    256 where several are. This interface does not state what an
 *                    implementation does with an index outside that range, and no return
 *                    code is reserved for it.
 * @param[out] pmoca_static_info Pointer to a caller-allocated `moca_static_info_t` that the
 *                               implementation fills in. It must be non-NULL. The caller
 *                               both allocates and releases the storage - the `Memory
 *                               Model` topic of the repository specification places every
 *                               output buffer on the caller and states that this interface
 *                               declares no allocator and no matching release function.
 *                               Nothing here establishes that the implementation retains
 *                               the pointer beyond the call. The fixed arrays bound what
 *                               may be read: `Name`, `FirmwareVersion` and `HighestVersion`
 *                               are 64-byte text arrays with no terminator established,
 *                               `MacAddress` is 6 + MAC_PADDING = 18 **raw bytes** of which
 *                               only the first six are address octets and not text, and
 *                               `FreqCapabilityMask` is 8 and `NetworkTabooMask` 128
 *                               vendor-specific mask bytes.
 *
 * @pre This interface declares no initialization or teardown call, so it imposes no call
 *      ordering and opens no session; `moca_HardwareEquipped()` is the only readiness
 *      check available, as `Initialization and Startup` in the repository specification
 *      in the HAL specification states. The pointer argument must address
 *      caller-allocated storage of the declared type. This interface does not state what
 *      an implementation does with a null pointer, so passing one is not covered by this
 *      contract and is not reported through a distinct return code.
 * @post On `STATUS_SUCCESS` the structure holds the interface's fixed properties. Because
 *       they are static, a caller may read them once and cache them for the lifetime of the
 *       interface. On `STATUS_FAILURE` this interface does not state whether any member was
 *       written, so the whole structure must be treated as unusable.
 *
 * @returns INT - the outcome of the read, reported synchronously as `Internal Error
 *          Handling` in the repository specification requires.
 * @retval STATUS_SUCCESS - The static information was read.
 * @retval STATUS_FAILURE - The operation failed. This interface does not distinguish the
 *         reason, so the client can only retry, fall back, or report the failure; it
 *         cannot branch on a cause, and it treats any non-zero value as a failure of this
 *         class because the code set is open.
 *
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification makes
 *       its non-blocking rule a steady-state one - a ready implementation must not suspend
 *       the caller's main thread - and excepts the start-up window before the vendor
 *       subsystem is ready, where a call may not return promptly and a caller bounds it.
 *       This is often the first substantive call a caller makes, so it is the one most
 *       likely to meet that start-up window.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process, so a vendor
 *       implementation guards its own state.
 *
 * @see moca_static_info_t
 * @see moca_IfGetDynamicInfo
 * @see moca_HardwareEquipped
 */
INT moca_IfGetStaticInfo(ULONG ifIndex, moca_static_info_t *pmoca_static_info);

/**
 * @brief Reads the network-layer traffic counters of a MoCA interface into a
 *        caller-supplied structure.
 *
 * Populates the caller's `moca_stats_t` with the byte, packet, error and discard counts in
 * each direction, the unicast, multicast and broadcast breakdowns, the unknown-protocol
 * receive count and the two vendor-specific aggregation averages. These are network-layer
 * counters; `moca_IfGetExtCounter()` reads the MoCA MAC-layer control-packet counters and
 * `moca_IfGetExtAggrCounter()` the aggregate PDU totals.
 *
 * @param[in] ifIndex Index of the MoCA interface to query.
 *                    `moca_cfg_t.InstanceNumber` states the range this interface uses for
 *                    an interface index: 0 where a single MoCA interface is present, 1 to
 *                    256 where several are. This interface does not state what an
 *                    implementation does with an index outside that range, and no return
 *                    code is reserved for it.
 * @param[out] pmoca_stats Pointer to a caller-allocated `moca_stats_t` that the
 *                         implementation fills in. It must be non-NULL. Every member is a
 *                         `ULONG`, whose width follows the target's data model, so the
 *                         representable range is 0 to at least (2^32)-1. **This interface
 *                         does not state whether a counter wraps at its maximum or
 *                         saturates, whether counters are reset by anything other than a
 *                         MoCA reset, or over what interval they accumulate**, so a caller
 *                         computing a rate must tolerate a value smaller than the previous
 *                         reading rather than assuming monotonic growth. The caller both
 *                         allocates and releases the storage, per the `Memory Model` topic
 *                         of the repository specification;
 *                         nothing here establishes that the implementation retains the
 *                         pointer beyond the call.
 *
 * @pre This interface declares no initialization or teardown call, so it imposes no call
 *      ordering and opens no session; `moca_HardwareEquipped()` is the only readiness
 *      check available, as `Initialization and Startup` in the repository specification
 *      in the HAL specification states. The pointer argument must address
 *      caller-allocated storage of the declared type. This interface does not state what
 *      an implementation does with a null pointer, so passing one is not covered by this
 *      contract and is not reported through a distinct return code.
 * @post On `STATUS_SUCCESS` the structure holds a snapshot of the counters. This interface
 *       does not state that the members are sampled atomically with respect to each other,
 *       so a caller should not assume internal consistency between two counters in one
 *       read. On `STATUS_FAILURE` no member may be relied on.
 *
 * @returns INT - the outcome of the read, reported synchronously as `Internal Error
 *          Handling` in the repository specification requires.
 * @retval STATUS_SUCCESS - The statistics were read.
 * @retval STATUS_FAILURE - The operation failed. This interface does not distinguish the
 *         reason, so the client can only retry, fall back, or report the failure; it
 *         cannot branch on a cause, and it treats any non-zero value as a failure of this
 *         class because the code set is open.
 *
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification makes
 *       its non-blocking rule a steady-state one - a ready implementation must not suspend
 *       the caller's main thread - and excepts the start-up window before the vendor
 *       subsystem is ready, where a call may not return promptly and a caller bounds it.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process, so a vendor
 *       implementation guards its own state.
 *
 * @see moca_stats_t
 * @see moca_IfGetExtCounter
 * @see moca_IfGetExtAggrCounter
 * @see moca_GetResetCount
 */
INT moca_IfGetStats(ULONG ifIndex, moca_stats_t *pmoca_stats);

/**
 * @brief Reports how many devices are currently associated with the MoCA network.
 *
 * Writes the count of nodes on the network behind the given interface through the caller's
 * pointer. This is the count a caller needs before walking the device records with
 * `moca_GetAssociatedDevices()`, which reports no count of its own, and the reconciliation
 * point that `Asynchronous Notification Model` in the repository specification
 * in the HAL specification directs a caller to instead of treating the notification stream
 * as authoritative.
 *
 * @param[in] ifIndex Index of the MoCA interface to query.
 *                    `moca_cfg_t.InstanceNumber` states the range this interface uses for
 *                    an interface index: 0 where a single MoCA interface is present, 1 to
 *                    256 where several are. This interface does not state what an
 *                    implementation does with an index outside that range, and no return
 *                    code is reserved for it.
 * @param[out] pulCount Pointer to a caller-allocated `ULONG` that receives the number of
 *                      associated devices. It must be non-NULL. The value is a count of
 *                      nodes, not an index: `kMoca_MaxMocaNodes` (16) is the maximum node
 *                      count this interface publishes for a MoCA network, and
 *                      `moca_dynamic_info_t.NumberOfConnectedClients` documents a 0 to 256
 *                      range for the connected-client count it reports separately. **This
 *                      interface does not state which of those bounds applies here**, so a
 *                      caller must not use the value to index a fixed array without
 *                      checking it against the capacity it actually allocated. The caller
 *                      owns the storage; nothing here establishes that the implementation
 *                      retains the pointer beyond the call.
 *
 * @pre This interface declares no initialization or teardown call, so it imposes no call
 *      ordering and opens no session; `moca_HardwareEquipped()` is the only readiness
 *      check available, as `Initialization and Startup` in the repository specification
 *      in the HAL specification states. The pointer argument must address
 *      caller-allocated storage of the declared type. This interface does not state what
 *      an implementation does with a null pointer, so passing one is not covered by this
 *      contract and is not reported through a distinct return code.
 * @post On `STATUS_SUCCESS` `*pulCount` holds the count at the moment of the read. **The
 *       count can change immediately afterwards** - a node may join or leave - so a caller
 *       that uses it to size or bound a later `moca_GetAssociatedDevices()` is working from
 *       a value that may already be wrong. On `STATUS_FAILURE` this interface does not
 *       state whether `*pulCount` was written, so it must not be read.
 *
 * @returns INT - the outcome of the read, reported synchronously as `Internal Error
 *          Handling` in the repository specification requires.
 * @retval STATUS_SUCCESS - The count was read.
 * @retval STATUS_FAILURE - The operation failed. This interface does not distinguish the
 *         reason, so the client can only retry, fall back, or report the failure; it
 *         cannot branch on a cause, and it treats any non-zero value as a failure of this
 *         class because the code set is open.
 *
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification makes
 *       its non-blocking rule a steady-state one - a ready implementation must not suspend
 *       the caller's main thread - and excepts the start-up window before the vendor
 *       subsystem is ready, where a call may not return promptly and a caller bounds it.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process, so a vendor
 *       implementation guards its own state.
 *       The node set is changed by the network rather than by the caller, so serialising
 *       calls does not make a count stable.
 *
 * @see moca_GetAssociatedDevices
 * @see moca_GetMocaCPEs
 */
INT moca_GetNumAssociatedDevices(ULONG ifIndex, ULONG *pulCount);

/**
 * @brief Reads the MoCA MAC-layer control-packet counters of an interface into a
 *        caller-supplied structure.
 *
 * Populates the caller's `moca_mac_counters_t` with the receive counts for each class of
 * MoCA control traffic: MAP packets, reservation requests, link control, admission
 * requests, probes and asynchronous beacons. These are MoCA protocol counters, distinct
 * from the network-layer traffic counters `moca_IfGetStats()` reads and from the aggregate
 * payload totals `moca_IfGetExtAggrCounter()` reads.
 *
 * @param[in] ifIndex Index of the MoCA interface to query.
 *                    `moca_cfg_t.InstanceNumber` states the range this interface uses for
 *                    an interface index: 0 where a single MoCA interface is present, 1 to
 *                    256 where several are. This interface does not state what an
 *                    implementation does with an index outside that range, and no return
 *                    code is reserved for it.
 * @param[out] pmoca_mac_counters Pointer to a caller-allocated `moca_mac_counters_t` that
 *                                the implementation fills in. It must be non-NULL. All six
 *                                members are `ULONG` receive counts, whose width follows
 *                                the target's data model, so the representable range is 0
 *                                to at least (2^32)-1; there are no transmit counterparts
 *                                in this structure. **This interface does not state whether
 *                                a counter wraps or saturates at its maximum, or over what
 *                                interval it accumulates**, so a caller computing a rate
 *                                must tolerate a value smaller than the previous reading.
 *                                The caller both allocates and releases the storage, per
 *                                the `Memory Model` topic of the repository specification
 *                                in the HAL specification; nothing here establishes that
 *                                the implementation retains the pointer beyond the call.
 *
 * @pre This interface declares no initialization or teardown call, so it imposes no call
 *      ordering and opens no session; `moca_HardwareEquipped()` is the only readiness
 *      check available, as `Initialization and Startup` in the repository specification
 *      in the HAL specification states. The pointer argument must address
 *      caller-allocated storage of the declared type. This interface does not state what
 *      an implementation does with a null pointer, so passing one is not covered by this
 *      contract and is not reported through a distinct return code.
 * @post On `STATUS_SUCCESS` the structure holds a snapshot of the six counters; this
 *       interface does not state that they are sampled atomically with respect to each
 *       other. On `STATUS_FAILURE` no member may be relied on.
 *
 * @returns INT - the outcome of the read, reported synchronously as `Internal Error
 *          Handling` in the repository specification requires.
 * @retval STATUS_SUCCESS - The MAC-layer counters were read.
 * @retval STATUS_FAILURE - The operation failed. This interface does not distinguish the
 *         reason, so the client can only retry, fall back, or report the failure; it
 *         cannot branch on a cause, and it treats any non-zero value as a failure of this
 *         class because the code set is open.
 *
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification makes
 *       its non-blocking rule a steady-state one - a ready implementation must not suspend
 *       the caller's main thread - and excepts the start-up window before the vendor
 *       subsystem is ready, where a call may not return promptly and a caller bounds it.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process, so a vendor
 *       implementation guards its own state.
 *
 * @see moca_mac_counters_t
 * @see moca_IfGetStats
 * @see moca_IfGetExtAggrCounter
 */
INT moca_IfGetExtCounter(ULONG ifIndex, moca_mac_counters_t *pmoca_mac_counters);

/**
 * @brief Reads the aggregate transmitted and received payload data unit totals of a MoCA
 *        interface into a caller-supplied structure.
 *
 * Populates the caller's `moca_aggregate_counters_t` with two totals: payload data units
 * transmitted and payload data units received, both **excluding** MoCA control packets.
 * Control traffic is counted separately by `moca_IfGetExtCounter()`, so the two calls
 * partition the traffic rather than overlapping.
 *
 * @param[in] ifIndex Index of the MoCA interface to query.
 *                    `moca_cfg_t.InstanceNumber` states the range this interface uses for
 *                    an interface index: 0 where a single MoCA interface is present, 1 to
 *                    256 where several are. This interface does not state what an
 *                    implementation does with an index outside that range, and no return
 *                    code is reserved for it.
 * @param[out] pmoca_aggregate_counts Pointer to a caller-allocated
 *                                    `moca_aggregate_counters_t` that the implementation
 *                                    fills in. It must be non-NULL. Both members are
 *                                    `ULONG` PDU counts - not byte counts - whose width
 *                                    follows the target's data model, so the representable
 *                                    range is 0 to at least (2^32)-1. **This interface does
 *                                    not state whether a counter wraps or saturates at its
 *                                    maximum, or over what interval it accumulates.** The
 *                                    caller both allocates and releases the storage, per
 *                                    the `Memory Model` topic of the repository
 *                                    specification; nothing here
 *                                    establishes that the implementation retains the
 *                                    pointer beyond the call.
 *
 * @pre This interface declares no initialization or teardown call, so it imposes no call
 *      ordering and opens no session; `moca_HardwareEquipped()` is the only readiness
 *      check available, as `Initialization and Startup` in the repository specification
 *      in the HAL specification states. The pointer argument must address
 *      caller-allocated storage of the declared type. This interface does not state what
 *      an implementation does with a null pointer, so passing one is not covered by this
 *      contract and is not reported through a distinct return code.
 * @post On `STATUS_SUCCESS` both totals are populated; this interface does not state that
 *       they are sampled atomically with respect to each other. On `STATUS_FAILURE` neither
 *       may be relied on.
 *
 * @returns INT - the outcome of the read, reported synchronously as `Internal Error
 *          Handling` in the repository specification requires.
 * @retval STATUS_SUCCESS - The aggregate counters were read.
 * @retval STATUS_FAILURE - The operation failed. This interface does not distinguish the
 *         reason, so the client can only retry, fall back, or report the failure; it
 *         cannot branch on a cause, and it treats any non-zero value as a failure of this
 *         class because the code set is open.
 *
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification makes
 *       its non-blocking rule a steady-state one - a ready implementation must not suspend
 *       the caller's main thread - and excepts the start-up window before the vendor
 *       subsystem is ready, where a call may not return promptly and a caller bounds it.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process, so a vendor
 *       implementation guards its own state.
 *
 * @see moca_aggregate_counters_t
 * @see moca_IfGetExtCounter
 * @see moca_IfGetStats
 */
INT moca_IfGetExtAggrCounter(ULONG ifIndex, moca_aggregate_counters_t *pmoca_aggregate_counts);

/**
 * @brief Reads the MAC addresses of every MoCA CPE node on the network into a
 *        caller-supplied array.
 *
 * Fills the caller's `cpes` array with one `moca_cpe_t` per MoCA Customer Premises
 * Equipment node reachable on the network behind the given interface, and reports how
 * many entries it wrote through `pnum_cpes`. This is the node-address view of the
 * network; `moca_GetAssociatedDevices` is the full per-node record for the same set.
 *
 * @param[in] ifIndex Index of the MoCA interface to query. `moca_cfg_t.InstanceNumber`
 *                    states the range this interface uses for an interface index: 0 for
 *                    a single interface, 1 to 256 where several are present. This
 *                    interface does not state what an implementation does with an index
 *                    outside that range.
 * @param[out] cpes Caller-allocated array of `moca_cpe_t` that the implementation fills
 *                  in. It must be non-NULL and must have capacity for
 *                  `kMoca_MaxCpeList` (256) entries, which is the one array capacity
 *                  this interface states outright and therefore the only safe
 *                  allocation: the implementation writes one entry per node found and
 *                  this interface gives the caller no way to declare a smaller
 *                  capacity, so allocating fewer than 256 entries admits a write past
 *                  the end of the array. Each entry's `mac_addr` member is six raw
 *                  address octets, not text. The caller allocates and releases this
 *                  storage - the `Memory Model` topic of the repository specification
 *                  in the HAL specification places every output buffer on the caller
 *                  and states that the interface declares no allocator and no matching
 *                  release function. This interface does not specify whether the
 *                  implementation retains the pointer beyond the call, so the caller
 *                  keeps ownership and conservatively keeps the array valid.
 * @param[out] pnum_cpes Pointer to a caller-allocated `INT` that receives the number of
 *                       entries written to `cpes`. It must be non-NULL. This count, not
 *                       the array capacity, bounds what a caller may read: entries from
 *                       `*pnum_cpes` to 255 are not written by this call and must not be
 *                       read. This interface does not state what an implementation does
 *                       when more than `kMoca_MaxCpeList` nodes are present, so a caller
 *                       must not assume the count is clamped rather than the array
 *                       overrun.
 *
 * @pre This interface declares no initialization or teardown call, so it imposes no call
 *      ordering and opens no session; `moca_HardwareEquipped()` is the only readiness
 *      check available, as `Initialization and Startup` in the repository specification
 *      states. Both pointers must address caller-allocated storage of the declared type
 *      and `cpes` must have the capacity above. This interface does not state what an
 *      implementation does with a null pointer or an undersized array, so neither is
 *      covered by this contract and neither is reported through a distinct return code.
 * @post On `STATUS_SUCCESS` the first `*pnum_cpes` entries of `cpes` are populated. On
 *       `STATUS_FAILURE` this interface does not state whether either output was
 *       written, so the caller must treat both the count and the whole array as
 *       unusable.
 *
 * @returns INT - the outcome of the read, reported synchronously as `Internal Error
 *          Handling` in the repository specification requires.
 * @retval STATUS_SUCCESS - The node list was read; the caller may read `cpes` up to
 *         `*pnum_cpes` entries.
 * @retval STATUS_FAILURE - The read failed. This interface does not distinguish the
 *         reason, so the client can only retry, fall back to
 *         `moca_GetNumAssociatedDevices` for a count alone, or report the failure; it
 *         cannot branch on a cause. The header records that the codes are intended to be
 *         extended with specific reasons, so a client treats any non-zero value as a
 *         failure of this class rather than assuming the set is closed.
 *
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification makes
 *       its non-blocking rule a steady-state one - a ready implementation must not suspend
 *       the caller's main thread - and excepts the start-up window before the vendor
 *       subsystem is ready, where a call may not return promptly and a caller bounds it.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process, so a vendor
 *       implementation guards its own state.
 *
 * @see moca_GetNumAssociatedDevices
 * @see moca_GetAssociatedDevices
 * @see moca_cpe_t
 */
INT moca_GetMocaCPEs(ULONG ifIndex, moca_cpe_t *cpes, INT *pnum_cpes);

/**
 * @brief Reads the full per-node record of every device associated with the MoCA
 *        network into a caller-owned array.
 *
 * Populates one `moca_associated_device_t` per device on the network behind the given
 * interface, carrying each node's MAC address, node ID, `PHY` transmit and receive
 * rates, power levels, packet counters and capability flags. It is the detailed
 * counterpart of `moca_GetMocaCPEs`, and the reconciliation point that `Asynchronous
 * Notification Model` in the repository specification directs
 * a caller to when the notification stream cannot be treated as authoritative.
 *
 * @param[in] ifIndex Index of the MoCA interface to query. `moca_cfg_t.InstanceNumber`
 *                    states the range this interface uses: 0 for a single interface, 1
 *                    to 256 where several are present. This interface does not state
 *                    what an implementation does with an index outside that range.
 * @param[out] ppdevice_array Address of the caller's `moca_associated_device_t *`, whose
 *                            pointee array the implementation fills in. The caller owns
 *                            that array: it allocates it and sets `*ppdevice_array` to
 *                            point at it before the call, and it releases it afterwards.
 *                            The `Memory Model` topic of the repository specification
 *                            states this for both pointer-to-pointer outputs in this
 *                            interface, and states that no function returns allocated
 *                            storage the caller must free and that the interface
 *                            declares no allocator and no matching release function.
 *                            **A caller must therefore not read `*ppdevice_array` as a
 *                            pointer the implementation allocated, and must not attempt
 *                            to release it with any function of this interface: there is
 *                            none, and freeing storage the implementation owns, or
 *                            failing to free storage the caller owns, are both defects
 *                            this interface gives no means to detect.**
 *                            **Neither the element count nor the capacity is established
 *                            by this interface.** This call writes back no count, takes no
 *                            capacity argument, and no declaration ties the array to a
 *                            macro. The counts this header declares belong to other things
 *                            - `kMoca_MaxMocaNodes` (16) is a network's node maximum,
 *                            `kMoca_MaxCpeList` (256) bounds the `moca_GetMocaCPEs` array,
 *                            and `moca_dynamic_info_t` documents 0 to 256 connected
 *                            clients - so neither 16 nor any other figure here is a safe
 *                            size for this array. Where this interface means a capacity it
 *                            states one, as it does for `moca_GetMocaCPEs`; the silence
 *                            here is silence. **The capacity must be agreed with the vendor
 *                            implementation before this call is made**; until it is, any
 *                            allocation admits a write past the end that no return code
 *                            reports. This interface does not specify whether the
 *                            implementation retains the pointer, so ownership stays here.
 *
 * @pre This interface declares no initialization or teardown call, so it imposes no call
 *      ordering and opens no session; `moca_HardwareEquipped()` is the only readiness
 *      check available. `ppdevice_array` must be non-NULL and `*ppdevice_array` must
 *      address caller-allocated storage of the declared type with the capacity discussed
 *      above. This interface does not state what an implementation does with a null
 *      pointer or an undersized array, so neither is covered by this contract.
 * @post On `STATUS_SUCCESS` the array is populated, but **this call reports no count of the
 *       entries it wrote**, and nothing relates the count `moca_GetNumAssociatedDevices`
 *       reports - a separate observation taken at a different instant - to what this call
 *       wrote. A caller therefore has no interface-established bound on how much of the
 *       array to read. On `STATUS_FAILURE` this interface does not state whether any part
 *       of the array was written, so the whole array must be treated as unusable.
 *
 * @returns INT - the outcome of the read, reported synchronously as `Internal Error
 *          Handling` in the repository specification requires.
 * @retval STATUS_SUCCESS - The device records were read, subject to the count caveat
 *         above.
 * @retval STATUS_FAILURE - The read failed. This interface does not distinguish the
 *         reason, so the client can only retry, fall back to
 *         `moca_GetNumAssociatedDevices` for a count alone, or report the failure. The
 *         header records that the codes are intended to be extended with specific
 *         reasons, so a client treats any non-zero value as a failure of this class.
 *
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification scopes
 *       its non-blocking rule to steady state and excepts the start-up window before the
 *       vendor subsystem is ready, where a call may not return promptly; a caller bounds it.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and the
 *       node set can change under a caller between the count call and this one, so a
 *       caller that walks the array must serialise both calls against its own
 *       notification path as well.
 *
 * @warning The entry count is not reported by this call, and the count read through
 *          `moca_GetNumAssociatedDevices` is a separate observation this interface does not
 *          relate to this one. Sizing or bounding the array from it risks a write past the
 *          end and a read of entries this call never wrote; no return code reports either.
 *
 * @see moca_GetNumAssociatedDevices
 * @see moca_GetMocaCPEs
 * @see moca_associated_device_t
 */
INT moca_GetAssociatedDevices(ULONG ifIndex, moca_associated_device_t **ppdevice_array);

/**
 * @brief Converts a MoCA frequency bitmask into the frequency value it denotes.
 *
 * A utility for interpreting the frequency masks this interface exposes as opaque byte
 * arrays - `FreqCurrentMaskSetting`, `NodeTabooMask` and `ChannelScanMask` in `moca_cfg_t`,
 * `FreqCapabilityMask` and `NetworkTabooMask` in `moca_static_info_t`, and `FreqCurrentMask`
 * in `moca_dynamic_info_t`. It reads no hardware state. `Initialization and Startup` in the
 * repository specification lists it among the routine monitoring
 * and mask interpretation calls.
 *
 * @param[in] mask Pointer to a caller-owned array of `UCHAR` holding the frequency mask.
 *                 It must be non-NULL. **The number of bytes this call reads is not
 *                 established by this interface**: the declaration carries no length
 *                 parameter, and the only figure stated anywhere is that the buffer
 *                 "should be at least 16 bytes" and is vendor-dependent - a rule for the
 *                 buffer a caller supplies, not a width the interface commits to reading.
 *                 The two cannot be reconciled: `FreqCapabilityMask` and `FreqCurrentMask`
 *                 are 8 bytes each, while the four 128-byte masks named above are larger.
 *                 **The width must therefore be established with the vendor implementation
 *                 before any mask member is passed**, because passing an 8-byte member to
 *                 an implementation that reads 16 is a read past the end of that member,
 *                 which no return code reports and which this interface gives no length
 *                 argument to prevent. **Retention is not specified either**: `Memory
 *                 Model` in the repository specification asks a caller to treat a buffer
 *                 as valid for the call only, but binds no implementation, so a caller
 *                 cannot assume it is safe to free or overwrite this one on return.
 *
 * @pre This interface declares no initialization or teardown call, so it opens no session
 *      and imposes no ordering. `mask` must address at least as many readable bytes as the
 *      implementation consumes; a null or a short buffer is not covered by this contract.
 * @post None. The call computes a value and changes no state.
 *
 * @return The frequency value the mask denotes, as a signed `INT`. **This is a value, not a
 *         status code, and it must not be compared against `STATUS_SUCCESS` or
 *         `STATUS_FAILURE`.** The unit of the value, its valid range and the mask
 *         interpretation that produces it are all vendor-specific and are not stated by this
 *         interface. **No sentinel is defined for failure**: this interface does not state
 *         what is returned when the mask denotes no frequency or cannot be interpreted, so a
 *         caller cannot distinguish a failed conversion from a successful one that yields 0
 *         or a negative value, and must not treat any particular return as an error.
 *
 * @note Blocking: this is a computation over the caller's buffer with no hardware access,
 *       so it returns without waiting and the start-up window that `Blocking calls` in the
 *       repository specification excepts does not arise here. That topic's non-blocking rule
 *       is a steady-state obligation on every call in this interface, and states no timeout.
 * @note Thread safety: this interface is not thread safe and `Threading Model` in the
 *       repository specification places serialisation on the calling module. This call reads
 *       only the caller's own buffer, but that is an observation about its signature and not
 *       a guarantee this interface makes, so a caller must not rely on it being reentrant.
 *
 * @see moca_cfg_t
 * @see moca_static_info_t
 */
INT moca_FreqMaskToValue(UCHAR *mask);

/**
 * @brief Reports whether MoCA hardware is present and correctly configured on this system.
 *
 * This is the readiness check a caller makes before anything else. Because this interface
 * declares no initialization call and no return code meaning "not initialized yet", it is
 * the only readiness signal available - `Initialization and Startup` in the repository
 * specification puts it first in a caller's normal bring-up
 * order, ahead of `moca_IfGetStaticInfo()`.
 *
 * @pre None. It takes no argument, requires no prior call, and is the one call in this
 *      interface a caller may safely make first.
 * @post None. It reports presence and changes nothing.
 *
 * @return `TRUE` if MoCA hardware is equipped and correctly configured, `FALSE` otherwise.
 *         Those two macros are the whole of the stated result: this header defines `TRUE`
 *         as 1 and `FALSE` as 0, and `BOOL` as `unsigned char`, so the value is a single
 *         byte carrying 1 or 0. Both definitions are `#ifndef`-guarded, so a translation
 *         unit that defines either name before including this header changes what a
 *         returned value means - compare against the macros, not against literals. **This
 *         is a value, not a status code**: this interface defines no third outcome and no
 *         sentinel, so a caller cannot tell a failed call from a genuine `FALSE` - `FALSE`
 *         means "do not proceed" rather than "no hardware".
 *
 * @note Blocking: `Blocking calls` in the repository specification makes its non-blocking
 *       rule a steady-state one and excepts the start-up window before the vendor subsystem
 *       is ready. Because this is normally the first call made, it is the one most likely to
 *       meet that window, where a call may not return promptly and no timeout is stated.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process.
 * @note This interface does not state whether the result can change during the lifetime of
 *       the process - hardware presence is not declared to be static - so a caller that
 *       caches a `TRUE` result should be prepared for a later call to fail rather than
 *       treating the check as permanent.
 *
 * @see moca_IfGetStaticInfo
 */
BOOL moca_HardwareEquipped(void);

#ifndef MOCA_VAR
/**
 * @brief Reads the unicast transmit `PHY` rate between every pair of MoCA nodes into a
 *        caller-supplied table.
 *
 * Fills the caller's array with one `moca_mesh_table_t` entry per ordered node pair on
 * the network behind the given interface, each carrying the transmitting and receiving
 * node IDs and the transmit rate between them, plus the MoCA 2.x NPER and VLPER rates.
 * This is the network-wide rate view; the per-node rates a single device reports are in
 * `moca_associated_device_t`.
 *
 * @param[in] ifIndex Index of the MoCA interface to query. `moca_cfg_t.InstanceNumber`
 *                    states the range this interface uses: 0 for a single interface, 1
 *                    to 256 where several are present. This interface does not state
 *                    what an implementation does with an index outside that range.
 * @param[out] pDeviceArray Caller-allocated array of `moca_mesh_table_t` that the
 *                          implementation fills in. It must be non-NULL. **The capacity
 *                          rule this interface states is "enough entries for all
 *                          possible node pairs", and it states no figure.** With
 *                          `kMoca_MaxMocaNodes` (16) as the published node maximum,
 *                          all ordered pairs come to at most 16 x 16 = 256 entries, so
 *                          that is the allocation a caller must make; the arithmetic is
 *                          this documentation's, not a figure the interface publishes,
 *                          and a caller that allocates fewer entries relies on an
 *                          assumption this interface does not support. There is no
 *                          in-parameter by which a caller can declare a smaller
 *                          capacity, so an undersized array admits a write past its end.
 *                          The caller allocates and releases this storage - the `Memory
 *                          Model` topic of the repository specification
 *                          in the HAL specification places every output buffer on the
 *                          caller and states that the interface declares no allocator and
 *                          no matching release function. Nothing here establishes that
 *                          the implementation retains the pointer beyond the call.
 * @param[out] pulCount Pointer to a caller-allocated `ULONG` that receives the number of
 *                      entries written to `pDeviceArray`. It must be non-NULL. This
 *                      count, not the allocated capacity, bounds what a caller may read.
 *
 * @pre This interface declares no initialization or teardown call, so it imposes no call
 *      ordering and opens no session; `moca_HardwareEquipped()` is the only readiness
 *      check available. Both pointers must address caller-allocated storage of the
 *      declared type and the array must have the capacity above. This interface does not
 *      state what an implementation does with a null pointer or an undersized array, so
 *      neither is covered by this contract.
 * @post On `STATUS_SUCCESS` the first `*pulCount` entries of `pDeviceArray` are
 *       populated. On `STATUS_FAILURE` this interface does not state whether either
 *       output was written, so the caller must treat both the count and the whole array
 *       as unusable.
 *
 * @returns INT - the outcome of the read, reported synchronously as `Internal Error
 *          Handling` in the repository specification requires.
 * @retval STATUS_SUCCESS - The mesh rate table was read; the caller may read
 *         `pDeviceArray` up to `*pulCount` entries.
 * @retval STATUS_FAILURE - The read failed. This interface does not distinguish the
 *         reason, so the client can only retry, fall back to the per-node rates in
 *         `moca_GetAssociatedDevices`, or report the failure.
 *
 * @note This declaration exists only where the `MOCA_VAR` macro is **not** defined, and
 *       so does the `moca_mesh_table_t` type it returns entries of. Declaration and type
 *       are guarded together, so this function is the coherent case of the two
 *       `MOCA_VAR` guards in this header; see the warning on
 *       `moca_IfGetDynamicInfo()` for the one that is not. `Platform or Product
 *       Customization` in the repository specification carries the full exclusion list.
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification scopes
 *       its non-blocking rule to steady state and excepts the start-up window before the
 *       vendor subsystem is ready. Reading a full mesh is also proportionally the most
 *       expensive read here, so a caller that needs a bound on either case imposes it.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process.
 *
 * @see moca_GetAssociatedDevices
 * @see moca_mesh_table_t
 */
INT moca_GetFullMeshRates(ULONG ifIndex, moca_mesh_table_t *pDeviceArray, ULONG *pulCount);
#endif

/**
 * @brief Reads the ingress PQoS flow table of a MoCA interface into a caller-supplied
 *        array.
 *
 * Fills the caller's array with one `moca_flow_table_t` entry per ingress Priority
 * Quality of Service flow on the given interface, each carrying the flow's ID, its
 * ingress and egress node IDs, its destination MAC address, its lease and remaining
 * lease times and its rate and burst characteristics.
 *
 * @param[in] ifIndex Index of the MoCA interface to query. `moca_cfg_t.InstanceNumber`
 *                    states the range this interface uses: 0 for a single interface, 1
 *                    to 256 where several are present. This interface does not state
 *                    what an implementation does with an index outside that range.
 * @param[out] pDeviceArray Caller-allocated array of `moca_flow_table_t` that the
 *                          implementation fills in. It must be non-NULL. **This interface
 *                          states no capacity for this array and publishes no maximum
 *                          flow count.** The header asks only for "enough memory ... to
 *                          hold all entries", `pulCount` is an output and so cannot be
 *                          used to size the allocation, and no macro in this header
 *                          bounds the flow table the way `kMoca_MaxCpeList` bounds the
 *                          CPE list. **The required capacity is therefore not specified
 *                          by this interface**, and a caller cannot derive it from the
 *                          declarations: it must be agreed with the vendor implementation
 *                          before this call is made. Until it is, any allocation admits a
 *                          write past the end of the array, and this interface offers the
 *                          caller no bound to enforce and no in-parameter with which to
 *                          declare one. The caller allocates and releases the storage -
 *                          the `Memory Model` topic of the repository specification
 *                          in the HAL specification places every output buffer on the
 *                          caller and states that the interface declares no allocator and
 *                          no matching release function.
 * @param[out] pulCount Pointer to a caller-allocated `ULONG` that receives the number of
 *                      entries written to `pDeviceArray`. It must be non-NULL. This count
 *                      bounds what a caller may read, and it is also the only signal a
 *                      caller gets about how much of its array the implementation used -
 *                      a value larger than the capacity allocated means the array has
 *                      already been overrun.
 *
 * @pre This interface declares no initialization or teardown call, so it imposes no call
 *      ordering and opens no session; `moca_HardwareEquipped()` is the only readiness
 *      check available. Both pointers must address caller-allocated storage of the
 *      declared type. This interface does not state what an implementation does with a
 *      null pointer or an undersized array, so neither is covered by this contract.
 * @post On `STATUS_SUCCESS` the first `*pulCount` entries of `pDeviceArray` are
 *       populated, and a caller should compare `*pulCount` against the capacity it
 *       allocated before reading any of them. On `STATUS_FAILURE` this interface does not
 *       state whether either output was written, so the caller must treat both the count
 *       and the whole array as unusable.
 *
 * @returns INT - the outcome of the read, reported synchronously as `Internal Error
 *          Handling` in the repository specification requires.
 * @retval STATUS_SUCCESS - The flow table was read; the caller may read `pDeviceArray`
 *         up to `*pulCount` entries.
 * @retval STATUS_FAILURE - The read failed. This interface does not distinguish the
 *         reason, so the client can only retry or report the failure; it cannot branch on
 *         a cause.
 *
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification scopes
 *       its non-blocking rule to steady state and excepts the untimed start-up window.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process. Flows are also
 *       created and torn down by the network rather than by the caller, so the table can
 *       change between successive reads.
 *
 * @warning The capacity this call requires is not stated by this interface. Sizing the
 *          array by guess is what makes an out-of-bounds write possible here, and no
 *          return code reports that it happened.
 *
 * @see moca_flow_table_t
 */
INT moca_GetFlowStatistics(ULONG ifIndex, moca_flow_table_t *pDeviceArray, ULONG *pulCount);

/**
 * @brief Reports how many times the MoCA module has been reset.
 *
 * Writes the reset count through the caller's pointer. It is the one call in this interface
 * that takes no interface index, so it reports for the MoCA module as a whole rather than
 * per interface. A caller uses it to detect that a reset has occurred between two reads of
 * the traffic counters, since `moca_IfGetStats()` does not say whether its counters were
 * cleared.
 *
 * @param[out] resetcnt Pointer to a caller-allocated `ULONG` that receives the reset count.
 *                      It must be non-NULL. `ULONG` is `unsigned long`, whose width follows
 *                      the target's data model, so the representable range is 0 to at least
 *                      (2^32)-1; this interface states no narrower valid range and does not
 *                      state whether the count survives a device reboot or counts only
 *                      resets since the module started. The caller owns the storage;
 *                      nothing here establishes that the implementation retains the pointer
 *                      beyond the call.
 *
 * @pre This interface declares no initialization or teardown call, so it imposes no call
 *      ordering and opens no session; `moca_HardwareEquipped()` is the only readiness check
 *      available. `resetcnt` must address caller-allocated storage of the declared type.
 *      This interface does not state what an implementation does with a null pointer, so
 *      passing one is not covered by this contract.
 * @post On `STATUS_SUCCESS` `*resetcnt` holds the count. **On failure the value pointed to
 *       by `resetcnt` is left unchanged** - this is the one function in the interface that
 *       states its failure post-condition, so a caller may safely pre-initialise the
 *       variable and read it back, though checking the return remains the correct practice.
 *
 * @returns INT - the outcome of the read, reported synchronously as `Internal Error
 *          Handling` in the repository specification requires.
 * @retval STATUS_SUCCESS - The reset count was read.
 * @retval STATUS_FAILURE - The read failed and `*resetcnt` was not modified. This interface
 *         does not distinguish the reason, so the client can only retry or report the
 *         failure; it cannot branch on a cause.
 *
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification makes
 *       its non-blocking rule a steady-state one - a ready implementation must not suspend
 *       the caller's main thread - and excepts the start-up window before the vendor
 *       subsystem is ready, where a call may not return promptly and a caller bounds it.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process, so a vendor
 *       implementation guards its own state.
 *
 * @see moca_IfGetStats
 */
INT moca_GetResetCount(ULONG *resetcnt);

/**
 * @brief Configures an Automatic Channel Adaptation run and, if asked to, starts it.
 *
 * Sets the ACA parameters for the given interface and begins the run when the
 * configuration's `ACAStart` member is `TRUE`. An ACA run assesses channels so that the
 * best operating channel for the MoCA network can be selected. The run is long-running but
 * this call is not: it returns once the run has been started, and the caller learns of
 * completion by polling `moca_getIfAcaStatus()`. `Blocking calls` in the repository
 * specification states this explicitly, and `State Diagram` gives
 * the resulting idle - running - completed cycle.
 *
 * @param[in] interfaceIndex Index of the MoCA interface on which to run ACA. The ACA calls
 *                           take an `int` index rather than the `ULONG` the read calls take;
 *                           this interface states no range for it and reserves no return
 *                           code for an out-of-range value.
 * @param[in] acaCfg Configuration for the run, **passed by value**, so the implementation
 *                   receives a copy and the caller's own structure is neither read after
 *                   the call returns nor written to. There is no pointer and therefore no
 *                   ownership or lifetime question here. Every member must be set: `NodeID`
 *                   selects the node to assess, an unknown node yielding `STATUS_NO_NODE`;
 *                   `Type` must be `PROBE_QUITE` or `PROBE_EVM`, any other value yielding
 *                   `STATUS_INVALID_PROBE`; `Channel` selects the channel, an invalid one
 *                   yielding `STATUS_INVALID_CHAN`; `ReportNodes` is a bitmask of the nodes
 *                   to include in the assessment, with the least significant bit denoting
 *                   node 0; and `ACAStart` decides whether this call starts the run or only
 *                   records the configuration. This interface states no valid range for
 *                   `NodeID` or `Channel` beyond those error codes.
 *
 * @pre This interface declares no initialization or teardown call, so it imposes no call
 *      ordering and opens no session. No ACA run may already be in progress on the
 *      interface: if one is, this call does not start another and reports
 *      `STATUS_INPROGRESS`. A caller that cannot tolerate that should read
 *      `moca_getIfAcaStatus()` first, or clear the way with `moca_cancelIfAca()`.
 * @post On `STATUS_SUCCESS` with `ACAStart` `TRUE` a run has begun and
 *       `moca_getIfAcaStatus()` reports its progress; with `ACAStart` `FALSE` the
 *       configuration has been recorded and no run has begun. On any failure this interface
 *       does not state whether the configuration was recorded, so a caller should read it
 *       back with `moca_getIfAcaConfig()` rather than assuming either outcome. Nothing is
 *       persisted - `Persistence Model` in the repository specification places persistence
 *       on the caller.
 *
 * @returns int - the outcome, reported synchronously as `Internal Error Handling` in the
 *          repository specification requires.
 * @retval STATUS_SUCCESS - The configuration was accepted, and the run was started if
 *         `ACAStart` was `TRUE`.
 * @retval STATUS_FAILURE - Configuration or initiation failed. The client retries or
 *         reports the failure; this interface does not distinguish the reason.
 * @retval STATUS_INPROGRESS - A run is already under way on this interface and no new one
 *         was started. The client polls `moca_getIfAcaStatus()` or calls
 *         `moca_cancelIfAca()` rather than retrying immediately.
 * @retval STATUS_NO_NODE - The `NodeID` in the configuration does not name a node on the
 *         network. The client re-reads the node set with `moca_GetAssociatedDevices()`
 *         before retrying.
 * @retval STATUS_INVALID_PROBE - `Type` is not one of the two `PROBE_TYPE` members. The
 *         client corrects the call site; retrying with the same value cannot succeed.
 * @retval STATUS_INVALID_CHAN - `Channel` is not a channel the implementation accepts. The
 *         client corrects the call site.
 *
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification makes
 *       its non-blocking rule a steady-state one - a ready implementation must not suspend
 *       the caller's main thread - and excepts the start-up window before the vendor
 *       subsystem is ready, where a call may not return promptly and a caller bounds it.
 *       The duration of the ACA run itself is not part of this call.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process, so a vendor
 *       implementation guards its own state.
 *       Two callers starting a run concurrently is the case this matters most for, and the
 *       second one sees `STATUS_INPROGRESS` rather than a queued request.
 *
 * @warning **`STATUS_INPROGRESS` is `-1`, the same value as `STATUS_FAILURE`, and
 *          `STATUS_NO_NODE` is `-2`, the same value as `STATUS_NOT_AVAILABLE`.** A caller
 *          therefore cannot distinguish "already running" from "failed" by the return value
 *          alone: it must read `moca_getIfAcaStatus()` and inspect the `stat` member of
 *          `moca_aca_stat_t`. `Internal Error Handling` in the repository specification
 *          records the collisions.
 *
 * @see moca_getIfAcaConfig
 * @see moca_getIfAcaStatus
 * @see moca_cancelIfAca
 * @see moca_aca_cfg_t
 */
int moca_setIfAcaConfig(int interfaceIndex, moca_aca_cfg_t acaCfg);

/**
 * @brief Reads back the Automatic Channel Adaptation configuration in force on an
 *        interface.
 *
 * Populates the caller's `moca_aca_cfg_t` with the parameters a previous
 * `moca_setIfAcaConfig()` recorded. It reads configuration only and changes nothing, so it
 * may be called in any ACA state - idle, running or completed - which is why `State Diagram`
 * in the repository specification places it on no transition.
 *
 * @param[in] interfaceIndex Index of the MoCA interface to query. The ACA calls take an
 *                           `int` index rather than the `ULONG` the read calls take; this
 *                           interface states no range for it and reserves no return code for
 *                           an out-of-range value.
 * @param[out] acaCfg Pointer to a caller-allocated `moca_aca_cfg_t` that the implementation
 *                    fills in. It must be non-NULL. The caller both allocates and releases
 *                    the storage, per the `Memory Model` topic of the repository
 *                    specification; nothing here establishes that the implementation retains
 *                    the pointer beyond the call. All five members are scalars, so there is
 *                    no array bound to observe. **This interface does not state what is
 *                    reported when no configuration has ever been set on the interface**, so
 *                    a caller must not read the returned values as defaults.
 *
 * @pre This interface declares no initialization or teardown call, so it imposes no call
 *      ordering and opens no session, and no ACA run need be configured or running.
 *      `acaCfg` must address caller-allocated storage of the declared type. This interface
 *      does not state what an implementation does with a null pointer, so passing one is not
 *      covered by this contract.
 * @post On `STATUS_SUCCESS` the structure holds the recorded configuration. **The `ACAStart`
 *       member read back here is a configuration flag, not a run-state indicator**: this
 *       interface does not state whether it reflects whether a run is currently under way,
 *       so a caller must use `moca_getIfAcaStatus()` to learn that. On `STATUS_FAILURE` no
 *       member may be relied on.
 *
 * @returns int - the outcome of the read, reported synchronously as `Internal Error
 *          Handling` in the repository specification requires.
 * @retval STATUS_SUCCESS - The configuration was read.
 * @retval STATUS_FAILURE - The operation failed. This interface does not distinguish the
 *         reason, so the client can only retry, fall back, or report the failure; it
 *         cannot branch on a cause, and it treats any non-zero value as a failure of this
 *         class because the code set is open.
 *
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification makes
 *       its non-blocking rule a steady-state one - a ready implementation must not suspend
 *       the caller's main thread - and excepts the start-up window before the vendor
 *       subsystem is ready, where a call may not return promptly and a caller bounds it.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process, so a vendor
 *       implementation guards its own state.
 *       A concurrent `moca_setIfAcaConfig()` can replace the configuration this call is
 *       reading.
 *
 * @see moca_setIfAcaConfig
 * @see moca_getIfAcaStatus
 * @see moca_aca_cfg_t
 */
int moca_getIfAcaConfig(int interfaceIndex, moca_aca_cfg_t *acaCfg);

/**
 * @brief Terminates an Automatic Channel Adaptation run that is under way.
 *
 * Stops a running ACA process on the given interface. **The call is idempotent**: if no run
 * is active it has no effect and still reports success, so a caller may use it
 * unconditionally to clear the way for a `moca_setIfAcaConfig()` that would otherwise be
 * refused with `STATUS_INPROGRESS`. `State Diagram` in the repository specification
 * in the HAL specification shows it as the running-to-idle transition.
 *
 * @param[in] interfaceIndex Index of the MoCA interface whose ACA run is to be cancelled.
 *                           The ACA calls take an `int` index rather than the `ULONG` the
 *                           read calls take; this interface states no range for it and
 *                           reserves no return code for an out-of-range value.
 *
 * @pre This interface declares no initialization or teardown call, so it imposes no call
 *      ordering and opens no session. No run need be in progress - the call is defined for
 *      the idle case and reports success there.
 * @post On `STATUS_SUCCESS` the cancellation was accepted, which by the return contract
 *       below covers both a run that was cancelled and no run having been active. **This
 *       interface does not state whether the run has finished stopping when the call
 *       returns**, so a caller that needs to know polls `moca_getIfAcaStatus()`. Nor does
 *       it state what happens to the statistics a partially completed run had collected,
 *       so a later `moca_getIfAcaStatus()` or `moca_getIfScmod()` result must not be read
 *       as the outcome of a completed run, and the configuration is not stated to be
 *       cleared. On `STATUS_FAILURE` this interface does not state whether it was stopped.
 *
 * @returns int - the outcome, reported synchronously as `Internal Error Handling` in the
 *          repository specification requires.
 * @retval STATUS_SUCCESS - The run was cancelled, **or no run was active**. The two cases
 *         are not distinguished, so a caller cannot learn from this call whether there had
 *         been anything to cancel.
 * @retval STATUS_FAILURE - The cancellation attempt failed. This interface does not
 *         distinguish the reason, so the client can only retry or report the failure; a
 *         subsequent `moca_setIfAcaConfig()` may still report `STATUS_INPROGRESS`.
 *
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification makes
 *       its non-blocking rule a steady-state one - a ready implementation must not suspend
 *       the caller's main thread - and excepts the start-up window before the vendor
 *       subsystem is ready, where a call may not return promptly and a caller bounds it.
 *       Cancelling is a control operation rather than a wait: it does not block for the run
 *       it stops, and the post-condition above states what is left unspecified about that.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process, so a vendor
 *       implementation guards its own state.
 *
 * @see moca_setIfAcaConfig
 * @see moca_getIfAcaStatus
 */
int moca_cancelIfAca(int interfaceIndex);


/**
 * @brief Reads the progress and results of an Automatic Channel Adaptation run into a
 *        caller-supplied structure.
 *
 * Populates the caller's `moca_aca_stat_t` with the configuration the run is using, the
 * run's own `stat` outcome, the total received power, the per-channel power profile and the
 * flag that says whether that profile is ready. It reports on a run that is **ongoing or
 * completed**, so it is both the completion poll for `moca_setIfAcaConfig()` - which
 * returns as soon as the run has started - and the way a caller reads the result
 * afterwards.
 *
 * @param[in] interfaceIndex Index of the MoCA interface to query. The ACA calls take an
 *                           `int` index rather than the `ULONG` the read calls take; this
 *                           interface states no range for it and reserves no return code for
 *                           an out-of-range value.
 * @param[out] pacaStat Pointer to a caller-allocated `moca_aca_stat_t` that the
 *                      implementation fills in. It must be non-NULL. The caller both
 *                      allocates and releases the storage, per the `Memory Model` topic of
 *                      the repository specification; nothing here establishes that the
 *                      implementation retains the pointer beyond the call. `ACAPowProfile`
 *                      is a fixed 512-element array of `INT` power levels in dBm, one per
 *                      channel, and it bounds what a caller may read: **this interface does
 *                      not state how many of the 512 elements carry a meaningful value**, so
 *                      a caller must not read the whole array as populated. `stat` carries
 *                      the run outcome as an untyped integer - 0 success, 1 bad channel, 2
 *                      no EVM probe, 3 failure, 4 in progress - and `ACATrapCompleted` is
 *                      `TRUE` only once the power profile is ready.
 *
 * @pre This interface declares no initialization or teardown call, so it imposes no call
 *      ordering and opens no session. A run should have been configured or started with
 *      `moca_setIfAcaConfig()`; **this interface does not state what is reported when none
 *      ever has been**, so a caller must not read the structure as an idle state. `pacaStat`
 *      must address caller-allocated storage of the declared type; this interface does not
 *      state what an implementation does with a null pointer.
 * @post On `STATUS_SUCCESS` the structure holds the run's status. `ACAPowProfile` should be
 *       read only once `ACATrapCompleted` is `TRUE`, and `stat` distinguishes in-progress
 *       from the success and failure outcomes. On `STATUS_FAILURE` no valid ACA information
 *       was obtained and no member may be relied on.
 *
 * @returns int - the outcome of the read, reported synchronously as `Internal Error
 *          Handling` in the repository specification requires. **It reports whether the read
 *          succeeded, not whether the ACA run succeeded** - the run's own outcome is the
 *          `stat` member.
 * @retval STATUS_SUCCESS - The status was read; inspect `stat` and `ACATrapCompleted` for
 *         the run's own outcome.
 * @retval STATUS_FAILURE - The read failed and no valid ACA information was obtained. This
 *         interface does not distinguish the reason, so the client can only retry or report
 *         the failure.
 *
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification makes
 *       its non-blocking rule a steady-state one - a ready implementation must not suspend
 *       the caller's main thread - and excepts the start-up window before the vendor
 *       subsystem is ready, where a call may not return promptly and a caller bounds it.
 *       It polls a run rather than waiting for one, so it does not block for the duration of
 *       the run; this interface specifies no polling interval, so a caller chooses its own.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process, so a vendor
 *       implementation guards its own state.
 *       A concurrent `moca_setIfAcaConfig()` or `moca_cancelIfAca()` can change the run this
 *       call is reporting on.
 *
 * @warning `stat` is the only way to separate `STATUS_INPROGRESS` from `STATUS_FAILURE` on
 *          the return of `moca_setIfAcaConfig()`, because those two macros share the value
 *          `-1`. `Internal Error Handling` in the repository specification records the
 *          collision.
 *
 * @see moca_setIfAcaConfig
 * @see moca_cancelIfAca
 * @see moca_getIfScmod
 * @see moca_aca_stat_t
 */
int moca_getIfAcaStatus(int interfaceIndex,moca_aca_stat_t *pacaStat);

/**
 * @brief Reads the per-subcarrier modulation statistics collected by an ACA run into a
 *        caller-owned array.
 *
 * Populates one `moca_scmod_stat_t` per node pair, each carrying the transmitting and
 * receiving node IDs, the channel the measurement was taken on, and the 512-element
 * modulation, NPER and VLPER profiles across the subcarriers of that channel. The
 * statistics are those collected **after** an Automatic Channel Adaptation run, so this
 * call follows `moca_setIfAcaConfig()` and a `moca_getIfAcaStatus()` that reports
 * completion.
 *
 * @param[in] interfaceIndex Index of the MoCA interface to query, in the same range the
 *                           other ACA calls use. This interface does not state what an
 *                           implementation does with an index outside it.
 * @param[out] pnumOfEntries Pointer to a caller-allocated `int` that receives the number
 *                           of entries written to `*ppscmodStat`. It must be non-NULL.
 *                           **It is an output, so it cannot be used to size the
 *                           allocation** - a caller learns the entry count only after the
 *                           implementation has already written that many entries. It
 *                           bounds what a caller may read, and a value larger than the
 *                           capacity the caller allocated means the array has already
 *                           been overrun.
 * @param[out] ppscmodStat Address of the caller's `moca_scmod_stat_t *`, whose pointee
 *                         array the implementation fills in. The caller owns that array:
 *                         it allocates it and sets `*ppscmodStat` to point at it before
 *                         the call, and it releases it afterwards. The `Memory Model`
 *                         topic of the repository specification
 *                         in the HAL specification states this for both
 *                         pointer-to-pointer outputs in this interface, and states that
 *                         no function returns allocated storage the caller must free and
 *                         that the interface declares no allocator and no matching
 *                         release function - so a caller must neither treat
 *                         `*ppscmodStat` as implementation-allocated storage to release,
 *                         nor look for a release entry point here, because there is
 *                         none.
 *                         **The required element count is not specified by this
 *                         interface.** `pnumOfEntries` reports it rather than supplying
 *                         it, no macro in this header bounds the SCMOD table, and no
 *                         declaration ties it to `kMoca_MaxMocaNodes` (16) even though
 *                         the entries are per node pair. The capacity must therefore be
 *                         agreed with the vendor implementation before this call is
 *                         made; until it is, any allocation admits a write past the end
 *                         of the array, and this interface gives the caller no bound to
 *                         enforce. Each entry is large - three 512-byte profiles plus
 *                         three `INT` members - so the allocation is not incidental.
 *                         Nothing here establishes that the implementation retains the
 *                         pointer beyond the call, so the caller keeps ownership.
 *
 * @pre An ACA run must have completed for the statistics to be meaningful; this interface
 *      does not state what is reported when none has, so a caller should read
 *      `moca_getIfAcaStatus()` first. `ACATrapCompleted` in `moca_aca_stat_t` indicates
 *      that the profile is ready. Both pointers must be non-NULL and `*ppscmodStat` must
 *      address caller-allocated storage of the declared type. This interface does not
 *      state what an implementation does with a null pointer or an undersized array, so
 *      neither is covered by this contract.
 * @post On `STATUS_SUCCESS` the first `*pnumOfEntries` entries of `*ppscmodStat` are
 *       populated, and a caller should compare that count against the capacity it
 *       allocated before reading any of them. On `STATUS_FAILURE` no valid SCMOD
 *       information was obtained and this interface does not state whether either output
 *       was written, so the caller must treat both as unusable.
 *
 * @returns int - the outcome of the read, reported synchronously as `Internal Error
 *          Handling` in the repository specification requires.
 * @retval STATUS_SUCCESS - The SCMOD statistics were read, subject to the count and
 *         capacity rules above.
 * @retval STATUS_FAILURE - The read failed and no valid SCMOD information was obtained.
 *         This interface does not distinguish the reason, so the client can only re-run
 *         the ACA process, retry, or report the failure; it cannot branch on a cause.
 *
 * @note Blocking: synchronous, with the outcome in the return value and no timeout stated
 *       anywhere in this interface. `Blocking calls` in the repository specification scopes
 *       its non-blocking rule to steady state. This call reports statistics an earlier ACA
 *       run collected and does not wait for a run, so the run duration is not part of it.
 * @note Thread safety: this interface is not thread safe. `Threading Model` in the
 *       repository specification places serialisation on the calling module, and states
 *       that the same call may be made from more than one process. A concurrent
 *       `moca_setIfAcaConfig()` can replace the statistics this call is reading.
 *
 * @warning The capacity this call requires is not stated by this interface, and the count
 *          arrives too late to size the allocation. Sizing the array by guess is what
 *          makes an out-of-bounds write possible here, and no return code reports that it
 *          happened.
 *
 * @see moca_getIfAcaStatus
 * @see moca_setIfAcaConfig
 * @see moca_scmod_stat_t
 */
int moca_getIfScmod(int interfaceIndex,int *pnumOfEntries,moca_scmod_stat_t **ppscmodStat);

/** @} */  //END OF GROUP MOCA_HAL_APIS
#endif

