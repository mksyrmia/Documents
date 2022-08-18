# Debagovanje GDB debagera

## Cilj
Prevesti gdb debager sa ukljucenim debag informacijama (opcija -g) i iskoristiti sistemski gdb za debagovanje prevedenog. Ukoliko sistemski debager ne postoji, ili ga preuzeti sa `https://github.com/bminor/binutils-gdb`, kompajlirati i iskoristiti kao sistemski ili ga jednostavno instalirati iz sistemskog softver centra. Dakle, sablonski predstavljeno: dbg (release) ---> dbg (debug) --> test (debug), gde se '-->' cita kao 'pokrece' i gde je `test` izvrsni program ciji je izvorni kod sacuvan u `test.c`:
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
i koji se dobija prevodjenjem `test.c` fajla:
```bash
gcc -g test.c -o test
```
## Prevodjenje GDB-a
```bash
cd "$HOME"
# Naredne dve linije treba ostaviti nezakomentarisane samo prvi put.
git clone 'git://sourceware.org/git/binutils-gdb.git'
cp -r binutils-gdb binutils-gdb-backup   
cd binutils-gdb
mkdir build && cd build
../configure --enable-tui --enable-source-highlight --with-python=python3 --prefix="$HOME/builds"
make -j$(nproc) >build.log
echo $?
```
Ako se na kraju ispise `0`, prevodjenje je proslo uspesno.
Ukoliko se ispise neki drugi broj, onda je potrebno pronaci gresku u `build.log` fajlu.
Za to se moze koristiti:
```bash
grep -C4 -i "error" build.log
``` 
Greska je najcesce nedostatak odredjenih paketa u sistemu. Nakon sto je ispravimo, treba postupak kompajliranja ponoviti:
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
Ispravljati greske i ponavljati sve dok se ne dobije `0` kao poslednji ispis.
Tada ce se u `build/gdb` folderu pojaviti `gdb` izvrsni fajl. Da bi bilo moguce debagovati ga, neophodno je da su u njemu prisutne debag informacije.

## Debug / Release
Postoje bar dva nacina da se utvrdi da li izvrsni fajl sadrzi debag podatke. Obe naredne komande ce ispisati `0` ukoliko se radi o debag verziji programa, a neki drugi broj za release verziju:
```bash
file <fajl> | grep 'with .debug_info'
echo $?
```
```bash
objdump -Wi <fajl> | grep '.debug_info' -m 1
echo $?
```

Ako debag informacije nisu prisutne, gdb se mora iznova prevesti sa ukljucenom opcijom -g.

## Instalacija GDB-a
Tek nakon sto je kompilacija prosla uspesno, gdb se moze instalirati na sistemu. Potrebno je:
```bash
cd "$HOME/binutils-gdb/build" # Pozicionirati se u build folder
make install                  # Instalirati prevedeni gdb
```
Primetimo da nismo koristili `sudo make install` jer se instalacija obavlja u nesistemskom `--prefix` folderu. Sada se u `$HOME/builds/` moze naci `gdb` debager. Takodje, strogo je preporucljivo da se koristi `--prefix /neki/podfolder/naseg/home/direktorijuma` opcija pre svega da ne bismo pregazili sistemski gdb ili instalaciju drugog korisnika sa kojim delimo isti racunar.

## Pokretanje GDB-a pomocu druge instance GDB-a
Neka je `$HOME/builds/gdb/bin/gdb` putanja do debag verzije gdb-a i neka je `/usr/bin/gdb` putanja do
sistemskog gdb-a. Dalje, neka je `gdb-release` sinonim za `sistemski gdb` i `gdb-debug` sinonim za `debag verziju gdb-a`. Na mom sistemu, `gdb-release` ima verziju 9.2, a `gdb-debug` 13.0.50.
Otvorimo `gdb-debug` pomocu `gdb-release`:
```bash
/usr/bin/gdb $HOME/bin/gdb/bin/gdb
``` 

Pomocu `show version` mozemo utvrditi da li smo u `gdb-release` ili `gdb-debug` debageru, odnosno do kod debagera komande koje zadajemo dolaze:
```bash
$ show version
GNU gdb (Ubuntu 9.2-0ubuntu1~20.04.1) 9.2
Copyright (C) 2020 Free Software Foundation...
```
sto znaci da smo u debageru `gdb-release`. Postavimo breakpoint na funkciju `value_as_address(struct value *value)` koja se nalazi u `gdb-debug`:
```bash
b value_as_address
Breakpoint 1 at 0x5646c0: file ../../gdb/value.c, line 2757.
```
Uverimo se da smo jos uvek u `gdb-release` debageru:
```bash
show version
GNU gdb (Ubuntu 9.2-0ubuntu1~20.04.1) 9.2 ...
```
Ucitajmo simbole iz fajla `test`:
```bash
run $HOME/gdb-test/test # Ucitavamo simbole iz test izvrsnog programa
Starting program: /home/syrmia/bin/gdb/bin/gdb ~/gdb-vezba/test
[Thread debugging using libthread_db enabled]
[Detaching after vfork from child process 6043]
[New Thread 0x7ffff49fd700 (LWP 6044)] ...
GNU gdb (GDB) 13.0.50.20220815-git
```
Primecujemo da je kontrolu preuzeo `gdb 13.x.y`, odnosno `gdb-debug` jer `run` komanda pokrece program koji se debaguje, a sto je u ovom slucaju `gdb-debug`. Prikazimo koje smo breakpointove postavili:
```bash
info breakpoints
No breakpoints or watchpoints.
```
To se dogodilo zbog toga sto je breakpoint na `value_as_address` postavljen na `gdb-debug` od strane `gdb-release` i njega vidi samo `gdb-release`. `gdb-debug` je svestan samo onih breakpointova koje je sâm postavio na izvrsni program koji debaguje, sto je u ovom slucaju `test`:
```bash
# Strelica se cita 'vidi breakpoint-ove postavljene na'
`gdb-release`   -->   `gdb-debug`   -->   `test`
```
Pogledajmo dokle smo sa izvrsavanjem kôda stigli i trenutni trag steka:
```bash
list
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

bt
No stack.
```
Nema stek okvira jer `test` jos uvek nije pokrenuo svoje izvrsavanje.
Pre nego sto to ucinimo, postavimo breakpoint na liniju 5 `test` programa:
```bash
b 5
Breakpoint 1 at 0x1193: file test.c, line 5.
run
Starting program: /home/syrmia/gdb-test/test 
[Detaching after vfork from child process 7077]
[Detaching after fork from child process 7078]
[Detaching after fork from child process 7079]
Thread 1 "gdb" hit Breakpoint 1, value_as_address (val=0x5555562390e0) at ../../gdb/value.c

show version
GNU gdb (Ubuntu 9.2-0ubuntu1~20.04.1) 9.2
```
Dakle, bili smo u `gdb-debug` i postavili breakpoint na `test` kako bi `test` bio zaustavljen od strane `gdb-debug` kada dodje do breakpointa. Medjutim, onda se kontrola vratila sistemskom `gdb-release` debageru.
Zasto se to dogodilo? Komandu `backtrace (skraceno bt)` cemo zadati sistemskom debageru i ona ce nam ispisati stek programa `gdb-debug`:
```bash
bt
#0  value_as_address (val=0x5555562390e0) at ../../gdb/value.c:2757
#1  0x00005555559c7cc6 in svr4_handle_solib_event () at ../../gdb/solib-svr4.c:1838
#2  0x00005555559cd7f0 in handle_solib_event () at ../../gdb/solib.c:1338
#3  0x00005555556f4865 in bpstat_stop_status (aspace=<optimized out>, 
    bp_addr=bp_addr@entry=140737353955253, thread=thread@entry=0x5555561745c0, ws=..., 
    stop_chain=stop_chain@entry=0x0) at ../../gdb/breakpoint.c:5558
#4  0x000055555587faac in handle_signal_stop (ecs=0x7fffffffdcd0) at ../../gdb/regcache.h:344
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
`gdb-debug` je pokrenuo `main` funkciju `test-a` ali je stao pre nego sto je dosao do svog breakpointa u liniji 5. To je zbog toga sto se funkcija `value_as_address` poziva svaki put kada je potrebno prevesti pokazivac u adresu. Naime, na [linku](https://wwwcdf.pd.infn.it/localdoc/gdbint.pdf) se moze naci poglavlje 'Pointers are not always addresses' koje govori o tome da se zbog razlicitog nacina zapisivanja adresa na razlicitim arhitekturama procesora koriste funkcije `gdb-a` koje te razlike nadomescuju (`value_as_address`, `value_from_pointer`, `store_typed_address`, `extract_typed_address`). 
Funkcija `value_from_address` se ocigledno poziva i pre nego sto se pristupi bilo kom eksplititno deklarisanom pokazivacu, kao sto je u nasem primeru `pa`. Ostaje pitanje adresa cega je `0x5555562390e0` koja se moze procitati iz poslednjeg `bt` poziva (linija koja pocinje sa `#0`).
