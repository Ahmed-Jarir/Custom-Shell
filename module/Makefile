obj-m += mymodule.o

all:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" clean
nix:
	make -C $(nix-build -E '(import <nixpkgs> {}).linux.dev' --no-out-link)/lib/modules/*/build M=$(pwd) modules
