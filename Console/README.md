# COMET Servants, May 2025

## Background
The Servants project builds on the C# Servants created in 2020.  This project built on the AWS Cloud computing (by Joshua King) and this was built on the Galaxy Software (C++) created by John Revil.  So this is the 4th version.

## Scope

The major scope changes in this version include keeping all files locally, not using the COMET Hub, rather build one on this machine.

The idea is to have both local processing and remote processing (Hub) in this software.

## Distributed processing

While one machine must act as a "Hub" to cordinate the processing, several machines may act as processors of COMET Cases or "jobs".  One machine could also do both tasks.

## Servant HUB

The hub will get information from machines when a case is ready for processing and it will decide where it should be run.

The Hub will be contacted by other Servants to let it know how many jobs can be processed (related to available cores).


## Setting up a New Development  Machine
Mac
```bash
brew install doxygen
brew install graphviz
```


# Setting up a new Windows
- This contains a very useful link
`https://drogonframework.github.io/drogon-docs/#/ENG/ENG-02-Installation?id=windows`
- It seems using vcpkg is a popular consistent way to install Drogon on Windows/Mac/Linux.
put it somewhere like `C:\vcpkg` and then run the following commands in a terminal:
```bash
git clone https://github.com/microsoft/vcpkg
cd vcpkg
./bootstrap-vcpkg.bat
git pull
```
- Add this folder to the environmemnt variables PATH

- Check it works with `vcpkg list` or `vcpkg --version' in a new shell

- Using Administrator PowerShell
```Powershell
vcpkg install drogon:x64-windows
vcpkg list
````
- Check if others are needed
```Powershell
vcpkg install curl:x64-windows
vcpkg install sqlite3:x64-windows



vcpkg install jsoncpp:x64-windows zlib:x64-windows openssl:x64-windows sqlite3:x64-windows libpq:x64-windows libpqxx:x64-windows 

vcpkg install jsoncpp:x64-windows zlib:x64-windows openssl:x64-windows sqlite3:x64-windows libpq:x64-windows libpqxx:x64-windows drogon[core,ctl,sqlite3,postgres,orm]:x64-windows
```
To list what is there
```Powershell
vcpkg list
```

# Setting up on MacOS
```angular2html
brew upgrade
brew install jsoncpp
brew install ossp-uuid
yum install zlib-devel
brew install openssl
brew install sqlite3

```

# Setting up two machines

My Laptop is called "one.local", the server will be a Windows machine called "two"

## On "one.local"

Make sure each one can get to the other one

- If a Mac

> Edit the hosts file as administrator

> `sudo nano /etc/hosts`

> Add the following if `192.168.1.152` is the ip address of `two`

> `192.168.1.152 two`

- If Windows

> As an administrator, edit the Windows host file

> `notepad C:\Windows\System32\drivers\etc\hosts`

> Add the row if `192.168.1.48` is the ip address of `one.local`

> `192.168.1.48 one.local`


## On a mac connect to the server

Make a mounting point

`mkdir -p ~/NetworkShares`

Add the following line to link to the mount  folder:
`ln -s /Volumes/COMET_Data ~/NetworkShares`







