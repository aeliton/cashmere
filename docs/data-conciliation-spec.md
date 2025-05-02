# Data Conciliation

As each client records and share transactions they must create the same local
model independently of the order they received the transactions.

In order to achieve this we can use vector clocks: each client will increment
their counter whenever they perform a change on their data and then share with
the other devices the updates (how the data is shared will be discussed on a
different moment).

For example, on a scenario with two clients, let's call them A and B, the state
can be represented by the vector `[0, 0]` initially, where the entries refer to A
and B respectively. Let's say that A records a new transaction, it's local
vector would become `[1, 0]` and this would be eventually shared with B, that
would update it's own local vector to `[1, 0]`. They would then display the same
data consistently.

I the where B also registers transactions, they would eventually get to A and
because the would have the same logic of creating the visualization, they must
show exactly the same content.

# Conflict resolution

Let's construct a case with 3 clients (A, B and C) and assume that all of them
are on the same state `[3, 7, 5]`.

Let's say the user records a transaction of value 123,30 on the device B and
unknowingly introduces two mistakes: a) adding an extra 0; and b) the last 3
should have been a 4.

Having all the devices offline, the user checks A and spots the first mistake
and fixes it. A now has the clock `[4, 7, 5]` and the fixed transaction shows
12.33. Later, when checking his bank app
the user notices that second mistake and fixes it on the device C to the
corrects the transaction to 123,40. C has the clock `[3, 7, 6]`.

When the devices are online they will propagate their changes and the cha

# How to ensure the view of data

* CRDT
* Model/View
* Efficiency considerations
* Data Pruning
