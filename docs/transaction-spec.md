# Transactions

A transaction can be added, deleted and modified.

Due to the replicated nature of the application, we need to make sure we will
keep consistency in place and apply the same modifications to data in all
instances.

For addition this is trivial. We increment the current state with the new
transaction and we share the information with all devices that are online to
receive the update.

For editing and deleting we can introduce a special transaction that will
change an existing transaction by applying its reverse amount (deletion) or by
introducing an offset to the date or value of the transaction (editing).

A strategy of pruning can be devised fix the history for all devices. This can
be thought through later on.

# Transaction structure

Each transaction is uniquely identifiable on its origin device by an
increment-only id starting from 0. The type if the transaction ID must be
`unsigned long int` (64 bits).

The payload of a transaction can be composed of one or more source accounts and
can have one or more destination accounts. This is to enable split
transactions.

| type | DID    | TID | alters | timestamp | src      | dst       | values |
| ---- | ------ | --- | ------ | --------- | -------- | --------- | ------ |
| Add  | 01be02 | 1   | -      | 20250324  | checking | groceries | 10000  |
|      |        |     |        |           | savings  | groceries | 2000   |

As mentioned previously, a transaction needs to have a type in order to allow
addition, deletion and editing. We use an enumeration to indicate that:

```c++
enum TransactionType {
  Add,
  Edit,
  Delete
};
```

It makes sense to have the transaction type as part of the transaction data
structure because the transaction history is part of the model that will then be
rendered on a view.

A device ID (DID) and transaction ID (TID) can identify uniquely a transaction
in a group of devices.

| type | DID    | TID | alters     | timestamp | src      | dst       | values |
| ---- | ------ | --- | ---------- | --------- | -------- | --------- | ------ |
| Edit | 01be02 | 2   | 01be02/1/0 | 20250324  | checking | groceries | 1000   |

A `Edit` type transaction alters a previous transaction by indicating on its
`alters` column which device, TID and line(s) are being edited. In the above
table we can see that line 0 of the transaction of ID 1 on the device `01be02`
has its value being updated to 1000.

| type   | DID    | TID | alters   | timestamp | src      | dst       | values |
| ------ | ------ | --- | -------- | --------- | -------- | --------- | ------ |
| Delete | 01be02 | 3   | 01be02/1 | 20250324  | checking | groceries | 1000   |

A similar transaction with type `Delete` would indicate to the model to ignore
`01be02-1`. Note that no lines are indicated, meaning that all lines of the
altered transaction must be ignored.

Any transaction can be added, edited or deleted from any device that is part of
the pool.

# Storage specification

TBD
