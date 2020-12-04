
/*
 * Copyright (c) 2018 Metempsy Technology Consulting
 * All rights reserved.
 *
 * Copyright (c) 2006 INRIA (Institut National de Recherche en
 * Informatique et en Automatique  / French National Research Institute
 * for Computer Science and Applied Mathematics)
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: André Seznec, Pau Cabre, Javier Bueno
 *
 */

/*
 * Statistical corrector base class
 */

#include "cpu/pred/wormhole_predictor.hh"


 WormholePredictor::WormholePredictor(
    const WormholePredictorParams *p)
  : SimObject(p),
    confMin(p->confMin),
    confMax(p->confMax),
    HistoryVectorSize(p->HistoryVectorSize),
    SatCtrMin(p->SatCtrMin),
    SatCtrMax(p->SatCtrMax),
    SatCtrThres(p->SatCtrThres),
    SatCtrSize(p->SatCtrSize),
    whtsize(p->whtsize),
    stats_threshold(p->stats_threshold)
{
    // whTable.resize(whtsize);
    // for(int i = 0; i < whtsize; ++i){
    //     whTable[i].localhist.resize(HistoryVectorSize);
    //     whTable[i].SatCtrs.resize(SatCtrSize);
    // }
}

WormholePredictor::BranchInfo*
WormholePredictor::makeBranchInfo()
{
    return new BranchInfo();
}



unsigned int
WormholePredictor::getSatIndex(int pcIndex, BranchInfo* bi){
    unsigned int SatIndex = 0;
    SatIndex = 0;
    SatIndex |= whTable[pcIndex].localhist[bi->whLPTotal];
    SatIndex <<= 1;
    SatIndex |= whTable[pcIndex].localhist[bi->whLPTotal-1];
    SatIndex <<= 1;
    SatIndex |= whTable[pcIndex].localhist[bi->whLPTotal-2];
    SatIndex <<= 1;
    SatIndex |= whTable[pcIndex].localhist[0];
    
    SatIndex = SatIndex % SatCtrSize;
    return SatIndex;
}

bool
WormholePredictor::WhPredict(ThreadID tid, Addr branch_pc, bool cond_branch,
                   BranchInfo* bi, bool prev_pred_taken, unsigned instShiftAmt)
{

    bool pred_taken = prev_pred_taken;
    unsigned int pcIndex;

    if(cond_branch){
        if (bi->whLPTotal != 0) {
              for (pcIndex = 0; pcIndex < whTable.size(); pcIndex++) {
                  
                  if (whTable[pcIndex].PCtag == getPCtag(branch_pc,instShiftAmt)) {
                      break;
                  }
              }
              if (pcIndex < whTable.size() && bi->whLPTotal > 1) {
                    if (whTable[pcIndex].localhist.size() >= ((unsigned int)bi->whLPTotal + 1)) {

                        // generate index using 2D local history bits
                        bi->whIdx = getSatIndex(pcIndex, bi);

                        // get prediction from saturating counter
                        bi->whPred = whTable[pcIndex].SatCtrs[bi->whIdx];
                        
                        if (whTable[pcIndex].conf >= 0 && (abs((2*bi->whPred) + 1) >= SatCtrThres)) {
                            pred_taken = bi->whPred;
                            bi->usedWhPred = true;
                        }
                    }
              }
          }
    }

    return pred_taken;
}

void 
WormholePredictor::UpdateRanking(ThreadID tid, Addr branch_pc, int lsum, bool loopPredValid, 
                                 unsigned int instShiftAmt)

{
    // update the rank for entries in whTable if this branch is difficult for TAGE
    if (abs ((lsum*2) + 1) <= stats_threshold) {
        unsigned int i;
        for (i = 0; i < whTable.size(); i++) {

            // if entry already exists, move it up in the ranking
            if (whTable[i].PCtag == getPCtag(branch_pc, instShiftAmt)) {
                if (i > 0) {
                    std::swap(whTable[i-1], whTable[i]);
                }
                break;
            }
        }

        // if entry doesn't exist, push it to bottom of ranking
        if (i == whTable.size() && loopPredValid) { // Only allocate if it is inside a loop
            if (whTable.size() == whtsize) {
                whTable.pop_back();
            }

            WhEntry wormEntry;

            wormEntry.SatCtrs.resize(SatCtrSize);
            // wormEntry->localhist.resize(HistoryVectorSize);
            for (int j = 0; j < SatCtrSize; j++) {
                wormEntry.SatCtrs[j] = 0;
            }
            wormEntry.PCtag = getPCtag(branch_pc, instShiftAmt);
            wormEntry.conf = 0;
            whTable.push_back(wormEntry);
            
        }
    }
}

void 
WormholePredictor::updatehistorybits(BranchInfo* bi, int entry,  bool taken ){
    // update local history bits
    whTable[entry].localhist.insert(whTable[entry].localhist.begin(), taken);
    if (whTable[entry].localhist.size() > HistoryVectorSize) {
        whTable[entry].localhist.pop_back();
    }
}

void 
WormholePredictor::updateConf(BranchInfo* bi, int entry,  bool taken, bool tage_pred)
{
    if (bi->whPred  != tage_pred) {
        if (bi->whPred == taken) {
            whTable[entry].conf += (whTable[entry].conf < confMax) ? 1 : 0;
        } else {
            whTable[entry].conf += (whTable[entry].conf > confMin) ? -1 : 0;
        }
    }
}

void
WormholePredictor::updateSatCtrs(BranchInfo* bi, int entry,  bool taken)
{
    // update saturating counters
    if (taken) {
        whTable[entry].SatCtrs[bi->whIdx] += (whTable[entry].SatCtrs[bi->whIdx] < SatCtrMax) ? 1 : 0;
    } else {
        whTable[entry].SatCtrs[bi->whIdx] += (whTable[entry].SatCtrs[bi->whIdx] > SatCtrMin) ? -1 : 0;
    }
}

 void
 WormholePredictor::condBranchUpdate(ThreadID tid, Addr branch_pc, bool taken,
                                 bool tage_pred, BranchInfo* bi, int lsum, bool loopPredValid,
                                 unsigned instShiftAmt)

{
    UpdateRanking(tid, branch_pc,lsum, loopPredValid, instShiftAmt);
    for(int i = 0; i < whTable.size(); ++i){
        if(whTable[i].PCtag == getPCtag(branch_pc, instShiftAmt)){
            if(bi->whLPTotal > 1 && whTable[i].localhist.size() >= ((unsigned int)bi->whLPTotal + 1)){
                updateConf(bi, i,  taken, tage_pred);
                updateSatCtrs(bi, i,  taken);

            }
            updatehistorybits(bi,i,taken);
        }
    }

}


void
WormholePredictor::updateStats(bool taken, BranchInfo *bi)
{

if (bi->usedWhPred)
    {
    if (taken == bi->whPred) {
        whPredictorCorrect++;
        } else {
        whPredictorWrong++;
        }
    }

}

WormholePredictor*
WormholePredictorParams::create()
{
    return new WormholePredictor(this);
}