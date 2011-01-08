#include "output.h"

OutputPlay::OutputPlay () : mpDecoder(NULL), mpStream(NULL), mpOuter(NULL), mpClock (NULL),
                            mbQuit(false), mOperationLock("OutputPlay") {
    DEBUG ("construct OutputPlay!");
}

OutputPlay::~OutputPlay () {
}

void OutputPlay::SetDecoder (WindDecoder* decoder) {
    mpDecoder = decoder;
}

void OutputPlay::SetStream (AVStream* stream) {
    mpStream = stream;
}

void OutputPlay::SetOuter (WindOuter* vout) {
    mpOuter = vout;
}

void OutputPlay::SetMasterClock (MasterClock* clock) {
    mpClock  = clock;
}

int OutputPlay::Start () {
    mbQuit = false;
    if (mpDecoder == NULL) {
        ERROR ("no decoder set!"); 
        return -1;
    }
    ThreadExec ();
    return 0;
}

int OutputPlay::Stop () {
    mbQuit = true;
    ThreadWait ();
    return 0;
}

void OutputPlay::ThreadEntry () {
    DEBUG ("empty ThreadEntry! you must do it!");
}
