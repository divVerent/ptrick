Binary verification proof of concept - avoiding the stat/exec race condition by
means of ptrace and procfs.

As proof of concept, this program will execute a binary, and print its size.

The testing program does the following:

1. find file size via stat
2. execute program, watch it
3. find file size of just executed program via stat

Observe: the program actually executed, and the size found in 3., will always
match. The size in 1. will not always match.

Example output:

$ sh tester.sh
+ gcc ptrick.c -o ptrick -Wall -Wextra
+ gcc race1.c -o race1 -Wall -Wextra
+ gcc race2.c -o race2 -static -Wall -Wextra
+ set +x -- starting switcher
+ n=0
+ nmax=1000
+ set +x -- running loop 1000 times
      2 Pre-exec size: 4939 ptrick: execv: No such file or directory Program exited (status 29). Tracing was NOT active yet!
    141 Pre-exec size: 4939 Real size: 4939 Hello, earth! Program exited (status 0).
    337 Pre-exec size: 4939 Real size: 770230 Hello, sun! Program exited (status 0).
      5 Pre-exec size: 770230 ptrick: execv: No such file or directory Program exited (status 29). Tracing was NOT active yet!
    341 Pre-exec size: 770230 Real size: 4939 Hello, earth! Program exited (status 0).
    170 Pre-exec size: 770230 Real size: 770230 Hello, sun! Program exited (status 0).
      4 ptrick: stat: No such file or directory
+ kill 13818

Note that creating links, overriding the previous assignment, is not atomic;
this is why sometimes execution fails entirely. That is usually no security
issue, though. We only demand that the stat() output matches the binary
executed, and not the other one...
