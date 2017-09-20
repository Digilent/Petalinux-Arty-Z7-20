#!/bin/bash


DTBO="$2"

echo
echo
echo -e "\tOverlay File ------> $DTBO"

echo


load() {
	# Check to see if the Overlay and bit files are present in /lib/firmware
	if [ -e /lib/firmware/$DTBO ]; then
		echo -e "\tFound the .dtbo file"
	else
		echo "The specified .dtbo file is not present in /lib/firmware"
		exit 1
	fi


	echo -e "\t**********************************************"
	echo -e "\t          Loading Overlay $DTBO               "
	echo -e "\t**********************************************"
	
	# Set Up For Configfs
	if [ -e /config ]; then 
		echo -e "\t/config already exists"
	else
		mkdir /config
	fi
		
	# Check to see if configfs has already been mounted at /config
	if [ -e /config/device-tree ]; then
		echo -e "\tconfigfs has already been mounted"
	else
		mount -t configfs configfs /config
	fi

	# Check to see if the overlay has already been loaded
	if [ -e /config/device-tree/overlays/$DTBO ]; then
		echo "This overlay has already been loaded"
		echo "Or it was not unloaded properly"
		echo "Please unload it using the Stop function before proceeding"
		exit 1
	else
		mkdir /config/device-tree/overlays/$DTBO
	fi


	sleep 1
	echo $DTBO > /config/device-tree/overlays/$DTBO/path
	echo -e "\tOverlay Loaded"
	echo
	
}

unload() {
	echo -e "\t**********************************************"
	echo -e "\t          Unloading Overlay $DTBO             "
	echo -e "\t**********************************************"
	
	if [ -e /config/device-tree/overlays/$DTBO ]; then
		rmdir /config/device-tree/overlays/$DTBO
	else
		echo "The specified overlay has already been unloaded"
		echo "Or was never loaded in the first place"
		exit 1
	fi
	echo
	echo -e "\tOverlay unloaded"
	echo	
}

case "$1" in
	load)
		load
		;;
	unload)
		unload
		;;
	*)	
		echo -e "\tUsage: $0 (load | unload) ---- 0verlay file"
		echo -e "\tsuch as $0 start some_overlay.dtbo"
		echo -e "\tThe .dtbo file should both be in /lib/firmware"
		exit 1
esac

exit $?

