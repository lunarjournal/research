# EEE4022F

Author: [`Dylan Müller`](https://www.linkedin.com/in/dylanmuller/)
> University of Cape Town (UCT) [`uct.ac.za`](https://uct.ac.za)

```
ECE Undegraduate Research Project
Project Title: Self-Encrypting USB Password Manager
```

The goal of my final year thesis project was to design a self-encrypting `USB` based hardware password manager. Single sign on (`SSO`) capability was also another requirement for the project. `SSO` capability allows an end user to log into any of their online accounts through a single, unified interface. Credentials were required to be securely stored in an encrypted form and then subsequently decrypted during an `SSO` login session. 

The solution to this problem involved utilizing two `ATMega328P` microcontrollers. The first microcontroller, termed the auxiliary microcontroller implemented a `RSA-1024` cryptographic module as well as a custom `EEPROM` based filesystem to store encrypted credentials using the `Arduino` software stack and bootloader. Next a `USB` `CDC` `ACM` device class was implemented in firmware using the `V-USB` driver stack and flashed onto another `ATMega328P` microcontroller, termed the `USB` microcontroller. 

![High Level Overview](https://raw.githubusercontent.com/lunarjournal/research/main/images/HL.png)

The auxiliary and `USB` microcontrollers `UART` transceivers were then connected together to enable serial communications between both microcontrollers. On `Linux`, `CDC` `ACM` devices appear as virtual serial ports (`/dev/ttyACM*`) and this finally allowed for virtual serial communications between the host `PC` and the auxilliary microcontroller via the `USB` microcontroller.

A software client was then developed and consisted of a chrome browser extension to manage credentials through a UI interface as well as a native host (using google chromes native messsaging `API`) to enable serial communication between the chrome browser extension and the auxiliary `MCU`. The native host was also responsible for decrypting credentials as well as generating `RSA` keys and made use of `OpenSSL's` `C` `API` for these tasks. 

![Browser Extension](https://raw.githubusercontent.com/lunarjournal/research/main/images/DE.png)

This repository consists of three sub branches (folders):
* `firmware` - `auxiliary and USB driver MCU firmware`.
* `software` - `browser extension and native host source`.
* `report` - `latex source code of report`.

Files:
<br/>
* `report.pdf` - `compiled PDF version of the project report`.
* `topic.pdf` - `original topic PDF`.
* `poster.pdf` - `project poster`.
