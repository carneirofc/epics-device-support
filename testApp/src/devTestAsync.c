// stdlib
#include <stdint.h>
#include <stdlib.h>

// Unix
#include <unistd.h>

// EPICS
#include "aiRecord.h"
#include "callback.h"
#include "dbAccessDefs.h"
#include "dbCommon.h"
#include "dbLock.h"
#include "dbScan.h"
#include "devSup.h"
#include "epicsExport.h"
#include "errlog.h"
#include "link.h"
#include "recGbl.h"
#include "recSup.h"

#define RAND_MAX 50000

static void devTestAsync_myCallback(CALLBACK *pcallback){
    dbCommon *precord;
    rset *prset;
    // Get record from callback
    callbackGetUser(precord, pcallback);
    //errlogSevPrintf(errlogInfo, "Init callback processing: %s", precord->name);

    // RSET ptr
    prset = precord->rset;

    // Lock
    //errlogSevPrintf(errlogInfo, "dbScanLock: %s", precord->name);
    dbScanLock(precord);

    /* work work */
    int i = 0;
    for(i = 0; i < rand(); i++);{
    }
    ((aiRecord *)precord)->val = i;

    // Finish async processing
    (*prset->process)(precord);

    // Unlock
    //errlogSevPrintf(errlogInfo, "End callback processing:  %s %f", precord->name, ((aiRecord *)precord)->val);
    dbScanUnlock(precord);
}

static long devTestAsync_initRecord(aiRecord *pai){
    CALLBACK *pcallback;
    switch (pai->inp.type){
    case INST_IO:
        pcallback = (CALLBACK *)(calloc(1, sizeof(CALLBACK)));
        callbackSetCallback(devTestAsync_myCallback, pcallback);
        callbackSetUser(pai, pcallback);
        callbackSetPriority(pai->prio, pcallback);
        pai->dpvt = (void *)pcallback;
        break;
    default:
        recGblRecordError(S_db_badField, (void *)pai, "devAiTestAsyn (init_record) Illegal INP field");
        return S_db_badField;
    }

    /* Make sure record processing routine does not perform any conversion*/ 
    pai->linr = 0; 
    return 0;
}

static long devTestAsync_readAi(aiRecord *pai){
    CALLBACK *pcallback = (CALLBACK *)pai->dpvt;

    if(pai->pact) {
        pai->udf = FALSE;
        errlogSevPrintf(errlogInfo, "Completed asynchronous processing: %s\n", pai->name); 
        return 2; /* don't convert */
    }

    // Request callback processing...
    errlogSevPrintf(errlogInfo, "Starting asynchronous processing: %s\n", pai->name);
    pai->pact = TRUE;
    callbackRequest(pcallback);
    return 0;
}

/** Create the dset for devAiTestAsyn */ 
struct devTestAsync_devSup {
    long number;
    DEVSUPFUN report;
    DEVSUPFUN init;
    DEVSUPFUN init_record;
    DEVSUPFUN get_ioint_info;
    DEVSUPFUN read_write;
    DEVSUPFUN linconv;
};

static struct devTestAsync_devSup devTestAsync_devAiTestAsyn = { 
    6L,                      // number
    NULL,                    // report
    NULL,                    // init
    devTestAsync_initRecord, // init_record
    NULL,                    // get_ioint_info
    devTestAsync_readAi,     // read_write
    NULL                     // linconv
}; 
epicsExportAddress(dset, devTestAsync_devAiTestAsyn);
