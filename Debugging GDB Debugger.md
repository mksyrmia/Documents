# Debugging of GDB debugger
## Goal
Compile debug version of GDB (`-g` option) and use system `gdb` to debug compiled one. In case system `gdb` isn't present, either compile release version (without `-g` option) of `gdb` or simly install it from software center. Debug version should run `test` program whose source code is:
```c
#include <stdio.h>

int main()
{
	int a = 5;
	int *pa = &a;
	printf("'a' is at address '%p'.\n'a' has value '%d'\n", pa, *pa);

	return 0;
}
```
Compile `test` using:
```bash
gcc -g test.c -o test
```
## Compilation of GDB
Let's suppose you want to build `gdb` in `$HOME/builds` folder. 
```bash
cd "$HOME"
# The following two lines should be uncommented just in case you haven't already downloaded gdb repo
git clone 'git://sourceware.org/git/binutils-gdb.git'
cp -r binutils-gdb binutils-gdb-backup   
cd binutils-gdb
mkdir build && cd build
../configure --enable-tui --enable-source-highlight --with-python=python3 --prefix="$HOME/builds"
make -j$(nproc) >build.log
echo $?
```
If build succeeds, `0` should be printed as the last line in terminal. If it fails, you should find error in `build.log` file, which can be identified using:
```bash
grep -C4 -i 'error' build.log
```
The most common error is missing dependencies in your system. After you install them, retry build process:
```bash
cd $HOME
rm -rf binutils-gdb
cp -r binutils-gdb-backup binutils-gdb
cd binutils-gdb
mkdir build && cd build
../configure --enable-tui --enable-source-highlight --with-python=python3 --prefix="$HOME/bin"
make -j$(nproc) >build.log
echo $?
```
Repeat above procedure until you get `0` exit status. After you succeed, in `build/gdb` will be present `gdb` executable. Now, before you install it inside aforementioned `$HOME/builds` directory, check whether you got `debug` or `release` version of `gdb`.
## Debug / Release
Difference between `debug` and `release` version of executable is that `debug` version retains useful information for debugger, but `release` executable is optimized by compiler and much of these information is stripped. That being said, if you want to debug your program by some debugger, you should always compile it as `debug` version.
To find out if executable is `debug` or `release` version, you may use one of the following command:
```bash
file <executable file> | grep 'with .debug_info'
echo $?
```
```bash
objdump -Wi <executable file> | grep '.debug_info' -m 1
echo $?
```
Both of these commands print `0` if it's `debug` version, and some other number if it's `release`.
In case you compiled `release` version, you should include `-g` option in `Makefile` or `configure` script and repeat build process. For more info, you can always call `./configure --help` or consult official gdb [docs](https://github.com/bminor/binutils-gdb/tree/master/gdb).

## Install GDB
After compilation finished successfully, `gdb` can be installed:
```bash
cd "$HOME/binutils-gdb/build" # set current working dir to build folder
make install                  # install compiled gdb
```
Note that we didn't use `sudo make install` because we don't need admin privilleges to acces subdir of `$HOME` folder. Another two arguments to use `--prefix` option are to prevent conflict with system `gdb` in folders like `/usr/bin`, `/usr/local/bin`, `/bin` and conflict with another user on the same computer.

## Running GDB by another GDB instance
Let  `/usr/bin/gdb` be path to system `gdb` - we will call it `gdb-release`. Let `$HOME/builds/gdb/bin/gdb` be path to just built `gdb` - we will call it `gdb-debug`.
On my system, version of `gdb-release` is `9.2` and version of `gdb-debug` is `13.0.50`.
Let's open `gdb-debug` using `gdb-release`:
```bash
/usr/bin/gdb $HOME/builds/gdb/bin/gdb
```
Using `show version` we can find out whether we are in `gdb-release` or `gdb-debug` debugger:
```bash
(gdb) show version
GNU gdb (Ubuntu 9.2-0ubuntu1~20.04.1) 9.2
Copyright (C) 2020 Free Software Foundation...
```
which means we are in `gdb-release`. Put breakpoint on function `value_as_address(struct value *value)` which can be found in source code of `gdb-debug`.
```bash
(gdb) b value_as_address
Breakpoint 1 at 0x5646c0: file ../../gdb/value.c, line 2757
```
Make sure we are still in `gdb-release` debugger:
```bash
(gdb) show version
GNU gdb (Ubuntu 9.2-0ubuntu1~20.04.1) 9.2 ...
```
Load symbols from `test` executable:
```bash
(gdb) run ~/gdb-test/test # Load symbols from 'test' executable
Starting program: /home/syrmia/bin/gdb/bin/gdb ~/gdb-vezba/test
[Thread debugging using libthread_db enabled]
[Detaching after vfork from child process 6043]
[New Thread 0x7ffff49fd700 (LWP 6044)] ...
GNU gdb (GDB) 13.0.50.20220815-git
```
Note that `gdb 13.0.50` i.e. `gdb-debug` took terminal control. That's because `run` command 'starts debugged program'. Here, since `gdb-release` opened `gdb-debug`, `gdb-debug` is `debugged program`. When we say `run [args]`, `gdb-debug` is being run. Let's see present breakpoints:
```bash
(gdb) info b
No breakpoints or watchpoints.
```
We don't see any breakpoints because breakpoint we set on `value_as_address` is set on `gdb-debug` program, not `test`. So, `gdb-release` is aware of breakpoints set on `gdb-debug` and `gdb-debug` is aware of breakpoints set on `test`, which currently doesn't have any. Let us see `test` source  code:
```bash
(gdb) list
1   #include <stdio.h>
2	
3	int main()
4	{
5		int a = 5;
6		int *pa = &a;
7		printf("'a' is at address '%p'.\n'a' has value '%d'\n", pa, *pa);
8	
9		return 0;
10	}
```
and let's put one breakpoint on the very start of our main function and run the `test`:
```bash
(gdb) b 3
Breakpoint 1 at 0x1175: file test.c, line 4
(gdb) run
Starting program: /home/syrmia/gdb-test/test 
[Detaching after vfork from child process 15624]
[Detaching after fork from child process 15625]
...
Thread 1 "gdb" hit Breakpoint 1, value_as_address (val=0x55555622e3b0) at ../../gdb/value.c:2757
(gdb) show version
GNU gdb (Ubuntu 9.2-0ubuntu1~20.04.1) 9.2 ...
```
This means we are now in `gdb-release` on line `value_as_address` which is set on `gdb-debug` executable. Before `test` arrives to its breakpoint, `value_as_address` must be called first. Let's inspect stack trace of `gdb-debug` process:
```bash
(gdb) bt
#0  value_as_address (val=0x55555622e3b0) at ../../gdb/value.c:2757
#1  0x00005555559c7cc6 in svr4_handle_solib_event () at ../../gdb/solib-svr4.c:1838
#2  0x00005555559cd7f0 in handle_solib_event () at ../../gdb/solib.c:1338
#3  0x00005555556f4865 in bpstat_stop_status (aspace=<optimized out>, 
    bp_addr=bp_addr@entry=140737353955253, thread=thread@entry=0x555556174570, ws=..., 
    stop_chain=stop_chain@entry=0x0) at ../../gdb/breakpoint.c:5558
#4  0x000055555587faac in handle_signal_stop (ecs=0x7fffffffdd30) at ../../gdb/regcache.h:344
#5  0x000055555588209c in handle_inferior_event (ecs=<optimized out>) at ../../gdb/infrun.c:5869
#6  0x00005555558831fb in fetch_inferior_event () at ../../gdb/infrun.c:4233
#7  0x0000555555bc19c6 in gdb_wait_for_event (block=block@entry=0) at ../../gdbsupport/event-loop.cc:670
#8  0x0000555555bc1c86 in gdb_wait_for_event (block=0) at ../../gdbsupport/event-loop.cc:569
#9  gdb_do_one_event () at ../../gdbsupport/event-loop.cc:210
#10 0x00005555558c8b55 in start_event_loop () at ../../gdb/main.c:411
#11 captured_command_loop () at ../../gdb/main.c:471
#12 0x00005555558ca725 in captured_main (data=<optimized out>) at ../../gdb/main.c:1329
#13 gdb_main (args=<optimized out>) at ../../gdb/main.c:1344
#14 0x000055555565ccd0 in main (argc=<optimized out>, argv=<optimized out>) at ../../gdb/gdb.c:32
```
What we knew until now is that content of pointer is address. But, do we need to perform some operations (e.g. shift) on these bytes in order to get usable address for specific architecture?
Answer is **yes**, as can be read in 'Pointers Are Not Always Adresses' chapter of [this](https://wwwcdf.pd.infn.it/localdoc/gdbint.pdf) document. That is purpose of `value_as_address` function. Continuing program execution:
```bash
(gdb) continue
Continuing.

Breakpoint 1, main () at test.c:4
4      {
```
Only now is `test's` breakpoint hit. If we continue once more time, `test` program will regularly finish it's execution. Also, we see that `gdb-debug` instance stays active:
```bash
(gdb) c
Continuing.
'a' is at address '0x7fffffffdf3c'.
'a' has value '5'
[Inferior 1 (process 16797) exited normally]
(gdb) show version
GNU gdb (GDB) 13.0.50.20220815-git
```

 We found out that even if we don't access any pointer explicitly in `test` code, `value_as_address` in being called. The same function is invoked when we want to `print` some variable from interactive debugger, as we will show in the following example:
```bash
# Run gdb-debug using gdb-release
(bash) /usr/bin/gdb $HOME/builds/gdb/bin/gdb

# Make sure we are in gdb-release     
(gdb) show version 
GNU gdb (Ubuntu 9.2-0ubuntu1~20.04.1) 9.2        

# Put breakpoint in 'gdb-debug' exe on value_as_address function
(gdb) b value_as_address          
Breakpoint 1 at 0x5646c0: file ../../gdb/value.c, line 2757

# Make sure we are still in gdb-release
(gdb) show version  
GNU gdb (Ubuntu 9.2-0ubuntu1~20.04.1) 9.2 ...

# Load symbols from 'test' executable in 'gdb-debug'
(gdb) run ~/gdb-test/test               
Starting program: /home/syrmia/bin/gdb/bin/gdb ~/gdb-vezba/test
[Thread debugging using libthread_db enabled]
[Detaching after vfork from child process 6043]
[New Thread 0x7ffff49fd700 (LWP 6044)] ...
GNU gdb (GDB) 13.0.50.20220815-git

# Display code that is about to be run
(gdb) list                          
1	#include <stdio.h>
2	
3	int main()
4	{
5		int a = 5;
6		int *pa = &a;
7		printf("'a' is at address '%p'.\n'a' has value '%d'\n", pa, *pa);
8	
9		return 0;
10	}

# Put 3 breakpoints in 'test' executable
(gdb) b 3                                    
Breakpoint 1 at 0x1175: file test.c, line 4.
(gdb) b 7
Breakpoint 2 at 0x1193: file test.c, line 7.
(gdb) b 9
Breakpoint 3 at 0x11b1: file test.c, line 9.

# Make sure breakpoints are set
(gdb) info b                                
Num     Type           Disp Enb Address            What
1       breakpoint     keep y   0x0000000000001175 in main at test.c:4
2       breakpoint     keep y   0x0000000000001193 in main at test.c:7
3       breakpoint     keep y   0x00000000000011b1 in main at test.c:9

# Start 'test'
(gdb) run                                  
Starting program: /home/syrmia/gdb-test/test 
[Detaching after vfork from child process 17597]
[Detaching after fork from child process 17598]
[Detaching after fork from child process 17599]

Thread 1 "gdb" hit Breakpoint 1, value_as_address (val=0x555556225a00) at ../../gdb/value.c:2757
2757	{

# Finish value_as_address function and continue execution until next breakpoint
(gdb) c 
Continuing.

Breakpoint 1, main () at test.c:4
4	{

# Continue to next breakpoint
(gdb) c 
Continuing.

Breakpoint 2, main () at test.c:7
7		printf("'a' is at address '%p'.\n'a' has value '%d'\n", pa, *pa); 

# Recall 'test' source code to see if 'pa' pointer is initialized
(gdb) list -5  
1	#include <stdio.h>
2	
3	int main()
4	{
5		int a = 5;
6		int *pa = &a;
7		printf("'a' is at address '%p'.\n'a' has value '%d'\n", pa, *pa);
8	
9		return 0;
10	}

# Print pointer 'pa'. Note that 'value_as_address' will be hit 3 times
(gdb) print pa  

Thread 1 "gdb" hit Breakpoint 1, value_as_address (val=0x555556225a00) at ../../gdb/value.c:2757
2757	{
(gdb) c
Continuing.

Thread 1 "gdb" hit Breakpoint 1, value_as_address (val=0x555556225a00) at ../../gdb/value.c:2757
2757	{
(gdb) c
Continuing.

Thread 1 "gdb" hit Breakpoint 1, value_as_address (val=0x5555561f6c10) at ../../gdb/value.c:2757
2757	{
(gdb) c
Continuing.

# Now 'pa' prints its content
$1 = (int *) 0x7fffffffdf3c   

# Continue 'test' execution
(gdb) c    
Continuing.
'a' is at address '0x7fffffffdf3c'.
'a' has value '5'

Breakpoint 3, main () at test.c:9
9		return 0;

# Finish 'test'
(gdb) c  
Continuing.
[Inferior 1 (process 17597) exited normally]

```

