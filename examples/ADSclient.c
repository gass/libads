/*
    libads is an implementation of the Beckhoff's ADS protocol.
    
    (C) Thomas Hergenhahn (thomas.hergenhahn@web.de) 2003, 2005.

    libads is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libads is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    Lesser GNU General Public License for more details.

    You should have received a copy of the Lesser GNU General Public License
    along with libads.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>

#include "AdsDEF.h"
#include "ads.h"
#include "ads_connect.h"

#include <sys/time.h>

#define ThisModule "ADSclient : "

#define debug 10

#define ADSPtEmpty -2
#define ADSPtMPIAck -3
#define ADSPtUnknownMPIFunc -4
#define ADSPtUnknownPDUFunc -5
#define ADSPtReadResponse 1
#define ADSPtWriteResponse 2

void dump(char *name, void *v, int len)
{
        unsigned char *b = (unsigned char *) v;
        int j;
        printf("%s: ", name);
        if (len > MAXDATALEN)
                len = MAXDATALEN;       // this will avoid to dump zillions of chars
        for (j = 0; j < len; j++) {
                printf("%02X,", b[j]);
        }
        printf("\n");
};

void readIndexGroup(ADSConnection * dc, int igr, int off)
{
	int res;
	printf("Trying to read from index group 0x%04x, offset 0x%01x\n", igr, off);
	res = ADSreadBytes(dc, igr, off, 20, NULL, NULL);
	if (res == 0) {
		printf("Dumping:\n");
		dump("data", dc->dataPointer, dc->AnswLen);
	}
	else
		printf(" Error %s ", ADSerrorText(res));
	printf("\n");
}

int main(int argc, char **argv)
{
//    ADSDebug=ADSDebugAll;

//    ADSDebug=ADSDebugAnalyze;
	ADSConnection *dc;
	struct timeval;
	int nErr;
	AdsVersion Version;
	PAdsVersion pVersion = &Version;
	char pDevName[16];
	unsigned short ADSstate, devState;
	AmsAddr Addr, MeAddr;
	PAmsAddr pAddr = &Addr, pMeAddr = &MeAddr;
	char addr[20];

	if (argc < 3) {
		printf("Usage: ADSclient host port \n");
		printf("Example: ADSclient 192.168.17.110 801\n");
		return -1;
	}
	/* set the local and remote netId */
	snprintf(addr, 20, "%s.1.1", argv[1]);
	_ADSparseNetID(addr, &pAddr->netId);
	Addr.port = atol(argv[2]);
	ADSGetLocalAMSId(&pMeAddr->netId);
	/* connect */
	dc = ADSsocketConnect(pAddr, &nErr);
	if (dc == NULL) {
		fprintf(stderr, "Could not connect to ADS server\n");
		return 1;
	}

	/* start communicating 
	 * Read Device info
	 */
	ads_debug(ADSDebug,"device info:\n");
	ADSreadDeviceInfo(dc, pDevName, pVersion);
	printf("Device Name: %s\n", pDevName);
	printf("Device Version: %d.%d.%d\n", pVersion->version, pVersion->revision, pVersion->build);
	/*
	 * read state
	 */
	ads_debug(ADSDebug,"read state:\n");
	ADSreadState(dc, &ADSstate, &devState);
	printf("ADS State: %d Device State: %d\n", ADSstate, devState);
	//ads_debug(ADSDebug,"write control:");
	ADSwriteControl(dc, 4, 0, NULL, 0);

	readIndexGroup(dc, 0x4020,0);
	readIndexGroup(dc, 0x4020,2);
//    readIndexGroup(dc, 0x4025);
	readIndexGroup(dc, 0x4030,0);
	readIndexGroup(dc, 0x4031,0);
	readIndexGroup(dc, 0x4040,0);

	readIndexGroup(dc, 0xf003,0);
	readIndexGroup(dc, 0xf004,0);
	readIndexGroup(dc, 0xf020,0);
	readIndexGroup(dc, 0xf021,0);
	readIndexGroup(dc, 0xf030,0);
	readIndexGroup(dc, 0xf031,0);
	readIndexGroup(dc, 0xf060,0);
//    ADSaddDeviceNotification(dc, igr, 0, 10, ADS_TRANS_NOTRANS, 100000, 120000);
	/* something is missing here TODO 
	ADSaddDeviceNotification(dc, 0x4020, 0, 10, ADS_TRANS_CLIENTCYCLE,
				 100000, 120000);
	*/
//    ADSaddDeviceNotification(dc, igr, 0, 10, ADS_TRANS_CLIENT1REQ, 100000, 120000);
//    ADSaddDeviceNotification(dc, igr, 0, 10, ADS_TRANS_SERVERCYCLE, 100000, 120000);
//    ADSaddDeviceNotification(dc, igr, 0, 10, ADS_TRANS_SERVERONCHA, 100000, 120000);
//    ADSaddDeviceNotification(dc, 0x801f, 0, 10, 6, 100000, 120000);
//	ADSaddDeviceNotification(dc, 0x4020, 0, 10, ADSTRANS_SERVERONCHA,
//				 100000, 120000);
/*    
    k=0x4000;
    for (i=k; i<k+60; i++)
	ADSreadBytes(dc,i,0,100,NULL);
//    double usec = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec)*1e-6;
*/
	ADSsocketDisconnect(dc);
	ADSFreeConnection(dc);
	return 0;
}

