#!/bin/bash
# ./file peerPort storageSize #neighbours neigh0port .. [loggerIp loggerPort] < /dev/null > /dev/null 
./peer 5000 1 5002 5005 5001 < /dev/null > /dev/null & 
./peer 5001 2 5000 5005 5004 < /dev/null > /dev/null & 
./peer 5002 3 5000 5009 < /dev/null > /dev/null & 
./peer 5003 4 5004 5007 5006 < /dev/null > /dev/null & 
./peer 5004 5 5001 5003 5005 5006 5007 5011 < /dev/null > /dev/null & 
./peer 5005 6 5000 5009 5001 5004 5010 5006 < /dev/null > /dev/null & 
./peer 5006 7 5004 5007 5011 5005 5003 < /dev/null > /dev/null & 
./peer 5007 8 5003 5006 5004 < /dev/null > /dev/null & 
./peer 5009 9 5002 5005 5011 < /dev/null > /dev/null & 
./peer 5010 10 5005 5011 < /dev/null > /dev/null & 
./peer 5011 11 5010 5006 5009 5004 < /dev/null > /dev/null & 

# Estimated PDUs: 42
# Estimated Storage: 66
