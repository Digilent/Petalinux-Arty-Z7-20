#!/bin/bash

BIN="$1"
echo
echo
echo -e "\tFPGA Bit file ------> $BIN"
echo


if [ -e /lib/firmware/$BIN ]; then
    echo -e "\tFound the FPGA bitstream file"
else
    echo "The specified bitstream file is not present in /lib/firmware"
    exit 1
fi

echo -e "\t**********************************************"
echo -e "\t          Loading Bitrsteam $BIN              "
echo -e "\t**********************************************"

cat /lib/firmware/$BIN > /dev/xdevcfg
echo -e "\tBitstream Loaded"
echo

exit $?