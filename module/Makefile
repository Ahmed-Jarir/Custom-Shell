obj-m += mymodule.o

all:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" clean
nix:
	make -C /nix/store/hyi6y904ws53wywi0zwzdf0m657zb061-linux-6.2.7-dev/lib/modules/*/build M=$(PWD) modules
nixClean:
	make -C /nix/store/hyi6y904ws53wywi0zwzdf0m657zb061-linux-6.2.7-dev/lib/modules/*/build M=$(PWD) clean
