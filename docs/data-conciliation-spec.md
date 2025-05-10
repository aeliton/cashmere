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

Conflicts will happen when the same transaction is edited on two or more
different devices concurrently. In such cases is not possible for the merge
algorithm to know which one is the correct value and the resolution should be
arbitrary. We can pick the device with the smaller id (lexicographical) as the
winning.

Because all the transactions will be eventually shared, it will be easy to show
to users an UI indication that a 'automatic' conflict resolution took place for
a particular transaction. This would invite the user to review the transaction
and potentially select another resolution that would be taking priority.

An example of conflict would be:
In the example bellow, A and B are the ID of the devices, the square brackets
`[]` will show the sum of transactions seen by the user and the parenthesis
display the vector clock of each device:

```
Time    A [ 0.00]                             B [ 0.00]
 0  .-- * add  5.00 [ 5.00] (1, 0) -.         * add  2.00 [ 2.00] (0, 1) ----.
 1  '---v                           '------>  * add  5.00 [ 7.00] (1, 1) --. |
 2  .-- * edit 4.00 [ 8.00] (2, 0)         .- * edit 6.00 [ 8.00] (1, 2) <-' |
 3  .-- * add  3.00 [ 7.00] (3, 0)         |                                 |
 4  |   * add  2.00 [ 9.00] (3, 1) <-------+---------------------------------'
    |   * edit 6.00 [ 9.00] (3, 2) <-------'
 5  |------------------------------------->  * add  3.00 [11.00] (2, 2)
                                     '---->  * edit 4.00 [ 9.00] (3, 2)
```

* Up to the time 1, there's a single shared transaction of value $5.00 from A to
B. A is not aware yet of the B's $2.00 transaction.
* In time 2, both devices edit the same shared transaction witch will cause a
  conflict when the devices receive each others edits. In
* In time 3 A adds a $3.00 transaction
* In time 4, A receives two updates from B, including the edit. The conflict is
  resolved automatically by choosing the smaller lexicographical ID as the
  winner.
* In time 5, A shares the edit alongside the with an addition. The conflict of
  the edit is then resolved in B, with both devices converging to the same
  agreed state.

Under the hood the data is available and can be used to indicate through user
facing UI that an automatic conflict resolution was made, which would help the
user to double check and potentially override the automatic conflict resolution
by creating an edit transaction that will alter the previous resolution.

# How to ensure the view of data

* CRDT
* Model/View
* Efficiency considerations
* Data Pruning
