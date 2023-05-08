# Multiprocessed-monitoring-tool

This is an augmented version of the system monitoring tool. It utilizes forks and pipe so that it runs concurrently and is multiprocessed. To compile this program, open a terminal in the directory with all the files in it and run "make". To then execute it, run ./main.out with your desired command line arguments.

Here are the command line arguments it accepts:

--system
        to indicate that only the system usage should be generated


--user

        to indicate that only the users usage should be generated


--graphics

        to include graphical output in the cases where a graphical outcome is possible as indicated below.


--sequential

        to indicate that the information will be output sequentially without needing to "refresh" the screen (useful if you would like to redirect the output into a file)

 

--samples=N

        if used the value N will indicate how many times the statistics are going to be collected and results will be average and reported based on the N number of repetitions.
If not value is indicated the default value will be 10.


--tdelay=T

        to indicate how frequently to sample in seconds.
If not value is indicated the default value will be 1 sec.

 

The last two arguments can also be considered as positional arguments if not flag is indicated in the corresponding order: samples tdelay.
