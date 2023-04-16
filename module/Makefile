obj-m += mymodule.o

all:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" clean
nix:
	make -C /nix/store/14mdx1mbwzf62w26c4i75gj9rvrzh0wi-linux-6.2.10-dev/lib/modules/*/build M=$(PWD) modules
nixClean:
	make -C /nix/store/14mdx1mbwzf62w26c4i75gj9rvrzh0wi-linux-6.2.10-dev/lib/modules/*/build M=$(PWD) clean
