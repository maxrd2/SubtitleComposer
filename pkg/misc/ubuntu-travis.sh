#!/bin/bash

set -e
trap '' 2  # Disable Ctrl+C
cd "$(dirname "$0")"

img_cdrom="$HOME/KVM/iso/ubuntu-14.04.5-server-amd64.iso"
img_disk="./ubuntu-travis.qcow2"
img_size="10G"

[ ! -f "$img_cdrom" ] && echo "ERROR: '$img_cdrom' doesn't exist." && exit 1

par=(
	-serial none
	-parallel none
	-enable-kvm
	-name "Travis"
	-cpu host,kvm=off
	-smp sockets=1,cores=4,threads=1
	-m 4096
	-device intel-hda,id=sound0,bus=pci.0
	-device hda-duplex
	-rtc base=localtime
	-net nic,macaddr=00:1d:cb:f0:27:3f,model=virtio,id=net0
	-net bridge,id=bridge0,br=kvm0
	-device ide-cd,drive=cd1,id=cdrom1,unit=0,bus=ide.0
	-drive if=none,id=cd1,media=cdrom,file="$img_cdrom"
	-drive if=virtio,id=disk0,format=qcow2,media=disk,file="$img_disk"
	-boot once=c,menu=off # boot from hdd
	-vga qxl
)
# mount -t 9p apt /mnt/apt/ -o trans=virtio,version=9p2000.L,posixacl,msize=104857600,cache=loose
[ -d "apt/trusty" ]	&& par+=(-virtfs local,mount_tag=apt,path="$PWD/apt/trusty",security_model=none)

img_reset() {
	qemu-img create -f qcow2 -b "${img_disk%.*}-clean.qcow2" "$img_disk"
}
img_compress() {
	if [ -d mnt ]; then echo "Directory 'mnt' already exists." ; exit 1; fi
	sudo modprobe nbd max_part=63
	sudo qemu-nbd -c /dev/nbd0 "${img_disk%.*}-clean.qcow2"
	mkdir mnt
	sudo mount /dev/nbd0p1 mnt/
	sudo fstrim -v mnt/
	sudo umount mnt/
	rmdir mnt
	echo -n "        zeroing unused space "
	sudo zerofree -v /dev/nbd0p1
	sudo qemu-nbd -d /dev/nbd0
	sudo modprobe -r nbd
	mv "${img_disk%.*}-clean.qcow2" "${img_disk%.*}.old.qcow2"
	echo "compressing ${img_disk%.*}-clean.qcow2"
	qemu-img convert -O qcow2 "${img_disk%.*}.old.qcow2" "${img_disk%.*}-clean.qcow2"
	rm "${img_disk%.*}.old.qcow2"
}
img_mount() {
	sudo modprobe nbd max_part=63
	sudo qemu-nbd -c /dev/nbd0 "$img_disk"
	mkdir mnt
	sudo mount /dev/nbd0p1 mnt/
	echo "exit this shell to unmount image"
	(cd mnt && bash -login)
	sudo umount mnt/
	rmdir mnt
	sudo qemu-nbd -d /dev/nbd0
	sudo modprobe -r nbd
}
img_merge() {
	echo "creating merged image..."
	qemu-img convert -O qcow2 "$img_disk" "${img_disk%.*}.new.qcow2"
	mv "${img_disk%.*}.new.qcow2" "${img_disk%.*}-clean.qcow2"
	img_reset
}

[ ! -f "${img_disk%.*}-clean.qcow2" ] && ( echo "creating $img_size base disk image..." ; qemu-img create -f qcow2 "${img_disk%.*}-clean.qcow2" $img_size ; img_reset )
[ "$1" == "reset" ] && img_reset
[ "$1" == "compress" ] && img_compress
[ "$1" == "mount" ] && img_mount
[ "$1" == "merge" ] && img_merge
[ "$1" == "help" ] && echo "$0 [reset|compress|mount|merge|video|help]"
[ "$1" == "video" ] && ( par+=(-monitor stdio) ; /usr/bin/qemu-system-x86_64 "${par[@]}" "${@:2}" )
[ "$1" == "" ] && (
	par+=(-monitor none -nographic) 
	(
		echo '*** virtual machine is booting in background ***'
		/usr/bin/qemu-system-x86_64 "${par[@]}" "$@" &>/dev/null
		echo -e '\n*** virtual machine terminated ***'
	) &
)

