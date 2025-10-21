# Exercise Sheet 02

**Team:** Marco Fröhlich and Lilly Schönherr

# Task 2:
This task was done without looking at the code at first. We simply asked different AI tool (ChatGPT, Mistral, Claude) to debug the code. For that initially the following prompt was used: 
```
Hey, the following code uses openMPI and does work. Please fix it and explain what the problem is.
<Code>
```
All tools were used in the free tier.

## Example 1:
### ChatGPT:
ChatGPT identified these problems and implemented solutions for all of them:

- Identifies the problem as mismatching tags in the `MPI_Send()` and `MPI_Recv()` calls.
- Suggests that the root nodes does not need to send data to itself.
- Removed shadowed `i` variable in the initialization loop.

This fixed the code.

### Mistral
Mistral identified the following issues and implemented solutions for all of them:

- Root process only sends to ranks $> 0$.
- Same tag used for send and receive.
- Removed unnecessary initialization for non-root processes.
- No self send.

This fixed the code. Interestingly the bot mentioned to remove the data initialization for non-root ranks but it is still in the code, only it changed the value from 1 to 0. Also self-sending is mentioned twice as a problem.

### Claude
Claude only identified the mismatch of tags as a problem and fixed it. In theory this fixed the code, but the code produced the following warning/error on runtime `mpool.c:43   UCX  WARN  object 0x1501f9cedfc0 was not returned to mpool ucp_am_bufs`. This was then reported back and the tool also disabled self-send for rank 0. Interestingly Claude decided to remove indentations for its resulting code, all unnecessary whitespace was removed. But on the other hand it did not care about unused or unnecessary variables.

## Example 2:
### ChatGPT
The bot identified two typos in the neighbor calculation. Inside the `MPI_SendRecv()` and `MPI_Recv()` an assignment was used `rank = 1 + size`, which reassigns the rank. This was solved by using `rank - 1 + size`.

### Mistral
Mistral identified the following problems:

- *Incorrect receiver calculation:* fixed by using `(rank - 1 + size) % size`
- *Consistent datatype usage:* Both send and receive use `newType` for consistency.
- *Resource management:* Added `MPI_Type_free(&newType)` to avoid memory leaks.

### Claude
Claude found the following problems:

- Assignment operator in ring calculation: `(rank = 1 + size) % size` uses `=` (assignment) instead of `-` (subtraction). This overwrites the rank variable! Should be `(rank - 1 + size) % size` to get the previous rank in the ring.
- Inconsistent datatype usage in Sendrecv: The send part uses `newType` (1 element of type newType = 2 ints), but the receive part uses `MPI_BYTE` with `sizeof(int) * 2`. This mismatch causes the receive buffer to interpret the data incorrectly. Both should use `newType` with count 1.
- Inconsistent datatype in Recv: Same issue as above—should receive 1 element of newType, not `sizeof(int) * 2` bytes of `MPI_BYTE`.
- Missing comment fix: The comment said `/)` instead of `//` for the first section.

Interestingly Claude now uses code formatting and also identified a problem with a comment in the first section that is present in the input code but only of cosmetic relevance.

## Conclusion
All used models found the underlying problem in both code examples on the first try. The interesting part is what else the models care about. Especially Claude is here an interesting example, one time it does not care about code formatting but on the second try it complains about typos in code comments.