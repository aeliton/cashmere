# Broker

The broker will allow devices to announce their presence by publishing their
clock version. The broker will keep the pool data in memory only.

The broker will be able to identify when devices diverged in their transactions
by identifying the differences in the clocks of devices on a given pool.

As soon a device is updated or has become present, the process of
synchronization starts.

The broker can work as a relay in between devices.

# Presence

Devices will use the broker whenever their pool contains more than one device.

When a device starts, it announces their presence to their broker. The broker
will find a way to connect to the broker of other devices and triggers callbacks
when the version clocks change on the online devices.

* publish transactions
* announce presence via broker
* announce presence via 'direct communication' (network?, Bluetooth, NFC?)
