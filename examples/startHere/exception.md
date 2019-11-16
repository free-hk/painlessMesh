--- Quit: Ctrl+C | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H ---
3515565] {"type":"group_message","group_id":"public","message":"Group Message : 178","source_id":673515565}
Changed connections
Num nodes: 3
Connection list: 3220211649 673514637 673516837
--> startHere: New Connection, nodeId = 673516837
--> startHere: New Connection, {
  "nodeId": 673515565,
  "subs": [
    {
      "nodeId": 3220211649
    },
    {
      "nodeId": 673514637
    },
    {
      "nodeId": 673516837
    }
  ]
}
Sending: [673515565] heartbeat - Free Memory 49288
Delay to node 673514637 is 13140 us
Delay to node 3220211649 is 29933 us
heartbeat message from : 673514637
heartbeat message from : 3220211649
heartbeat message from : 673514637
Sending: [673515565] heartbeat - Free Memory 49536
heartbeat message from : 673514637
heartbeat message from : 3220211649
dhcps: send_nak>>udp_sendto result 0
dhcps: send_offer>>udp_sendto result 0
dhcps: send_offer>>udp_sendto result 0
Sending: [673515565] heartbeat - Free Memory 37828
heartbeat message from : 673514637
Sending: [673515565] heartbeat - Free Memory 35584
heartbeat message from : 3220211649
Receive: [3220211649] msg= #### {"type":"group_message","group_id":"public","message":"Group Message : 24","source_id":3220211649}  ####
Changed connections
Num nodes: 2
Connection list: 3220211649 673514637
Sending: [673515565] heartbeat - Free Memory 33400
heartbeat message from : 673514637
Sending: [673515565] heartbeat - Free Memory 31656
Guru Meditation Error: Core  1 panic'ed (LoadProhibited). Exception was unhandled.
Core 1 register dump:
PC      : 0x4000127a  PS      : 0x00060930  A0      : 0x800d48fd  A1      : 0x3ffcea10  
A2      : 0x3f401a3c  A3      : 0x00000000  A4      : 0x0000000a  A5      : 0x0000ff00  
A6      : 0x00ff0000  A7      : 0x40404040  A8      : 0x00000068  A9      : 0x3ffce9e0  
A10     : 0x00000000  A11     : 0x3f4039e1  A12     : 0x00000000  A13     : 0x00000001  
A14     : 0x00000ea4  A15     : 0x00000ea4  SAR     : 0x0000000a  EXCCAUSE: 0x0000001c  
EXCVADDR: 0x00000000  LBEG    : 0x4000c46c  LEND    : 0x4000c477  LCOUNT  : 0x00000000  

Backtrace: 0x4000127a:0x3ffcea10 0x400d48fa:0x3ffcea20 0x401d45bd:0x3ffcea80 0x400d28c5:0x3ffceaa0 0x400d6eae:0x3ffcead0 0x400d4f46:0x3ffceb60 0x400e4613:0x3ffcec20 0x400e480b:0x3ffcece0 0x400e592e:0x3ffced60 0x400e5a47:0x3ffced80 0x400d69fa:0x3ffceda0 0x400e7785:0x3ffcedc0 0x4008ea55:0x3ffcede0

Rebooting...
ets Jun  8 2016 00:22:57

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0018,len:4
load:0x3fff001c,len:1044
load:0x40078000,len:8896
load:0x40080400,len:5828
entry 0x400806ac

=======
ERROR: routePackage(): parsing failed. err=3, total_length=118, data={"type":8,"dest":0,"from":3220211649,"msg":"{\"type\":\"heartbeat\",\"free_memory\":\"44000\",\"nodeId\":3220211649}"}<--
ERROR: routePackage(): parsing failed. err=3, total_length=65, data={"nodeId":3220211649,"type":6,"dest":673515565,"from":3220211649}<--
Sending: [673515565] heartbeat - Free Memory 49984
ERROR: routePackage(): parsing failed. err=3, total_length=107, data={"type":3,"dest":673515565,"from":3220211649,"msg":{"type":2,"t0":392418325,"t1":392469175,"t2":392469844}}<--
Sending: [673515565] heartbeat - Free Memory 46552
Sending: [673515565] heartbeat - Free Memory 45852
ERROR: routePackage(): parsing failed. err=3, total_length=118, data={"type":8,"dest":0,"from":3220211649,"msg":"{\"type\":\"heartbeat\",\"free_memory\":\"44640\",\"nodeId\":3220211649}"}<--
ERROR: routePackage(): parsing failed. err=3, total_length=77, data={"type":3,"dest":673515565,"from":3220211649,"msg":{"type":1,"t0":396758537}}<--
Changed connections
Num nodes: 1
Connection list: 3220211649