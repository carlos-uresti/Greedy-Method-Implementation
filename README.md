# fileSystem

1. Suppose you have several baskets of laundry to wash and dry, but only one washer and one dryer. For each basket of
laundry, you know the number of minutes to wash and the number of minutes to dry. To minimize the total time (the
makespan), https://en.wikipedia.org/wiki/Johnson%27s_rule is used to reorder the baskets.

2. a. C program to read the two times for each basket, perform the sort, then apply Johnson’s rule, and finally print
details of the schedule. The first line of the input will be n, the number of baskets. Each of remaining n lines will contain
a pair of non-negative integers for a basket’s washing and drying times. The baskets will have indices from 0 to n - 1 to
correspond to the order in which they are read. Input is read from standard input.
b. The schedule is printed with one output line per basket with the following details in this order: index, washing
time, drying time, start time on the washer, start time on the dryer. Note that the schedule starts at time 0. After printing the
entire schedule, makespan is printed. There is not account for drye idle time.
