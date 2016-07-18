#!/bin/bash

export PATH=/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/

# --- Test Parameters ---
test_namefile=$(ls *.name)
test_name="${test_namefile%.*}" # test network id
nwconf=$(ls *.conf) # blank test network config file
nwid="${nwconf%.*}" # test network id
file_path=/opt/results/ # test result output file path (fs shared between host and containers)
file_base="$test_name".txt # test result output file
tmp_ext=.tmp # temporary filetype used for sharing test data between containers
address_file="$file_path$test_name"_addr"$tmp_ext" # file shared between host and containers for sharing address (optional)

# --- Network Config ---
echo '*** ZeroTier SDK Test: ' "$test_name"
chown -R daemon /var/lib/zerotier-one
chgrp -R daemon /var/lib/zerotier-one
su daemon -s /bin/bash -c '/zerotier-sdk-service -d -U -p9993 >>/tmp/zerotier-sdk-service.out 2>&1'
virtip4=""
while [ -z "$virtip4" ]; do
	sleep 0.2
	virtip4=`/zerotier-cli listnetworks | grep -F $nwid | cut -d ' ' -f 9 | sed 's/,/\n/g' | grep -F '.' | cut -d / -f 1`
	dev=`/zerotier-cli listnetworks | grep -F "" | cut -d ' ' -f 8 | cut -d "_" -f 2 | sed "s/^<dev>//" | tr '\n' '\0'`
done
echo '*** Up and running at' $virtip4 ' on network: ' $nwid
echo '*** Writing address to ' "$address_file"
echo $virtip4 > "$address_file"

# --- Test section ---
echo '*** Starting application...'
sleep 0.5

export ZT_NC_NETWORK=/var/lib/zerotier-one/nc_"$dev"
export LD_PRELOAD=./libztintercept.so
/usr/bin/redis-server --port 6379
