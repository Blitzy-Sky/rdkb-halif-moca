# MoCA HAL Documentation

## Version History

| Date | Comment | Version |
| --- | --- | --- |
| 08/24/26 | Rewritten against the canonical HAL specification topic set. Page renamed from `MoCAHalSpec.md` to `halSpec.md`. Added `Version History`, `Optional Components`, `Data Structures and Defines`, `API Surface` and `State Diagram`; documented the asynchronous notification callback against the header for the first time; corrected the API narrative so that every named identifier is one the header declares. | 1.0.1 |
| 07/29/24 | Initial release of the MoCA HAL interface definition — `RDKB-52502` MoCA HAL header migration to GitHub, `moca_hal.h` update, and refinement following MTA HAL review comments. | 1.0.0 |

Four different version identities apply to this repository and are deliberately kept apart, because
a caller needs a different one in each case.

| Identity | Value | What it is |
| --- | --- | --- |
| Document revision | `1.0.1` | The revision of this specification, as recorded in the table above. |
| Interface version | Not declared | `moca_hal.h` defines **no** interface version macro. The `FirmwareVersion`, `HighestVersion` and `CurrentVersion` members it does declare report the vendor firmware and the MoCA **protocol** version of a node — they are not a version of this interface, and a caller cannot use them to determine which revision of the header a platform was built against. This interface offers no compile-time or run-time version check. |
| Release tag | `1.0.0` | The repository's only git tag, dated 29 July 2024, matching the single section in the changelog. This is the release this document describes. |
| Generated-site version string | `<tag>-<commits since that tag>-g<abbreviated commit>` | The output of `git describe --tags`, which `docs/generate_docs.sh` passes to the documentation generator as `PROJECT_VERSION`. Whenever the tree is ahead of a tag it takes the form shown — here the `1.0.0` tag plus however many commits have been made since it. That string identifies a position in history, not a version of this interface or of this document, and must not be read as one. No literal value is quoted, because the string changes with every commit. |

**Provenance of this page.** It was renamed from `docs/pages/MoCAHalSpec.md` to `docs/pages/halSpec.md` in the same change that rewrote it against the canonical topic set. Git records a rename only where the two versions still resemble each other, and a full rewrite does not, so `git log --follow -- docs/pages/halSpec.md` begins at that change: the revisions before it are reached with `git log -- docs/pages/MoCAHalSpec.md`.

*Derived from `CHANGELOG.md`, the repository's git tags, `docs/generate_docs.sh`:26,
and `include/moca_hal.h`:362, :364, :397.*

## Acronyms

- `ACA` \- Automatic Channel Adaptation
- `CPE` \- Customer Premises Equipment
- `EVM` \- Error Vector Magnitude
- `HAL` \- Hardware Abstraction Layer
- `MoCA` \- Multimedia over Coax Alliance
- `NC` \- Network Coordinator
- `OEM` \- Original Equipment Manufacturer
- `PHY` \- Physical Layer
- `PQoS` \- Priority Quality of Service
- `RDK-B` \- Reference Design Kit for Broadband Devices
- `SCMOD` \- Subcarrier Modulation
- `SLA` \- Service Level Agreement

*Bounded to the terms this document uses. Expansions are taken from `include/moca_hal.h` — `ACA` at
:637, `CPE` at :193, `EVM` at :272, `PQoS` at :552, `SCMOD` at :1441 and `NC` at :606.*

## Description

The diagram below describes a high-level software architecture of the MoCA HAL module stack.

```mermaid
flowchart TD;
    RDK-BSTACK[RDK-B Stack] <-->
    CcspMoCA[CcspMoCA] <-->
    MoCAHAL["MoCA HAL(libhal_moca)"] <-->
    VendorSoftware[Vendor Software]
```

The MoCA (Multimedia over Coax Alliance) HAL (Hardware Abstraction Layer) is a component within the
RDK-B (Reference Design Kit Broadband) framework designed to facilitate interaction with MoCA
network adapters. Its primary purpose is to provide a standardized interface that allows
higher-level software components to access and control MoCA functionalities, regardless of the
specific underlying hardware or vendor implementation.

This repository carries the **interface definition only**. `include/moca_hal.h` declares 21
functions and the data types they exchange; each vendor or `OEM` ships its own implementation behind
that contract as a shared library. Nothing in this repository implements the interface.

For a caller, the interface covers five things:

- **Interface configuration** — read and write the frequency mask, channel scanning, privacy
  passphrase, power limits and bandwidth thresholds of a MoCA interface.
- **Interface information** — static properties that do not change while the interface is up
  (firmware version, MAC address, capability masks) and dynamic properties that do (link status,
  operating frequency, `NC` node identity, connected client count).
- **Statistics and counters** — network-layer, MAC-layer, aggregate and `PQoS` flow statistics, the
  full mesh `PHY` rate table, and the module reset count.
- **Associated devices** — enumerate the nodes on the MoCA network, and receive an asynchronous
  notification when one joins or leaves.
- **`ACA`** — configure, start, cancel and read back an Automatic Channel Adaptation run, plus the
  `SCMOD` statistics it produces.

**How to read this document.** `Description` and `Component Runtime Execution Requirements` answer
"what is this and how do I call it". `Non functional requirements` and `Interface API Documentation`
answer the protocol-level questions — exact identifiers, data structures, call ordering, return-code
semantics and state transitions. "API Surface" is the boundary between the two: a reader who came
for the overview can stop above it.

*Derived from `include/moca_hal.h`:59-85 and :723-1478, and from the architecture chain carried by the
predecessor of this page. The superproject inventory corroborates this scope — interface bring-up
and configuration, static and dynamic interface information, statistics, and associated-device
enumeration.*

## Optional Components

The following parts of the interface are optional; the rest of it is not.

- **`moca_GetFullMeshRates` and the mesh `PHY` rate table.** This function and the
  `moca_mesh_table_t` structure it fills are compiled only when `MOCA_VAR` is **not** defined, as are
  `moca_if_status_t` and `moca_dynamic_info_t`. A build that defines `MOCA_VAR` exposes a smaller
  interface. See "Platform or Product Customization" for the
  full consequence.
- **The associated-device notification callback.** `moca_associatedDevice_callback_register` is
  optional: a caller that polls `moca_GetNumAssociatedDevices` and `moca_GetAssociatedDevices` never
  has to register one. A caller that does register one receives node join and leave events
  asynchronously instead of polling for them.

No other component of this interface is optional, and this repository declares no optional external
daemon or library that a vendor may substitute.

*Derived from `include/moca_hal.h`:238, :374, :528, :1161 (the `MOCA_VAR` guards) and :705 (the
callback registration).*

## Component Runtime Execution Requirements

The requirements in this block apply to every implementation of this interface. Failure to meet them
will result in undefined behaviour in the calling middleware.

### Initialization and Startup

**This interface exposes no initialization or teardown entry point.** None of the 21 declared
functions opens, initializes, closes or de-initializes the MoCA subsystem, and the interface
declares no session handle, no context object and no paired create and destroy calls. Bringing the
MoCA hardware and the vendor software into a usable state happens out of band — in the vendor's own
start-up path, its driver load, or its own daemon — and is not something a caller performs or can
observe through this interface.

The practical consequence for a caller is that there is no initialization call to sequence against,
and no return code that means "not initialized yet". The available readiness signal is
`moca_HardwareEquipped`, which reports whether MoCA hardware is present and correctly configured on
the system.

RDK-B's MoCA Hardware Abstraction Layer (HAL) is responsible for setting up the MoCA interface on a
device. This involves starting up the interface and adjusting its parameters, including frequency
band, channel, and encryption settings. The HAL provides a collection of APIs that allow developers
to interact with the MoCA hardware and manage the interface configuration.

A caller's normal bring-up order is:

- `moca_HardwareEquipped()` \- confirm MoCA hardware is present before calling anything else.
- `moca_IfGetStaticInfo()` \- read the interface name, MAC address, firmware version and capability
  masks, which do not change while the interface is up.
- `moca_GetIfConfig()` \- read the configuration currently in force.
- `moca_SetIfConfig()` \- apply configuration, having first read it back so that unmodified members
  are preserved.
- `moca_associatedDevice_callback_register()` \- register the notification callback, if the caller
  wants node join and leave events rather than polling for them.
- `moca_IfGetDynamicInfo()` \- read link status and the current operating state.
- `moca_IfGetStats()`, `moca_GetNumAssociatedDevices()` and `moca_FreqMaskToValue()` \- routine
  monitoring and mask interpretation thereafter.

Third party vendors will implement appropriately to meet operational requirements.

Before the vendor's MoCA subsystem is ready, a call may not return promptly; see "Blocking calls",
which states what is and is not specified about that window.

*Derived from `include/moca_hal.h`:723-1478 (the complete declaration set, which contains no
lifecycle call) and :1531-1554 (`moca_HardwareEquipped`).*

### Threading Model

Vendors may implement internal threading and event mechanisms to meet their operational
requirements. These mechanisms must be designed to ensure thread safety when interacting with HAL
interface. Proper cleanup of allocated resources (e.g., memory, file handles, threads) is mandatory
when the vendor software terminates or closes its connection to the HAL.

This interface is not inherently required to be thread-safe. It is the responsibility of the calling
module or component to ensure that all interactions with the APIs are properly synchronized.

This is a deliberate property of the MoCA HAL and differs from other RDK-B HALs, some of which
require the implementation itself to be thread safe. A caller must not carry an assumption of thread
safety over from another HAL: concurrent calls into this interface from more than one thread must be
serialized by the caller.

One consequence is specific to the notification callback. The callback registered through
`moca_associatedDevice_callback_register` is invoked by the implementation in response to a network
event rather than by the caller, and this interface does not state whether it is delivered on a
thread of the implementation's own or from within some call the caller has made. A caller that both
polls and registers a callback must therefore serialize its own state against both paths without
assuming either delivery context. See "Asynchronous Notification Model".

*Derived from the runtime policy stated by the predecessor of this page, retained here as this
repository's own statement, and from `include/moca_hal.h`:669-705.*

### Process Model

All APIs are expected to be called from multiple processes. Due to this concurrent access, vendors
must implement protection mechanisms within their API implementations to handle multiple processes
calling the same API simultaneously. This is crucial to ensure data integrity, prevent race
conditions, and maintain the overall stability and reliability of the system.

*Derived from the process model stated by the predecessor of this page.*

### Memory Model

Every function in this interface that produces data writes it through a caller-supplied pointer, and
none of them returns allocated storage that the caller must free. Five functions produce no data
through a pointer and so raise no output-ownership question at all: `moca_HardwareEquipped`,
`moca_FreqMaskToValue`, `moca_setIfAcaConfig` and `moca_cancelIfAca` return their result as the
return value, and `moca_associatedDevice_callback_register` returns nothing. The interface declares
no allocator and no matching release function.

#### Caller Responsibilities

- Manage memory passed to specific functions as outlined in the API documentation. This includes
  allocation and deallocation to prevent leaks.
- Allocate every output buffer before the call. Two cases carry an explicit sizing rule from the
  header: `moca_GetMocaCPEs` requires the `cpes` array to be pre-allocated with space for
  `kMoca_MaxCpeList` (256) entries, and `moca_GetFullMeshRates` requires enough entries for all
  possible node pairs. `moca_GetAssociatedDevices` and `moca_getIfScmod` take a pointer-to-pointer
  and the header states that the caller is responsible for the array's memory in both cases.
- **Do not assume the implementation is finished with a buffer when the call returns.** This
  interface states no lifetime rule that holds for every function, and no declaration in it says
  that a caller-supplied pointer is dropped at return. What the declarations do say is that the
  caller owns the storage on both sides of the call: each of the eleven single-pointer structure and
  array out-parameters is documented as addressing caller-allocated storage that **the caller both
  allocates and releases** — `moca_GetIfConfig` (`moca_hal.h`:759-761), `moca_IfGetDynamicInfo`
  (:907-910), `moca_IfGetStaticInfo` (:993-996), `moca_IfGetStats` (:1062-1071),
  `moca_IfGetExtCounter` (:1195-1205), `moca_IfGetExtAggrCounter` (:1259-1268), `moca_GetMocaCPEs`
  (:1330-1332), `moca_GetFullMeshRates` (:1596-1598), `moca_GetFlowStatistics` (:1672-1674),
  `moca_getIfAcaConfig` (:1867-1869) and `moca_getIfAcaStatus` (:1981-1983). Each of those entries
  refers the reader to this topic, and none of them states how long the implementation may use the
  pointer it was given. Ownership is therefore settled and retention is not, so the conservative
  reading — the one to follow for all eleven — is to hold each output structure or array in storage
  that stays allocated for as long as the caller keeps using this interface, and to copy the values
  out if it needs to release or reuse that storage sooner.
- **The two pointer-to-pointer out-parameters say nothing about retention either, and add a second
  trap.** `moca_GetAssociatedDevices` (`moca_hal.h`:1405-1412) and `moca_getIfScmod` (:2063-2070)
  take the address of the caller's own pointer variable. Both declarations state that the caller
  allocates the array and that the pointer written back must not be freed as though the
  implementation had allocated it; neither states a retention rule, so the conservative reading above
  applies to them unchanged. A caller frees such an array with the allocator it used itself, and only
  once it has stopped calling the interface.
- **The two non-`const` input pointers carry the same caveat from the other direction.**
  `moca_SetIfConfig` takes `moca_cfg_t *` and `moca_FreqMaskToValue` takes `UCHAR *mask`, and neither
  declaration states whether the implementation writes to the buffer or retains it, so a caller must
  assume neither is safe to rely on: it should not treat its own copy as unchanged after the call and
  should not reuse the storage for something else while it continues to call the interface
  (`moca_hal.h`:831-836 and :1485-1490).
- **Three parameters are the exceptions, and they are exceptions because a declaration says so.**
  `moca_setIfAcaConfig` takes its configuration **by value**, so no pointer and no lifetime question
  arises at all (`moca_hal.h`:1283-1285). `moca_GetResetCount` states that its `resetcnt` out-value
  is left untouched on failure, so a caller may initialise the variable once and re-read into it
  (:1729-1730, :1743-1744). Every other count out-parameter is undefined on failure and must be
  discarded rather than reused — `moca_GetNumAssociatedDevices` states exactly that (:1148-1149).
- **A pointer the interface hands *to* the caller is the opposite case and is even shorter-lived.**
  The `moca_associated_device_t` record delivered to a registered callback is the implementation's,
  not the caller's; it is valid only for the duration of the callback and must be copied there.
  `Asynchronous Notification Model` states that rule with its evidence.

#### Module Responsibilities

- Modules must allocate and de-allocate memory for their internal operations, ensuring efficient
  resource management.
- Modules are required to release all internally allocated memory upon closure to prevent resource
  leaks.
- All module implementations and caller code must strictly adhere to these memory management
  requirements for optimal performance and system stability. Unless otherwise stated specifically in
  the API documentation.
- All strings used in this module must be zero-terminated. This ensures that string functions can
  accurately determine the length of the string and prevents buffer overflows when manipulating
  strings.

**No memory footprint limit is specified for this interface.** Neither this repository nor the header
states a maximum resident size, heap budget or allocation count for an implementation. A caller
cannot rely on a bound, and an implementer is not held to one by this specification.

*Derived from `include/moca_hal.h`:1028-1029, :1064-1066, :1171-1172 and :1450-1452 (the per-function
ownership and sizing statements); from the per-parameter retention statements at :736-739, :830-832,
:869-871, :903-905, :968-970, :997-999, :1030-1033, :1174-1176, :1213-1214, :1339-1341 and
:1407-1409, the non-`const` input caveats at :781-785 and :1104-1108 with :1115-1117, the by-value
parameter at :1283-1285, and the on-failure statements at :938-939 and :1249-1254; and from the
memory model stated by the predecessor of this page.*

### Power Management Requirements

The MoCA HAL is not involved in any of the power management operation. It does not participate in or
require involvement in any power-related functions.

The transmit power members this interface exposes — `TxPowerLimit`, `BeaconPowerLimit` and
`AutoPowerControlEnable` in `moca_cfg_t` — are MoCA radio transmission settings. They are not device
power states, and setting them does not participate in system power management.

*Derived from the power management statement carried by the predecessor of this page, and from
`include/moca_hal.h`:327-337.*

### Asynchronous Notification Model

This interface has exactly one asynchronous notification: an associated-device event, delivered
through a caller-registered callback. Everything else in the interface is synchronous.

**Registration.** A caller installs the callback with
`void moca_associatedDevice_callback_register(moca_associatedDevice_callback callback_proc)`.
The function returns `void`, so it reports no failure and there is nothing to check. The header
states that the registered function "will be called whenever a MoCA client is activated or
deactivated on the network". The interface declares no de-registration call, and does not specify
whether passing a null pointer removes a previously registered callback — a caller must not assume
that it does.

**Callback signature.** The callback itself is
`typedef INT (*moca_associatedDevice_callback)(ULONG ifIndex, moca_associated_device_t *moca_dev)`.

- `ifIndex` identifies the MoCA interface on which the event occurred.
- `moca_dev` points to a `moca_associated_device_t` record describing the device. Its `Active` member
  carries the event: `TRUE` means the device has been activated, `FALSE` means it has been
  deactivated. The remaining members carry the node's identity and link characteristics — MAC
  address, node ID, `PHY` transmit and receive rates, power levels and capability flags.
- The callback returns an `INT` status code indicating the result of handling the event. The interface
  does not define what an implementation does with a failure status, so a caller should not depend on
  any particular consequence of returning one.

**Execution context and caller obligations.** This interface does not specify which thread delivers
the callback, whether deliveries are serialized against each other, or whether the interface may be
called from inside a callback body. All three are unspecified, and the revision of the header this
specification was written against establishes none of them, so a caller must not assume that the
callback arrives on its own call sequence, nor that it arrives on a thread of the implementation's
own. The guidance that follows from that is the conservative reading, because every unknown falls
the same way: a callback body must complete quickly, must not block, and must synchronize any state
it shares with the caller's other threads, and a caller that needs to call back into the interface
from a callback body must establish with the vendor implementation that doing so is supported.

**What the interface does not say about `moca_dev`.** The parameter is declared
`moca_associated_device_t *moca_dev` - not `const` - and that declaration is the whole of the stated
contract: this interface does not say who owns the storage, how long it remains valid, or whether the
pointer may be NULL, and the revision of the header this specification was written against says none
of it either. The guidance here is therefore conservative rather than a guarantee, because every
unknown falls the same way for a callback body: copy whatever it needs out of the record before
returning, never free the pointer, never store it for later use, and check it for NULL before
dereferencing it. Reading the record after the callback has returned, or releasing it, would each
rely on a guarantee this interface has not made.

The interface specifies no event queue, no delivery guarantee and no ordering guarantee, and it
provides no way to enumerate the currently registered callback. A caller that requires a complete
picture of network membership should therefore reconcile against
`moca_GetNumAssociatedDevices` and `moca_GetAssociatedDevices` rather than treating the notification
stream as authoritative.

*Derived from `include/moca_hal.h`:575-603 (the callback typedef and its documented trigger),
:669-705 (the registration function), and :523 (the `Active` member).*

### Blocking calls

**Synchronous and Responsive:** All APIs within this module should operate synchronously and complete
within a reasonable timeframe based on the complexity of the operation. Specific timeout values or
guidelines may be documented for individual API calls.

**Timeout Handling:** To ensure resilience in cases of unresponsiveness, implement appropriate
timeouts for API calls where failure due to lack of response is a possibility. Refer to the API
documentation for recommended timeout values per function.

**Non-Blocking Requirement:** Given the single-threaded environment in which these APIs will be
called, it is imperative that they do not block or suspend execution of the main thread.
Implementations must avoid long-running operations or utilize asynchronous mechanisms where
necessary to maintain responsiveness.

**No timeout value is specified for any function in this interface.** No declaration in
`moca_hal.h` documents a timeout, a completion deadline or a maximum duration, and this repository
states none. A caller that needs a bound must impose it itself, and an implementer is not held to a
particular figure by this specification.

**Start-up latency is the one bounded exception, and it is unspecified.** The non-blocking
requirement above is a steady-state requirement. Before the vendor's MoCA subsystem is ready —
which, per "Initialization and Startup", this interface gives a caller no way to bring about or to
observe — a call may not return promptly, and this interface specifies neither a bound on that
window nor a distinct return code for it. A caller must not assume an immediate return during
start-up, and must not assume a bounded wait either. `moca_HardwareEquipped` is the only readiness
check available before the first substantive call.

The `ACA` calls are not an exception to the non-blocking requirement. An `ACA` run is long-running,
but it is not a blocking call: `moca_setIfAcaConfig` starts the run and returns, and the caller
learns of completion by polling `moca_getIfAcaStatus`. See "State Diagram".

*Derived from the blocking-call policy stated by the predecessor of this page, and from
`include/moca_hal.h`:1270-1327 and :1397-1438 (the `ACA` start and status contracts).*

### Internal Error Handling

**Synchronous Error Handling:** All APIs must return errors synchronously as a return value. This
ensures immediate notification of errors to the caller.
**Internal Error Reporting:** The HAL is responsible for reporting any internal system errors (e.g.,
out-of-memory conditions) through the return value.
**Focus on Logging for Errors:** For system errors, the HAL should prioritize logging the error
details for further investigation and resolution.

The return-code vocabulary is small. Eighteen of the 21 functions return an `INT` or `int` status
drawn from the macros in the header; the other three do not return a status at all and are listed in
"API Surface".

| Code | Value | Meaning and what a caller should do |
| --- | --- | --- |
| `STATUS_SUCCESS` | `0` | The operation succeeded and any output buffer has been populated. |
| `STATUS_FAILURE` | `-1` | The operation failed. The interface does not distinguish the reason, so a caller can only retry, fall back, or report the failure; it cannot branch on a cause. |
| `STATUS_NOT_AVAILABLE` | `-2` | Declared by the header for use where a value or capability is not available. |
| `STATUS_INPROGRESS` | `-1` | `ACA` only: an `ACA` process is already running on the interface, and the request did not start a new one. A caller should read `moca_getIfAcaStatus` or cancel with `moca_cancelIfAca` rather than retrying immediately. |
| `STATUS_NO_NODE` | `-2` | `ACA` only: the specified MoCA node does not exist. A caller should re-read the node set with `moca_GetAssociatedDevices` before retrying. |
| `STATUS_INVALID_PROBE` | `-3` | `ACA` only: the call supplied an invalid probe type. The valid values are the two `PROBE_TYPE` members. |
| `STATUS_INVALID_CHAN` | `-4` | `ACA` only: the call supplied an invalid channel. |

**Two pairs of these codes share a numeric value, and a caller cannot tell them apart from the return
value alone.** `STATUS_FAILURE` and `STATUS_INPROGRESS` are both `-1`, and `STATUS_NOT_AVAILABLE`
and
`STATUS_NO_NODE` are both `-2`. This matters on the `ACA` calls, where the header documents
`STATUS_FAILURE` and `STATUS_INPROGRESS` as distinct outcomes of `moca_setIfAcaConfig`: a return of
`-1` means either that configuration failed or that a run is already in progress, and the two must be
separated by reading `moca_getIfAcaStatus` and inspecting the `stat` member of `moca_aca_stat_t`
rather than by comparing the return value. A caller must not treat the two macro names as
distinguishable at run time.

The header also records that extending these codes with the specific reason for a failure was
raised during the open-source migration review and is **not** part of this revision, because it
would change the interface. A caller should therefore treat any non-zero value as a failure of the
class above rather than assuming the set is closed.

*Derived from `include/moca_hal.h`:143-153 and :219-222 (the macro values), :713-721 (the recorded
intent to extend the codes), :1812-1818 (the `moca_setIfAcaConfig` return contract) and :626 (the
`stat` member), plus the error-handling policy stated by the predecessor of this page.*

### Persistence Model

There is no requirement for HAL to persist any setting information. Application/Client is
responsible to persist any settings related to their implementation.

The `Reset` member of `moca_cfg_t` is the counterpart of this rule on the implementation side:
setting it returns the MoCA configuration parameters to their defaults, so a caller that has
persisted settings of its own is responsible for re-applying them afterwards.

*Derived from the persistence statement carried by the predecessor of this page, and from
`include/moca_hal.h`:334.*

## Non functional requirements

Following non functional requirement should be supported by the MoCA HAL component.

### Logging and debugging requirements

The component is required to record all errors and critical informative messages to aid in
identifying, debugging, and understanding the functional flow of the system. Logging should be
implemented using the syslog method, as it provides robust logging capabilities suited for
system-level software. The use of `printf` is discouraged unless `syslog` is not available.

All HAL components must adhere to a consistent logging process. When logging is necessary, it should
be performed into the `moca_vendor_hal.log` file, which is located in either the `/var/tmp/` or
`/rdklogs/logs/` directories.

Logs must be categorized according to the following log levels, as defined by the Linux standard
logging system, listed here in descending order of severity:

- **FATAL:** Critical conditions, typically indicating system crashes or severe failures that require
  immediate attention.
- **ERROR:** Non-fatal error conditions that nonetheless significantly impede normal operation.
- **WARNING:** Potentially harmful situations that do not yet represent errors.
- **NOTICE:** Important but not error-level events.
- **INFO:** General informational messages that highlight system operations.
- **DEBUG:** Detailed information typically useful only when diagnosing problems.
- **TRACE:** Very fine-grained logging to trace the internal flow of the system.

Each log entry should include a timestamp, the log level, and a message describing the event or
condition. This standard format will facilitate easier parsing and analysis of log files across
different vendors and components.

Because `STATUS_FAILURE` carries no reason code, this log is the only place a caller can look for
the cause of a failed call. An implementation should log enough detail at **ERROR** to identify
which operation failed and why, since the return value alone cannot convey it — see "Internal Error
Handling".

**Handling of secret values and node addresses in log and debug output.** This interface moves one
credential and six address fields, and both classes are excluded from the log described above:

- **The credential.** `moca_cfg_t.KeyPassphrase` (`include/moca_hal.h`:325) is the link privacy
  password. It travels in both directions — `moca_GetIfConfig()` reads the whole structure back and
  `moca_SetIfConfig()` writes the whole structure in — so a caller performing a read-modify-write
  holds the password in its own storage across two calls.
- **The node addresses.** `moca_static_info_t.MacAddress` (`include/moca_hal.h`:361) is the local
  node's address; `moca_dynamic_info_t.NetworkCoordinatorMACAddress` (:409) is the Network
  Coordinator's; `moca_cpe_t.mac_addr` (:488) and `moca_associated_device_t.MACAddress` (:507) are
  the addresses of the nodes and customer-premises equipment on the link; and
  `moca_flow_table_t.DestinationMACAddress` (:567) is the destination of an admitted PQoS flow. A
  node address identifies a subscriber's own equipment and links a unit to a subscriber record,
  which makes it personal data in this context.

The following requirements bind the vendor implementation and the `RDK-B` caller equally.

- **Neither class is written to log output at any severity.** Not at **FATAL**, and not at **DEBUG**
  or **TRACE** either. A value too sensitive for **INFO** does not become acceptable at a lower
  level, and a build that turns on verbose tracing must not become a build that discloses a
  password or a subscriber's equipment address. The same exclusion applies to `printf`, to a trace
  buffer and to any diagnostic dump the implementation writes.
- **Redact with one fixed marker, and never emit a fragment.** A prefix, a suffix, a character
  count or a hash is not redaction. `KeyPassphrase` holds at most 17 characters, so even its length
  narrows a search; and the first three bytes of a `MAC` address *are* the vendor `OUI`, so a
  truncated address still discloses the equipment vendor. Where an **ERROR** record must say what
  the implementation acted on, it records the operation, the outcome and a non-identifying
  discriminator — the `ifIndex`, the node index, the flow index — and substitutes the same fixed
  marker for the value itself.
- **Crash artefacts and telemetry are in scope.** A core file, a crash report, a diagnostic bundle
  collected off the device and any telemetry or metrics record are log output for the purpose of
  these rules. A password kept out of `moca_vendor_hal.log` and then carried off the device in a
  core file has not been protected.
- **Clear after use.** Every buffer in this interface is caller-allocated, so clearing is the
  caller's to do: overwrite the `moca_cfg_t` copy once `moca_SetIfConfig()` has returned rather than
  leaving the password on the stack or in a heap block that will be reused, and overwrite the node
  and flow arrays once they have been read. An implementation clears its own working copies on the
  same terms and is bound by the same obligation not to hold a password beyond the call that supplied it, an obligation this interface places on the implementation rather than a property a caller can verify.
- **A failure status does not license logging the input.** `STATUS_FAILURE` conveys no reason, which
  is exactly the situation in which an implementation is tempted to log the value it rejected. It
  records the operation and the outcome instead; the rejected `KeyPassphrase` is the one value a
  failed `moca_SetIfConfig()` must never place in a diagnostic.
- **The interface enforces none of this.** `include/moca_hal.h` declares no redaction helper, no
  opaque credential type and no flag by which a caller could ask an implementation to suppress
  these values, and `KeyPassphrase` is an ordinary `CHAR` array that any format string will print.
  An integrator establishes that a vendor implementation observes these rules by inspection or by
  contract, and treats their absence from a vendor log as unverified until it has done so.

*Derived from the logging policy stated by the predecessor of this page, and from
`include/moca_hal.h`:147-149, :302-313, :325, :361, :409, :488, :507, :567, :753-758 and
:803-809.*

### Memory and performance requirements

**Client Module Responsibility:** The client module using the HAL is responsible for allocating and
deallocating memory for any data structures required by the HAL's APIs. This includes structures
passed as parameters to HAL functions and any buffers used to receive data from the HAL.

**Vendor Implementation Responsibility:** Third-party vendors, when implementing the HAL, may allocate
memory internally for their specific operational needs. It is the vendor's sole responsibility to
manage and deallocate this internally allocated memory.

**No memory footprint requirement is specified for this interface**, and no CPU utilization budget or
per-call latency target is specified either. Neither this repository nor the header states one, so a
caller cannot rely on a bound and an implementer is not held to a figure by this specification.

The largest fixed allocations a caller should size for come from the declared types rather than from
a stated budget: `moca_scmod_stat_t` carries three 512-byte subcarrier arrays, `moca_aca_stat_t`
carries a 512-element power profile, and `moca_cfg_t` and `moca_static_info_t` each carry several
128-byte frequency masks. Array-returning calls are bounded by `kMoca_MaxCpeList` (256) and
`kMoca_MaxMocaNodes` (16).

*Derived from the memory and performance policy stated by the predecessor of this page, and from
`include/moca_hal.h`:198, :207, :324, :340-341, :366-367, :631-633 and :663.*

### Quality Control

To ensure the highest quality and reliability, it is strongly recommended that third-party quality
assurance tools like `Coverity`, `Black Duck`, and `Valgrind` be employed to thoroughly analyze the
implementation. The goal is to detect and resolve potential issues such as memory leaks, memory
corruption, or other defects before deployment.

Furthermore, both the HAL wrapper and any third-party software interacting with it must prioritize
robust memory management practices. This includes meticulous allocation, deallocation, and error
handling to guarantee a stable and leak-free operation.

**Keeping this document accurate.** Every topic in this specification names the file it was derived
from. Any change to one of those files obliges a review of the topics that cite it — in particular,
a change to `include/moca_hal.h` obliges a review of "Data Structures and Defines", "API Surface",
"Sequence Diagram" and "State Diagram", because a renamed or added declaration falsifies them
immediately. A change to `docs/generate_docs.sh` — in particular to its generator pin or to its
Doxygen parameters — obliges a review of "Variability Management", which records what that script
asks the documentation build to do and what the pinned generator actually does with it.

This repository declares no `CODEOWNERS`, so the addressee of that obligation is the maintainer
group that `CONTRIBUTING.md` directs contributions to: raise an issue, then open a pull request
against this repository for team review.

*Derived from the quality-control policy stated by the predecessor of this page, and from
`CONTRIBUTING.md`.*

### Licensing

MoCA HAL implementation is expected to released under the Apache License 2.0

The interface definition in this repository is itself licensed under Apache License 2.0; the full
text is carried in `LICENSE.md` and the attribution notice in `NOTICE.md`.

*Derived from the licensing statement carried by the predecessor of this page and from the
repository's own licence files.*

### Build Requirements

The source code should be capable of, but not be limited to, building under the Yocto distribution
environment. The recipe should deliver a shared library named as `libhal_moca.so`.

A caller consumes the interface by including `moca_hal.h` and establishing a linker dependency on
that library; see "Interface API Documentation".

This repository ships the interface header only — it contains no implementation, no build recipe and
no library — so the toolchain, compiler and build options used to produce `libhal_moca.so` are
determined by the vendor's own recipe and are not specified here.

*Derived from the build statement carried by the predecessor of this page, and from the contents of
this repository.*

### Variability Management

The role of adjusting the interface, guided by versioning, rests solely within architecture
requirements. Thereafter, vendors are obliged to align their implementation with a designated
version of the interface. As per Service Level Agreement (`SLA`) terms, they may transition to newer
versions based on demand needs.

Each API interface will be versioned using [Semantic Versioning 2.0.0](https://semver.org/spec/v2.0.0.html), the
vendor code will comply with a specific version of the interface. Note, per "Version History", that
`moca_hal.h` declares no version macro, so a caller cannot read the interface version at compile
time or at run time; the version a build complies with is established outside this interface.

`MOCA_VAR` is the single compile-time flag that changes this interface, and
"Platform or Product Customization" states exactly what it removes.

**The generated documentation describes the variant in which `MOCA_VAR` is *not* defined, and it does
so despite this repository asking for the opposite.** The distinction matters to any caller who
reads the generated site rather than the header, so it is stated here rather than left to be
discovered.

`docs/generate_docs.sh` passes `DOXYGEN_EXTRA_PARAMS="PREDEFINED='MOCA_VAR=0'"` to the documentation
build, which reads as an intention to document the `MOCA_VAR`-defined variant. **That setting has no
effect at the generator version this repository pins.** `docs/generate_docs.sh` pins the generator
at
`1.2.0`, and at that version neither the generator's `Makefile` nor its Doxygen configuration
references `DOXYGEN_EXTRA_PARAMS` at all — the configuration's `PREDEFINED` list is left empty. The
variable is therefore accepted by `make` and silently discarded, `MOCA_VAR` is left undefined for
the documentation build, and the four conditional symbols are extracted into the generated site
along with everything else:

- `moca_if_status_t` — the interface status enumeration.
- `moca_dynamic_info_t` — the dynamic interface information structure.
- `moca_mesh_table_t` — the mesh `PHY` rate table entry.
- `moca_GetFullMeshRates` — the function that fills that table.

The consequences for a reader are worth being precise about, because they run in the direction that
causes mistakes. The generated site presents all 21 declared functions and all 16 declared
structures and enumerations, so it describes a **larger** interface than a build that defines
`MOCA_VAR` actually provides. A caller working from the generated site alone, on a platform that
defines `MOCA_VAR`, would find four documented symbols missing at compile time with nothing in the
generated page to explain why. This page therefore marks all four as conditional at every mention,
and "Platform or Product Customization" states what a `MOCA_VAR` build loses.

*Derived from `docs/generate_docs.sh`:23 and :27, the pinned generator's `Makefile` and Doxygen
configuration at tag `1.2.0`, `include/moca_hal.h`:238, :374, :528, :1161 and :1192, and the
variability statement carried by the predecessor of this page.*

### Platform or Product Customization

When `MOCA_VAR` is defined in the provided header file, certain sections of the code are excluded
from compilation. Here's what will happen:

**Exclusion of `moca_if_status_t` Enumeration:** This enumeration defines possible states for a MoCA
interface, such as `IF_STATUS_Up`, `IF_STATUS_Down`, and others. When `MOCA_VAR` is defined, these
states are not declared, which means that any code relying on these specific MoCA interface states
will not compile.

**Exclusion of Dynamic Info Structure Definition:** The structure `moca_dynamic_info_t`, which
contains dynamic information about a MoCA interface like its status, last change, max ingress and
egress bandwidth, and so forth, is also excluded. This will affect the functionality that relies on
obtaining or manipulating dynamic status information of MoCA interfaces.

**Exclusion of MoCA Mesh Table Definition and Function:** The `moca_mesh_table_t` structure and
associated functions like `moca_GetFullMeshRates` that provide information on the `PHY` rates
between nodes in a MoCA network are not available. This limitation means that the software will lack
the capability to fetch or manipulate full mesh `PHY` rates when `MOCA_VAR` is defined.

These exclusions lead to a restricted set of functionalities related to interface status reporting,
dynamic information management, and network-wide `PHY` rate information retrieval and handling.

There is a second consequence a caller must account for: `moca_IfGetDynamicInfo` remains declared
when
`MOCA_VAR` is defined, but its `moca_dynamic_info_t` out-parameter type does not, so that function is
unusable in such a build even though it is still declared. A caller that must support both build
variants should compile its dynamic-information path under the same `#ifndef MOCA_VAR` guard the
header uses.

`MOCA_VAR` is the only customization flag this interface defines. No other platform or product
variation is expressed in the header.

*Derived from `include/moca_hal.h`:238-261, :374-413, :528-547 and :1161-1201, and the customization
inventory carried by the predecessor of this page.*

## Interface API Documentation

The [`moca_hal.h`](../../include/moca_hal.h) header file provides a complete reference for all HAL
function prototypes and data type definitions. Each declaration carries a Doxygen block giving its
per-parameter direction, valid ranges, pre-conditions and the return codes it can produce; that
block is the authority for the detail this page indexes rather than repeats.

A caller takes two steps to use the MoCA HAL from its own component or process:

1. **Inclusion:** include the `moca_hal.h` header file in the calling source file.
2. **Linking:** establish a linker dependency on the `libhal_moca` library.

### Theory of operation and key concepts

Broadband MoCA HAL serves as a standardized interface that simplifies developer interaction with
MoCA hardware. By abstracting the underlying complexities of the MoCA hardware, the HAL streamlines
development and enables developers to concentrate on application logic. Furthermore, the HAL offers
tools for monitoring and troubleshooting the MoCA interface, ensuring optimal performance of the
MoCA network.

The interface is deliberately narrow in shape: every call is a synchronous getter or setter
addressed to an interface index, results are written through caller-supplied pointers, and status is
reported by return value. There is no session, no handle and no connection to keep alive. The single
exception to the synchronous shape is the associated-device notification described in "Asynchronous
Notification Model", and the single long-running operation is the `ACA` process described in "State
Diagram".

#### Object Lifecycles

- **Creation:** The MoCA HAL itself does not explicitly create objects in the traditional sense.
  Instead, it provides an interface for interacting with the underlying MoCA hardware. Client modules
  are responsible for allocating memory for data structures like `moca_cfg_t` and `moca_stats_t` to
  receive information from the HAL.

- **Usage:** These structures are populated by the HAL's API functions (`moca_GetIfConfig`,
  `moca_IfGetDynamicInfo`, etc.). Client modules then use the information within these structures to
  configure the MoCA interface, monitor its status, and gather statistics.

- **Destruction:** Client modules are responsible for deallocating the memory they allocated for the
  data structures after they are no longer needed. The HAL itself does not manage the lifecycle of
  these objects.

- **Unique Identifiers:** The `ifIndex` parameter (an unsigned long) is used to identify specific MoCA
  interfaces. If a device has multiple MoCA interfaces, each would have a unique `ifIndex`. There are
  no other explicit unique identifiers for objects within the HAL. The header states the convention as
  0 for a single interface and 1 to 256 where several are present. The five `ACA` functions name the
  same concept `interfaceIndex` and type it as `int` rather than `ULONG`; the parameter identifies the
  interface in exactly the same way.

#### Method Sequencing

- **No initialization step:** This interface declares no initialization, open, close or teardown
  function, so there is no lifecycle call to sequence against and no state a caller is responsible for
  entering before its first call. Bringing the MoCA subsystem up is the vendor's own out-of-band
  concern, as "Initialization and Startup" states.
  `moca_HardwareEquipped` is the only readiness check the interface offers, and calling it first is the
  recommended practice rather than a documented pre-condition.

- **Configuration:** Configuration functions like `moca_SetIfConfig` should generally be called before
  attempting to retrieve information or perform other operations. Because `moca_SetIfConfig` takes a
  complete `moca_cfg_t`, a caller should read the current configuration with `moca_GetIfConfig`,
  modify the members it intends to change, and write the whole structure back; populating the
  structure from scratch would overwrite every member a caller did not set.

- **Sizing before enumeration:** `moca_GetNumAssociatedDevices` should be called before
  `moca_GetAssociatedDevices` so that the caller can size the array it must supply, and
  `moca_GetMocaCPEs` requires an array pre-allocated for `kMoca_MaxCpeList` (256) entries.

- **`ACA` ordering:** `moca_setIfAcaConfig` must precede `moca_getIfAcaStatus` and `moca_getIfScmod`,
  because both report on a run that the former configures or starts — the polling call states that a
  run must have been configured or started for its result to be meaningful (`moca_hal.h`:2039-2039),
  and the statistics call that a run must have completed (:1459-1460). That is a call-ordering rule,
  not a transition: "State Diagram" sets out what each operation is documented to do and what the
  interface leaves unspecified, and draws no edge.

- **Other Methods:** Most other functions (`moca_GetIfConfig`, `moca_IfGetDynamicInfo`, etc.) can be
  called in any order, as long as they are called from a single-threaded context due to the lack of
  thread safety.

#### State-Dependent Behavior

- **Interface Status:** Functions like `moca_IfGetDynamicInfo` and `moca_IfGetStats` will return
  different information depending on the current state of the MoCA interface (Up, Down, Dormant,
  etc.).

- **`ACA` Process:** Functions like `moca_setIfAcaConfig`, `moca_getIfAcaConfig` and
  `moca_cancelIfAca` are specifically related to the Automatic Channel Adaptation (`ACA`) process.
  `moca_setIfAcaConfig` is the one function in this interface whose return value depends on current
  state: it returns `STATUS_INPROGRESS` when a run is already active instead of starting a new one
  (`moca_hal.h`:1816, :1842-1843). The other two are defined whichever condition the interface is in:
  `moca_getIfAcaConfig` reads the parameters in any `ACA` condition and changes nothing (:1859-1860,
  :1930), and `moca_cancelIfAca` states no pre-condition beyond a valid interface index and reports
  the same success whether a run was active or not (:1927-1930, :1941-1943). What each of them is
  documented to leave unspecified is set out in "State Diagram".

- **State Model:** This interface reports state and does not model it. The `ACA` run's condition is
  read out of `moca_aca_stat_t` and the interface's condition out of `moca_dynamic_info_t::Status`;
  in neither case does a declaration state which value may follow which, what causes a change or in
  what order values occur, and the interface-status declaration says so in those terms
  (`moca_hal.h`:243-245). "State Diagram" therefore presents the documented effect of each operation
  and the observable value sets, and draws no transition. A caller must treat any of these values as
  a current observation and nothing more.

*Derived from `include/moca_hal.h`:250-259, :315-342, :723-818, :925-955, :1020-1055, :1270-1327 and
:1397-1478, and the theory-of-operation material carried by the predecessor of this page.*

### Data Structures and Defines

Every type below is declared in [`moca_hal.h`](../../include/moca_hal.h) under the
`MOCA_HAL_TYPES` Doxygen group, and the member-level meaning of each field is carried by the `/**< */`
comment on the field itself. Line numbers are given so a caller can go straight to the declaration.

**Enumerations.** There are exactly two in the active interface.

| Type | Declared at | What it represents |
| --- | --- | --- |
| `moca_if_status_t` | :250-259 | The seven possible states of a MoCA interface — `IF_STATUS_Up` (1), `IF_STATUS_Down` (2), `IF_STATUS_Unknown` (3), `IF_STATUS_Dormant` (4), `IF_STATUS_NotPresent` (5), `IF_STATUS_LowerLayerDown` (6) and `IF_STATUS_Error` (7). Reported through the `Status` member of `moca_dynamic_info_t`. **`MOCA_VAR`-conditional.** |
| `PROBE_TYPE` | :611-618 | The probe an `ACA` run uses — `PROBE_QUITE` (0), a quiet probe that transmits no signal, or `PROBE_EVM` (1), which transmits a signal to measure quality as `EVM`. Supplied in the `Type` member of `moca_aca_cfg_t`. |

The header also contains a third enumeration, `ACA_STATUS`, which is **excluded from compilation** —
it is enclosed in a disabled conditional block at :281-290 — and is therefore **not part of the
interface**. It is named here only so that a reader who finds it in the header knows why it cannot
be referenced from calling code; it must not be treated as an available type. The live `ACA` result
values are the four `STATUS_*` macros listed further down and the `stat` member of
`moca_aca_stat_t`, whose documented values are 0 for success, 1 for a bad channel, 2 for a missing
`EVM` probe, 3 for failure and 4 for in progress.

**Structures.** All fourteen are `typedef`-ed structures passed by pointer.

| Type | Declared at | What it represents |
| --- | --- | --- |
| `moca_cfg_t` | :315-342 | The writable configuration of an interface: alias, enable and privacy flags, preferred-`NC` flag, frequency and taboo masks, passphrase, transmit and beacon power limits, bandwidth thresholds, mixed mode, channel scanning and the reset flag. Exchanged by `moca_GetIfConfig` and `moca_SetIfConfig`. |
| `moca_static_info_t` | :337-351 | Properties fixed for the life of the interface: name, MAC address, firmware version, maximum bit rate, highest supported MoCA version, frequency capability and network taboo masks, and the `QAM256` and packet-aggregation capability flags. |
| `moca_dynamic_info_t` | :391-411 | Live interface and network state: status, last change, ingress and egress bandwidth maxima and their threshold flags, current MoCA version, `NC` and backup `NC` node IDs, local node ID, current and last operating frequency, connected client count, `NC` MAC address and link uptime. **`MOCA_VAR`-conditional.** |
| `moca_stats_t` | :423-442 | Network-layer counters: bytes and packets sent and received, errors, discards, and the unicast, multicast, broadcast and unknown-protocol splits. |
| `moca_mac_counters_t` | :441-449 | MAC-layer packet counters: MAP, reservation request, link control, admission request, probe and asynchronous beacon packets received. |
| `moca_aggregate_counters_t` | :460-464 | Aggregate transmitted and received payload data unit counts, excluding MoCA control packets. |
| `moca_cpe_t` | :469-472 | One `CPE` node, carrying its MAC address. Returned as an array by `moca_GetMocaCPEs`. |
| `moca_associated_device_t` | :477-498 | One node on the MoCA network: MAC address, node ID, preferred-`NC` flag, highest MoCA version, transmit and receive `PHY` rates, broadcast rates, power levels and reduction, packet and error counts, capability flags, receive signal-to-noise ratio, client count, and the `Active` flag that the notification callback uses to signal a join or a leave. |
| `moca_mesh_table_t` | :506-513 | One entry of the mesh `PHY` rate table: the receiving and transmitting node IDs and the transmit rate between them, plus the MoCA 2.x NPER and VLPER rates. **`MOCA_VAR`-conditional.** |
| `moca_flow_table_t` | :521-533 | One ingress `PQoS` flow: flow ID, ingress and egress node IDs, remaining and initial lease time, destination MAC address, packet size, peak data rate, burst size and flow tag. |
| `moca_assoc_pnc_info_t` | :587-592 | A node's preferred-`NC` information: node index, preferred-`NC` flag and the MoCA version the node supports. This structure is declared for callers that need it; no function in this interface takes or returns it. |
| `moca_scmod_stat_t` | :597-606 | `SCMOD` statistics for one node pair: transmitting and receiving node IDs, the channel, and 512-element modulation, NPER and VLPER arrays. Returned as an array by `moca_getIfScmod`. |
| `moca_aca_cfg_t` | :611-618 | The `ACA` request: node ID, probe type, channel, a bitmask of reporting nodes, and the `ACAStart` flag that decides whether the call configures only or also starts the run. |
| `moca_aca_stat_t` | :623-630 | The `ACA` result: the configuration used, the `stat` code, total received power, a 512-element per-channel power profile, and the `ACATrapCompleted` flag indicating that the profile is ready. |

**Callback type.** One function pointer type is declared, and it is installed by exactly one function.

| Type | Declared at | Installed by | What it represents |
| --- | --- | --- | --- |
| `moca_associatedDevice_callback` | :640-668 | `moca_associatedDevice_callback_register` (:669-705) | `INT (*)(ULONG ifIndex, moca_associated_device_t *moca_dev)` — invoked when a MoCA client is activated or deactivated on the network. See "Asynchronous Notification Model". |

**Macros a caller must interpret.**

| Macro | Value | What it represents |
| --- | --- | --- |
| `kMoca_MaxCpeList` | 256 | Maximum number of `CPE` devices in a MoCA network, and the number of entries `moca_GetMocaCPEs` requires its array to hold. |
| `kMoca_MaxMocaNodes` | 16 | Maximum number of MoCA nodes allowed in a network. |
| `MAC_PADDING` | 12 | Padding added to a 6-byte MAC address to make the 18-byte form some platforms require. Members declared `[6 + MAC_PADDING]` are 18 bytes wide. |
| `STATUS_SUCCESS` | 0 | Success, returned by every status-returning function. |
| `STATUS_FAILURE` | -1 | Generic failure. |
| `STATUS_NOT_AVAILABLE` | -2 | Value or capability not available. |
| `STATUS_INPROGRESS` | -1 | `ACA`: a run is already in progress. Numerically equal to `STATUS_FAILURE`. |
| `STATUS_NO_NODE` | -2 | `ACA`: the specified node does not exist. Numerically equal to `STATUS_NOT_AVAILABLE`. |
| `STATUS_INVALID_PROBE` | -3 | `ACA`: invalid probe type. |
| `STATUS_INVALID_CHAN` | -4 | `ACA`: invalid channel. |
| `TRUE` / `FALSE` / `ENABLE` | 1 / 0 / 1 | Control values for the `BOOL` members of the structures above. |

The header additionally defines the fixed-width aliases `ULONG`, `BOOL`, `CHAR`, `UCHAR`, `INT` and
`UINT` at :107-129, each guarded by `#ifndef` so that a caller which already defines them keeps its own
definition. `BOOL` is an `unsigned char`, so a `BOOL` member must be compared against `TRUE` or
`FALSE` rather than assumed to be a single bit.

*Derived from `include/moca_hal.h`:107-153, :192-222, :238-290, :294-665 and :669-705.*

### API Surface

All 21 declared functions are listed below by exact identifier, grouped by functional area. Per-API
detail — argument direction, valid ranges, array sizing, pre-conditions and the full return-code
list — is carried by the Doxygen block on each declaration in
[`include/moca_hal.h`](../../include/moca_hal.h), under the `MOCA_HAL_APIS` group.

Three of the 21 do not return a status code, and mistaking them for status-returning calls is the
easiest error to make against this interface: `moca_HardwareEquipped` returns a `BOOL`,
`moca_FreqMaskToValue` returns a frequency **value**, and
`moca_associatedDevice_callback_register` returns `void`. The remaining 18 return a status drawn from
"Internal Error Handling".

**Interface configuration:**

| API | Returns | Purpose |
| --- | --- | --- |
| `moca_GetIfConfig` | `INT` status | Reads the configuration parameters currently in force for an interface into a `moca_cfg_t`. |
| `moca_SetIfConfig` | `INT` status | Applies a complete `moca_cfg_t` — frequency mask, channel scanning, privacy passphrase, power limits, bandwidth thresholds and the reset flag. |

**Static and dynamic information:**

| API | Returns | Purpose |
| --- | --- | --- |
| `moca_IfGetStaticInfo` | `INT` status | Reads properties that do not change while the interface is up: name, MAC address, firmware version, maximum bit rate, highest supported MoCA version and capability masks. |
| `moca_IfGetDynamicInfo` | `INT` status | Reads live interface and network state: link status, last change, operating frequency, `NC` identity, connected client count and link uptime. Its out-parameter type is **`MOCA_VAR`-conditional**, so this call is unusable in a build that defines `MOCA_VAR`. |

**Statistics and counters:**

| API | Returns | Purpose |
| --- | --- | --- |
| `moca_IfGetStats` | `INT` status | Reads network-layer statistics for an interface into a `moca_stats_t`. |
| `moca_IfGetExtCounter` | `INT` status | Reads MAC-layer packet counters — MAP, reservation, link control, admission, probe and beacon — into a `moca_mac_counters_t`. |
| `moca_IfGetExtAggrCounter` | `INT` status | Reads aggregate transmitted and received payload data unit counts, excluding MoCA control packets. |
| `moca_GetFlowStatistics` | `INT` status | Reads the ingress `PQoS` flow table into a caller-allocated `moca_flow_table_t` array and reports the entry count. |
| `moca_GetResetCount` | `INT` status | Reads how many times the MoCA module has been reset. The out-parameter is left unchanged if the call fails. |
| `moca_GetFullMeshRates` | `INT` status | Reads the full mesh `PHY` rate table for every node pair. **`MOCA_VAR`-conditional** — absent from a build that defines `MOCA_VAR`. |

**Associated devices:**

| API | Returns | Purpose |
| --- | --- | --- |
| `moca_GetNumAssociatedDevices` | `INT` status | Reads the number of devices on the MoCA network. Call this first to size the array for the next entry. |
| `moca_GetAssociatedDevices` | `INT` status | Reads a `moca_associated_device_t` record for every associated node — MAC address, node ID, `PHY` rates, power levels, capability flags and the `Active` flag. |
| `moca_associatedDevice_callback_register` | `void` | Installs the callback invoked when a MoCA client is activated or deactivated. Reports no failure, and there is no matching de-registration call. |

**`CPE` and utility:**

| API | Returns | Purpose |
| --- | --- | --- |
| `moca_GetMocaCPEs` | `INT` status | Reads the MAC addresses of all MoCA `CPE` nodes and their count. The array must be pre-allocated for `kMoca_MaxCpeList` (256) entries. |
| `moca_FreqMaskToValue` | `INT` frequency value | Converts a frequency bit mask to the corresponding frequency value. Returns the value itself, not a status. The mask interpretation and the valid output range are vendor-specific, and the input buffer should be at least 16 bytes. |
| `moca_HardwareEquipped` | `BOOL` | Reports whether MoCA hardware is present and correctly configured. Returns `TRUE` or `FALSE`, not a status. This is the only readiness check in the interface. |

**`ACA` — Automatic Channel Adaptation:**

| API | Returns | Purpose |
| --- | --- | --- |
| `moca_setIfAcaConfig` | `int` status | Sets the `ACA` parameters and, when `ACAStart` is `TRUE`, starts the run. Returns `STATUS_INPROGRESS` if a run is already active, in which case no new run is started. |
| `moca_getIfAcaConfig` | `int` status | Reads back the `ACA` parameters currently set for an interface. |
| `moca_cancelIfAca` | `int` status | Terminates a running `ACA` process. Succeeds and has no effect if none is active. |
| `moca_getIfAcaStatus` | `int` status | Reads the status and results of an ongoing or completed run: the configuration used, the `stat` code, total received power, the power profile and the completion flag. |
| `moca_getIfScmod` | `int` status | Reads the `SCMOD` statistics collected after a run — modulation, NPER and VLPER per subcarrier for each node pair. |

*Derived from `include/moca_hal.h`:669-705 and :723-1478. The identifier list was extracted from the
header's declarations; this table names all 21 and nothing that the header does not declare.*

### Sequence Diagram

The exchange below uses only declared identifiers. `Vendor Software` denotes the vendor
implementation behind `libhal_moca`.

```mermaid
sequenceDiagram
participant Client Module
participant MoCA HAL
participant Vendor Software

Client Module ->>MoCA HAL: moca_HardwareEquipped()
MoCA HAL->>Vendor Software: check hardware presence
Vendor Software ->>MoCA HAL: hardware present
MoCA HAL->>Client Module: TRUE

Client Module ->>MoCA HAL: moca_IfGetStaticInfo()
MoCA HAL->>Vendor Software: read fixed interface properties
Vendor Software ->>MoCA HAL: name, MAC, firmware, capabilities
MoCA HAL->>Client Module: moca_IfGetStaticInfo() return

Client Module ->>MoCA HAL: moca_GetIfConfig()
MoCA HAL->>Vendor Software: read current configuration
Vendor Software ->>MoCA HAL: configuration parameters
MoCA HAL->>Client Module: moca_GetIfConfig() return

Client Module ->>MoCA HAL: moca_SetIfConfig()
MoCA HAL->>Vendor Software: apply configuration
Vendor Software ->>MoCA HAL: applied
MoCA HAL->>Client Module: moca_SetIfConfig() return

Client Module ->>MoCA HAL: moca_associatedDevice_callback_register()
MoCA HAL->>Client Module: void

Client Module ->>MoCA HAL: moca_IfGetDynamicInfo()
MoCA HAL->>Vendor Software: read link and network state
Vendor Software ->>MoCA HAL: status, frequency, node identity
MoCA HAL->>Client Module: moca_IfGetDynamicInfo() return

Client Module ->>MoCA HAL: moca_GetNumAssociatedDevices()
MoCA HAL->>Vendor Software: count nodes on the network
Vendor Software ->>MoCA HAL: node count
MoCA HAL->>Client Module: moca_GetNumAssociatedDevices() return

Client Module ->>MoCA HAL: moca_GetAssociatedDevices()
MoCA HAL->>Vendor Software: read per-node records
Vendor Software ->>MoCA HAL: node records
MoCA HAL->>Client Module: moca_GetAssociatedDevices() return

note over Vendor Software: a node joins or leaves the network
Vendor Software ->>MoCA HAL: associated device event
MoCA HAL->>Client Module: registered callback, Active TRUE or FALSE
```

*Derived from `include/moca_hal.h`:575-603, :669-705, :723-955 and :1020-1159, and the diagram carried
by the predecessor of this page.*

### State Diagram

**This interface reports state and does not model it, so no transition is drawn anywhere in this
topic.** Two things carry state: the `ACA` run, whose condition a caller reads out of
`moca_aca_stat_t`, and the interface itself, whose condition is reported through
`moca_dynamic_info_t::Status`. For each of them the declarations establish two things — the effect a
successful operation has, and the set of values a reading can return — and nothing about which value
may follow which. What follows is therefore the effects and the value sets, and an explicit account
of what the interface leaves unspecified.

**The `ACA` values a caller can observe.** `moca_aca_stat_t::stat` is a plain `INT` documented as `0`
`SUCCESS`, `1` `FAIL_BADCHANNEL`, `2` `FAIL_NOEVMPROBE`, `3` `FAIL` and `4` `IN_PROGRESS`, and
`ACATrapCompleted` is a `BOOL` that is `TRUE` only once the power profile is ready to read
(`moca_hal.h`:661, :664). The matching `ACA_STATUS` enumeration is enclosed in a permanently disabled
`#if 0` block and is **not part of this interface**, so a caller compares against the numbers rather
than the names (:275-290). Both value sets, drawn as declared values with **no edge between them,
because the interface specifies none**:

```mermaid
stateDiagram-v2
    state "ACA run status reported in moca_aca_stat_t::stat" as aca {
        state "0 - SUCCESS" as s0
        state "1 - FAIL_BADCHANNEL" as s1
        state "2 - FAIL_NOEVMPROBE" as s2
        state "3 - FAIL" as s3
        state "4 - IN_PROGRESS" as s4
    }
    state "Power-profile readiness reported in moca_aca_stat_t::ACATrapCompleted" as trap {
        state "FALSE - the power profile is not ready to read" as t0
        state "TRUE - the power profile is ready to read" as t1
    }
```

**What each `ACA` operation is documented to do, and what it leaves open.** The effect of a
*successful* call is established per declaration; a resulting or required predecessor value is not.
All locators are in `include/moca_hal.h`.

| Operation | Documented effect of success | What it does not establish |
| --- | --- | --- |
| `moca_setIfAcaConfig` with `ACAStart` `TRUE` | The run begins immediately (:1773), and the parameters supplied are in force (:1802-1803). | How long a run takes, and any notification that it has finished — polling is the only mechanism (:1827-1829). |
| `moca_setIfAcaConfig` with `ACAStart` `FALSE` | The configuration parameters are set and no run starts (:1803). | Nothing about the value `stat` reports before, during or after that call. |
| `moca_setIfAcaConfig` while a run is active | The call is rejected with `STATUS_INPROGRESS`, nothing is changed and the existing run continues (:1805, :1816, :1842-1843). | Which of the two outcomes occurred, from the return value alone: `STATUS_INPROGRESS` and `STATUS_FAILURE` are both `-1`, so a caller must read `moca_getIfAcaStatus` to tell them apart (:1840-1843). On any other failure, whether a parameter was applied (:1805-1807). |
| `moca_getIfAcaConfig` | The parameters currently in force are read, in any `ACA` condition, and nothing is changed (:1859-1860, :1881). | A default before the first `moca_setIfAcaConfig` (:1881-1883). It is not a run indicator either: `ACAStart` reflects the value last set, not whether a run is active (:1884-1885). |
| `moca_cancelIfAca` | On `STATUS_SUCCESS` no `ACA` run is active on the interface (:1930). | **That the run had stopped by the time the call returned**, and any bound on how long termination takes (:1948-1950). Whether a cancelled run's results stay readable through `moca_getIfAcaStatus` or `moca_getIfScmod`, and whether the configuration survives (:1930-1933). Whether a run was active at all, since success covers both cases (:1941-1943). The outcome of a cancellation racing another caller's start (:1955-1956). |
| `moca_getIfAcaStatus` | Reads `stat`, the configuration used, `RxPower`, `ACAPowProfile` and `ACATrapCompleted` for an ongoing or completed run (:1402-1403, :2000-2004). | Nothing about the run itself: success reports that the **read** succeeded, not that the run did (:2009-2011). What the members hold when no run has ever been requested (:2039-2042). |
| `moca_getIfScmod` | Reads the `SCMOD` statistics collected after a run (:2048). | Whether a failure means "no run has completed" or a read error — the two are indistinguishable, so a caller confirms completion with `moca_getIfAcaStatus` first (:2106-2109). |

**What is deliberately left unspecified rather than drawn.** Each of these was looked for in the
header and is absent, and drawing it would put a contract in front of a downstream test author that
the interface does not make:

- **No initial value.** Nothing states what `stat` reports before any run has been requested — the
  polling call states in terms that this interface does not say what the members hold when no run has
  ever been requested (:1411-1414) — so there is no value for a start marker to point at.
- **No ordering.** No declaration names a value as a required or forbidden predecessor of another,
  and no declaration reports a change of value; a caller reads the current value and cannot infer a
  sequence from two readings.
- **No terminal value and no restart edge.** `4` `IN_PROGRESS` and the three failure values are
  readings, not positions, and the interface neither states that a completed run's reading persists
  nor that requesting a new run replaces it.
- **No completion of a cancellation.** This is the sharpest case, and it is why the previous revision
  of this topic was wrong to draw an edge for it: `moca_cancelIfAca` does not state that the run has
  stopped when it returns, and reports the same success whether a run was cancelled or none was
  active (:1941-1943, :1948-1950). An edge would assert both a destination and a moment the interface
  refuses to name. A caller that needs to know a cancellation took effect reads
  `moca_getIfAcaStatus` and treats a `4` as "still running" (:2011-2013).

**What a caller may rely on, and what it must not.** A caller may rely on `stat` reporting one of
those five values, on `ACATrapCompleted` being `TRUE` only when the power profile is ready to read,
and on each operation above having its documented effect when it returns success. It must not treat a
value as reachable only from a particular predecessor, must not assume it will observe every value —
nothing states a dwell time, and no event announces a change — and must not track the run's condition
locally in the expectation of being told when it changes. The reliable way to establish the current
condition is to call `moca_getIfAcaStatus` again.

**Interface status is not a state machine, and this document does not draw one.** The seven values of
`moca_if_status_t` — `IF_STATUS_Up`, `IF_STATUS_Down`, `IF_STATUS_Unknown`, `IF_STATUS_Dormant`,
`IF_STATUS_NotPresent`, `IF_STATUS_LowerLayerDown` and `IF_STATUS_Error` — are reported through the
`Status` member of `moca_dynamic_info_t`. **This interface does not specify the transitions between
them**: the declaration says so in those terms — "These are observed values, not a state machine:
this interface specifies no transition between them, no event that causes one and no ordering, and no
call in the interface moves an interface from one value to another" (`moca_hal.h`:243-245). A caller
must read the current value and must not assume an ordering. The same holds for `LastChange`, which
timestamps that a change occurred without describing it.

*Derived from `include/moca_hal.h`:243-259 (the interface status values and the declaration's own
statement that they are not a state machine), :275-290 (the disabled `ACA_STATUS` enumeration),
:611-618 (`Status` and `LastChange`), :636-665 (`moca_aca_cfg_t` and `moca_aca_stat_t`, including the
`stat` and `ACATrapCompleted` members) and :1270-1478 (the five `ACA` declarations and their
pre-conditions, post-conditions, return values and notes).*
