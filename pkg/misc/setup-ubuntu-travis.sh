#!/bin/bash

set -e
cd "$(dirname "$0")"

[[ "$1" == "" ]] && echo "Usage: $0 <ssh login>" && exit 1

# get list of required apt-get packages
lc=
pkgs=()
while IFS='' read -r line || [[ -n "$line" ]]; do
	if [[ ! "$line" =~ ^[\ \t]*# && "$line" =~ ^.*apt-get\ install.*$ || "$lc" == "1" ]]; then
		lc=; [[ "$line" =~ ^.*\\$ ]] && lc=1
		line=${line##*apt-get install}
		line=${line%\\}
		for p in $line; do [[ ! $p =~ ^- ]] && pkgs+=($p) ; done
	fi
done < ./build-deps.sh

# setup ssh login
ssh -t "$1" "mkdir -p ~/.ssh ; chmod 700 ~/.ssh ; echo '$(cat ~/.ssh/id_rsa.pub)' >~/.ssh/authorized_keys"

cat >temp.sh <<EOF
set -e

# add user to sudo
sudo bash -c 'echo -e "\n'\$USER' ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers'

# setup color prompt
sed -E -i \\
	-e 's/^#force_color_prompt=yes/force_color_prompt=yes/' \\
	-e 's/(PS1=.*01);32m/\1;33m/' \\
	\$HOME/.bashrc

# add cdrom to apt
#[ "$(grep /cdrom /etc/fstab)" == "" ] && sudo bash -c 'echo -e "\n/dev/cdrom /cdrom auto ro,user,noauto,exec,utf8 0 0" >>/etc/fstab
#sudo mkdir -p /cdrom
#sudo ln -fs /cdrom /media/cdrom
#(mount /cdrom || exit 0)
#/usr/bin/apt-cdrom add --no-auto-detect -d /cdrom'

# add apt mount
[ "$(grep /apt /etc/fstab)" == "" ] && sudo bash -c 'echo -e "\napt /var/cache/apt/archives 9p trans=virtio,version=9p2000.L,posixacl,msize=104857600 0 0" >>/etc/fstab
mount /var/cache/apt/archives'

# update apt
sudo apt-get update
sudo apt-get dist-upgrade -y
sudo apt-get clean -y
sudo apt-get install -yd ${pkgs[@]}

# delete temp script
rm ~/temp.sh
EOF

scp temp.sh build-deps.sh "$1:" 
rm temp.sh
ssh -t "$1" "bash ~/temp.sh"
