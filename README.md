![build][1] ![docker-image][2]

# Cashmere

Cashmere is a distributed conflict-free replicated registry database. It's
designed to allow multiple devices from the same pool to exchange data in
between themselves in a conflict free manner so that all connected nodes would
share and converge on the interpretation of the data.

The current state of the code has gRPC communication (synchronous only at this
point), efficient append only data base and automatic conflict resolution
through an underlying CRDT implementation using vector clocks (map clocks
really).

This is a generic infraestructure that will enable applications in multiple
domains (see [First Intended Application](#first-intended-applicaiton)).

You can refer to the Dockerfile to see a simple test where two connected nodes
share data and to the auto-tests to see some more examples of use.

## Current Status


| Feature              | Column 2           |
| -------------------- | ------------------ |
| CRDT                 | :white_check_mark: |
| Connectivity         | :white_check_mark: |
| Replication          | :white_check_mark: |
| GRPC (Sync)          | :white_check_mark: |
| GRPC (Async)         | :x:                |
| Transport Encryption | :x:                |


## Contributing

The directions of the project are currently being set, so please reach out to
discuss ideas before sending a patch.

All contributors must digitally sign the [Contributors License Agreement](CLA).

All commits must signed-off and digitally signed (git commit -sS) to indicate
that the submitter accepts the [DCO 1.1](https://developercertificate.org/).

## First Intended Application

The first applicaiton of Cashmere is to build a privacy respecting a financial
accounting software intended to work across multiple devices and with local-only
data storage with automatic conflict resolution.

Data sharing will happen with encrypted true end-to-end communication directly in
between devices. If direct communication is not possible, a message relay
service (with no persistent storage) can be used to relay communication in
between devices.

### Workflow Example

A user wants to track their bank and investments accounts using the Cashmere
smartphone app and later wants to see all recorded transactions on their
desktop.

After installing the Cashmere desktop app, they select the pairing option,
which will display a QR code on the desktop screen. The user selects the
'include device via QR code' option on the smartphone and points the camera to
it. The devices then start to share their transactions and will do so
automatically whenever they are both in reach of each other.

### Functionalities

* Income/Expense tracking
* Reports
* Budgeting

### Privacy Considerations

* No personal user data is required, the app will create is own unique id and
  one of the devices id will be used to identify the accounts.
* No cloud storage.

### Architecture

Composed of:
* Client: uniquely identifiable transaction recorder.
* Pool: group of clients that share transactions in between them
* Broker: client message broker. Clients may announce their presence to other
  members of the same pool only.
* Protocol: communication protocol in between clients via a broker. A client can
  1. announce it's presence by publishing its state (vector clock)
  2. publish transactions (id, transaction data)
  3. request transactions from all clients
  4. a gossip protocol can be used to monitor clients state and trigger required
     updates

A pool is composed of one or more clients. All transactions are shared amongst
all pool members via a Broker.

All client must be uniquely identified. A client can join a pool at any time by
being given the pool secret pass. (typed or read via QR code).

The broker should store minimum information (transient only) to allow
end-to-end communication in between clients.

## Requirements

- CMake
- C++ compiler std-23
- gRPC >= 1.51.0
- googletests >= 1.17.0

## Building

To configure, build and run the tests run:

```
cmake --workflow --preset release
```

[1]: https://github.com/aeliton/cashmere/actions/workflows/c-cpp.yml/badge.svg?branch=main
[2]: https://github.com/aeliton/cashmere/actions/workflows/docker-image.yml/badge.svg?branch=main
