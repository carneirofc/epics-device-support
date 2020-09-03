#!../../bin/linux-x86_64/test

< envPaths

cd "${TOP}"

dbLoadDatabase "dbd/test.dbd"
test_registerRecordDeviceDriver pdbbase

dbLoadRecords "db/devTest.db", "device=D1"

cd "${TOP}/iocBoot/${IOC}"
iocInit

