# Generic Notes #

## Linux 

### Permissions to write to Serial PORT

```
# Quick fix, change permissions 
$ sudo chmod -R 777 /dev/ttyUSB0

##  Permanent solution, add user to group tty or dialout
# list user groups
$ groups

# list available group types
$ compgen -g

# add user to group tty
$ sudo usermod -a -G tty <username>
```