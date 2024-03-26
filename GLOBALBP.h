#include <iostream>
#include <fstream>
#include "pin.H"

using namespace std;

class GLOBALBP
{
public:
    GLOBALBP();
    UINT64 References() { return _references; };
    UINT64 Predicts() { return _predicts; };
    UINT64 Icount() { return _icount; };
    UINT64 Mispredicts() { return _references - _predicts; };
    VOID Activate(ofstream *outfile);

private:
    enum 
    {
        TABLESIZE = 4096, // Size of the prediction table
        HIST_LENGTH = 16  // Length of the global history
    };

    static VOID Instruction(INS ins, VOID *v);
    static VOID CondBranch(VOID *v, BOOL taken);
    static VOID Count(VOID *v);
    static VOID PrintStats(int n, VOID *v);
    static BOOL Predict(UINT16 globalHistory);
    static VOID Update(UINT16 &globalHistory, BOOL outcome);
    ofstream *_outfile;

    static INT8 _predTable[TABLESIZE]; // Global prediction table
    UINT16 _globalHistory;      // Global history register
    UINT64 _references, _predicts, _icount;
};

GLOBALBP::GLOBALBP()
{
  _globalHistory =  0;
      _references = 0;
    _predicts = 0;
    _icount = 0;
    _outfile = NULL;
    memset(_predTable, 0, sizeof(_predTable));
}

VOID GLOBALBP::Activate(ofstream *outfile)
{
    _outfile = outfile;
    INS_AddInstrumentFunction(Instruction, this);
    PIN_AddFiniFunction(PrintStats, this);
}

BOOL GLOBALBP::Predict(UINT16 globalHistory)
{
    UINT16 index = globalHistory % TABLESIZE;
    return _predTable[index] >= 2;
}
VOID GLOBALBP::Update(UINT16 &globalHistory, BOOL outcome)
{
    UINT16 index = globalHistory % TABLESIZE;
    if (outcome)
    {
        if (_predTable[index] < 3) _predTable[index]++;
    }
    else
    {
        if (_predTable[index] > 0) _predTable[index]--;
    }
    globalHistory = ((globalHistory << 1) | outcome) & ((1 << HIST_LENGTH) - 1);
}


VOID GLOBALBP::Instruction(INS ins, VOID *v)
{
    GLOBALBP *gbp = static_cast<GLOBALBP*>(v);
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(Count), IARG_PTR, gbp, IARG_END);
    if (INS_IsBranch(ins) && INS_HasFallThrough(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(CondBranch), IARG_PTR, gbp, IARG_BRANCH_TAKEN, IARG_END);
    }
}

VOID GLOBALBP::CondBranch(VOID *v, BOOL taken)
{
    GLOBALBP *gbp = static_cast<GLOBALBP*>(v);
    gbp->_references++;
    if ((taken && gbp->Predict(gbp->_globalHistory)) || (!taken && !gbp->Predict(gbp->_globalHistory)))
    {
        gbp->_predicts++;
    }
    gbp->Update(gbp->_globalHistory, taken);
}


VOID GLOBALBP::Count(VOID *v)
{
    GLOBALBP *gbp = reinterpret_cast<GLOBALBP*>(v);
    gbp->_icount++;
}


VOID GLOBALBP::PrintStats(int n, VOID *v)
{
    GLOBALBP *gbp  = reinterpret_cast<GLOBALBP *>(v);

    *(gbp->_outfile) << endl;
    *(gbp->_outfile) << "Icount: " << dec << gbp->Icount() << " Mispredicts: " << dec << gbp->Mispredicts() << " MPKI: " << fixed << ((double)gbp->Mispredicts()*1000)/(double)gbp->Icount() << endl;
    *(gbp->_outfile) << "References: " << dec << gbp->References() << " Predicts: " << dec << gbp->Predicts() << endl;
}
