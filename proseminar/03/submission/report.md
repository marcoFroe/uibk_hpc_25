# Exercise Sheet 02

**Team:** Marco Fröhlich and Lilly Schönherr
# Task 1:
For this task, the mistakes of the code were fixed the classic way. 

First, -Wall and -Wextra were enabled in the Makefile. Calling make, produced the following output:
``` 
example_1.c: In function 'main':
example_1.c:13:9: warning: unused variable 'i' [-Wunused-variable]
   13 |     int i;
      |         ^
example_2.c: In function 'main':
example_2.c:32:28: warning: operation on 'rank' may be undefined [-Wsequence-point]
   32 |                      (rank = 1 + size) % size, 123, MPI_COMM_WORLD, &status);
      |                      ~~~~~~^~~~~~~~~~~
```
The next step was looking through the code itself. In addition to removing the declaration of the unused variable `i` which was caught by the compiler, the following discoveries and changes were made in example 1:
- Brackets were added to the loop starting at line 26. This does not change the behaviour of the program but increases its readability. 
- The send and receive calls read/write to `&data`. As `data` is an array, we do not need the `&` operator here. 
- Rank 0 sends its data to `ranks 0 - (nprocs - 1)`. As `MPI_Send` is blocking, this will result in a deadlock. We changed the loop header to initialize `i` to 1.
- The receive call uses `tag2` instead of `tag`; We changed it to use `tag` and deleted `tag2` as a whole. 

Next, we had a look at the code of example 2. 
- Twice, instead of 1, `sizeof(int) * 2` is used for the number of elements to be sent.
- In the receive part of the `MPI_Sendrecv` and `MPI_Recv`, `MPI_BYTE` is used instead of our custom data type. 
- In the receive part of the `MPI_Sendrecv` and `MPI_Recv`, the sender is calculated with `(rank = 1 + size) % size`. The first of these statements was also flagged by the compiler. 
- The use of the blocking send for a ring buffer will lead to a deadlock; We changed it to the non-blocking version. 

When calling make again after these changes, this was the output:
``` 
/usr/site/hpc/spack/v0.19-lcc3-20230919/opt/spack/linux-rocky8-westmere/gcc-12.2.0/openmpi-3.1.6-d2gmn55g7hoinwfuk2lc3ibz6odzujak/include/mpi.h:1555:20: note: declared here
 1555 | OMPI_DECLSPEC  int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest,
      |                    ^~~~~~~~~
make: *** [Makefile:11: example_2] Error 1
```
This error is the result of not looking up the parameters of `MPI_Isend` and naively assuming it has the same signature as `MPI_Send`. After looking up how to use the function and adding an MPI_Request request, everything compiles without complaint.

We then ran the code of example 1 with `/scratch/c703429/software/must-1.9.1/bin/mustrun -n 4 example_1`. The resulting output tells us that MUST detected no MPI usage errors nor any suspicious behavior during this application run.

When repeating this for example 2, there is a `MUST_ERROR_LEAK_DATATYPE` and a `MUST_ERROR_LEAK_REQUEST`. After doing some research, we found that custom MPI datatypes and requests need to be freed at the end of the program. After adding the corresponding calls to the code, the MUST output for example 2 does not contain any errors either. 


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

# Conclusion
In summary, a lot of errors can be corrected simply by looking at the code. However, you cannot correct things you don't know are wrong. In such cases, tools such as sanitizers and debuggers are of great help.

When it comes to the AI tools, all models fixed the code in a way that it terminates without errors. But when running the examples with the *MUST* tool the first one exits without error. In the second one the tool notices a possible deadlock, even though the code exits without reporting any issues. The error of `&data` being used instead of `data` in example 1 was detected by none of the models.

It is also interesting what else the models care about. Especially Claude is here an notable example, one time it does not care about code formatting but on the second try it complains about typos in code comments. 

In summary, although debugging with AI tools is faster, it is less accurate. 