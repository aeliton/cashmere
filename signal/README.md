# Cashmere

Cashmere is privacy respecting a financial accounting software intended to work
across multiple devices and with local-only data storage.

Data sharing happens with encrypted true end-to-end communication directly in
between devices if they can communicate directly. If direct communication is
not possible, a message relay service (with no persistent storage) can be used
to relay communication in between devices.

## Workflow Example

A user wants to track their bank and investments accounts using the Cashmere
smartphone app and later wants to see all recorded transactions on their
desktop.

After installing the Cashmere desktop app, they select the pairing option,
which will display a QR code on the desktop screen. The user selects the
'include device via QR code' option on the smartphone and points the camera to
it. The devices then start to share their transactions and will do so
automatically whenever they are both in reach of each other.

## Functionalities

* Income/Expense tracking
* Reports
* Budgeting

## Privacy Considerations

* No personal user data is required, the app will create is own unique id and
  one of the devices id will be used to identify the accounts.
* No cloud storage.

## Architecture

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
- C++ compiler std-17

## Building

To configure, build and run the tests run:

```
cmake --workflow --preset release
```

## Open problems

* DDoS prevention on devices and message queue
