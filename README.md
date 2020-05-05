# The IP lookup tool for CIDR & Country with dynamic input.

## Information.

This toolkit has been writen by C/C++ programing.
* The processing core has been writen by C++ STL
* The wrapper is TCP Socket architecture, which has been writen by C standard Network lib.

Toolkit has 2 component:
* Server side: The program with Socket Server role. Listen incoming connections from socket clients.
* Client side: The program with Socket Client role. Send request command to Socket Server to impact with the Bloom filter at Server side.

Runtime:
* Server side will run forever.
* Client side will run only one time per request.

Data Structures and Algorithm
* Bloom filter: quickly verify the existing status of 1 element in 1 set.
* Hash table (ordered): extract CIDR and Country name from stable IP Address (bit string) after match with netmask.

Note: We can change the size of Bloom filter. But before that, we need analyze own demand of database to guarantee **false positive** rate of Bloom filter!

User can impact to Server side with commands of Client side (as same as Docker client)

## Build.
Add option ```-std=c++11``` or ```-std=c++14``` to ```g++``` compiler.

#### Client side
```bash
$ g++ cidr_client.cpp -std=c++11 -o cidr_client
```

#### Server side
```bash
$ g++ cidr_server.cpp -std=c++11 -o cidr_server
```

Note: ```sudo``` may be necessary.

## Run - Example

### Server side
```bash
$ ./cidr_server
```

or run as a daemon (and kill with pid)
```bash
$ ./cidr_server &
```

### Client side example
#### Show usage
```bash
$ ./cidr_client help
|*************************************************************|
|********************* Bloom Filter CIDR *********************|
|*************************************************************|
| Show Bloom filter info: $ ./cidr_client info                |
| Load new input file: $ ./cidr_client load <path> <location> |
| Add new value: $ ./cidr_client add <cidr> <location>        |
| Load check file: $ ./cidr_client check_file <path>          |
| Check new value: $ ./cidr_client check <ip_addr>            |
| Reset Bloom filter: $ ./cidr_client reset                   |
|*************************************************************|
```
#### Show current info
```bash
$ ./cidr_client info
Bloom filter of CIDR information:
Number of hash functions: 4
Size of the Bloom filter: 57707801
Maximum storage capacity CIDR value: 9999999
False positive probability: 6%
Current number of CIDR values: 519070
```
Above infor is state after load all 20 country CIDR file list. 
#### Load lists of CIDR belong country
```bash
$ ./cidr_client load cidr-Russia.txt Russia
Complete load the new input file "cidr-Russia.txt" into the Bloom filter
```
#### Check a IP address
```bash
$ ./cidr_client check 208.178.8.202
on 208.178.8.202/255.255.255.255 of Australia
```
Another IP address:
```bash
$ ./cidr_client check 172.15.2.3
on 172.0.0.0/255.240.0.0 of USA
```
And another local IP address:
```bash
$ ./cidr_client check 172.22.2.3
not on
```

#### Reset
```bash
$ ./cidr_client reset
The Bloom filter has been reset!
```