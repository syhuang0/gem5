
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
    SatCtrWidth(p->SatCtrWidth),
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
    // std::vector<int> offest = {bi->whLPTotal,bi->whLPTotal-1, bi->whLPTotal-2, 0 };
    std::vector<int> lengths = {bi->whLPTotal,bi->whLPTotal-1, bi->whLPTotal-2};
    for(int i: lengths){
        SatIndex |= whTable[pcIndex].localhist[i];
        SatIndex <<=1;
    }

    SatIndex |= whTable[pcIndex].localhist[0];
 
    SatIndex = SatIndex % SatCtrSize;
    return SatIndex;
}

int 
WormholePredictor::pctagsearch(Addr branch_pc, unsigned int instShiftAmt)
{
    for (unsigned int pcIndex = 0; pcIndex < whTable.size(); pcIndex++) {
                  
        if (whTable[pcIndex].PCtag == getPCtag(branch_pc,instShiftAmt) ) {
            return pcIndex;
        }
    }
    return -1; 
}
bool
WormholePredictor::makePrediction(int pcIndex, BranchInfo* bi)
{
    return whTable[pcIndex].conf >= 0 && (abs((2*whTable[pcIndex].SatCtrs[bi->whIdx]) + 1) >= SatCtrThres); 
}

bool 
WormholePredictor::recordingloophistory(int pcIndex, BranchInfo* bi)
{
    return whTable[pcIndex].localhist.size() <= ((unsigned int)bi->whLPTotal);
}

bool
WormholePredictor::WhPredict(ThreadID tid, Addr branch_pc, bool cond_branch,
                   BranchInfo* bi, bool prev_pred_taken, unsigned int instShiftAmt)
{

    bool pred_taken = prev_pred_taken;
    int pcIndex;

    
    if(cond_branch){

        if (bi->whLPTotal >= 7 && ((pcIndex = pctagsearch(branch_pc, instShiftAmt)) >=0) && !recordingloophistory(pcIndex, bi)) {
            // std::cerr << "\nWormHole Prediction\n";
            // std::cerr << "PCtag: "<< getPCtag(branch_pc,instShiftAmt) << " PC: " << branch_pc << " Branch instance "<< bi << std::endl;
            // std::cerr << "bi->whLPTotal = "<<bi->whLPTotal << std::endl;
                // generate index using 2D local history bits
                // std::cerr << "\nWormHole Prediction\n";
            // std::cerr << "PCtag: "<< getPCtag(branch_pc,instShiftAmt) << " PC: " << branch_pc << " Branch instance "<< bi << std::endl;
            // std::cerr << "bi->whLPTotal = "<<bi->whLPTotal << std::endl;
                bi->whIdx = getSatIndex(pcIndex, bi);
                // std::cerr << "satIndex:" << bi->whIdx << " satVal: " << whTable[pcIndex].SatCtrs[bi->whIdx] << " conf: " << whTable[pcIndex].conf<< std::endl;
                // get prediction from saturating counter
                bi->whPred = (whTable[pcIndex].SatCtrs[bi->whIdx] >=0);
                
                if (makePrediction(pcIndex, bi)) {
                    
                    // std::cerr << "wh prediction: " <<  bi->whPred << std::endl;
                    bi->whPrevPred = prev_pred_taken;
                    pred_taken = bi->whPred;
                    bi->usedWhPred = true;

                        if(prev_pred_taken != bi->whPred){
                        //  std::cerr << "\nWormHole Prediction\n";
                        //  std::cerr << "PCtag: "<< getPCtag(branch_pc,instShiftAmt) << " PC: " << branch_pc << " Branch instance: "<< bi << std::endl;
                        //  std::cerr << "bi->whLPTotal = "<<bi->whLPTotal << std::endl;
                        //  std::cerr << "Prior prediction: " << prev_pred_taken << " wh prediction: " <<  bi->whPred << std::endl;
                        //  std::cerr << "Sat Index: " << bi->whIdx << " Sat Value: "<< whTable[pcIndex].SatCtrs[bi->whIdx] << " conf: " << whTable[pcIndex].conf <<std::endl;
                        //  std::cerr << "\n";
                    }
                }
            }
    }

    return pred_taken;
}

void
WormholePredictor::regStats()
{


  whPredictorCorrect
      .name(name() + ".whPredictorCorrect")
      .desc("Number of time the WH predictor is the provider and "
            "the prediction is correct");

  whPredictorWrong
      .name(name() + ".whPredictorWrong")
      .desc("Number of time the WH predictor is the provider and "
            "the prediction is wrong");
}

void
WormholePredictor::addworm_entry(Addr branch_pc, unsigned int instShiftAmt)
{
    WhEntry wormEntry;

    wormEntry.SatCtrs.resize(SatCtrSize, 0);
    wormEntry.PCtag = getPCtag(branch_pc, instShiftAmt);
    wormEntry.conf = 0;
    whTable.push_back(wormEntry);
}

void 
WormholePredictor::UpdateRanking(ThreadID tid, Addr branch_pc, int lsum, int thres, bool loopPredValid, 
                                 unsigned int instShiftAmt, BranchInfo* bi)

{
    // update the rank for entries in whTable if this branch is difficult for TAGE
    if (abs ((lsum*2) + 1) <= thres *3) {
        // std::cerr << "\nUpdate Ranking\n";
        // std::cerr << "Problematic branch identified " << "PC: " << branch_pc << " lsum: " << lsum << std::endl;
        // std::cerr << "PCtag: "<< getPCtag(branch_pc,instShiftAmt) <<std::endl;
        int pcIndex;

        pcIndex = pctagsearch(branch_pc, instShiftAmt);
        if((pcIndex = pctagsearch(branch_pc, instShiftAmt)) > 0){
            // std::cerr << "PC: "<<whTable[pcIndex].PCtag <<   " Before Ranking: " << pcIndex << std::endl;
            std::swap(whTable[pcIndex-1], whTable[pcIndex]);
            // std::cerr << "PC: "<<whTable[pcIndex-1].PCtag <<  " After Ranking: " << pcIndex-1 << std::endl;
        }
        else{
            if(pcIndex < 0 && loopPredValid && bi->whLPTotal >= 7){
                if (whTable.size() == whtsize) {
                // std::cerr << "wormhole entry deleted. wormhole size is " << whTable.size() << std::endl;
                    whTable.pop_back();
                }

                addworm_entry(branch_pc, instShiftAmt);
                //    std::cerr << "\nwormhole entry added. wormhole entry size is " <<whTable.size() << std::endl;
                //    std::cerr << "PCtag: "<< getPCtag(branch_pc,instShiftAmt) << " PC: " << branch_pc << " Branch instance "<< bi << std::endl;
            }
        }

    }
}

void 
WormholePredictor::updatehistorybits(BranchInfo* bi, int entry,  bool taken ){
    // update local history bits
    // std::cerr << "update history bits\n";

    // std::cerr << "Outcome: " << taken;
    if(whTable[entry].localhist.size() >0){
        //std::cerr <<  " history bit before: " << whTable[entry].localhist[0]; 
    }
    if(whTable[entry].localhist.size() == HistoryVectorSize ){
        whTable[entry].localhist.pop_back();
    }

    whTable[entry].localhist.push_front(taken); 
    
}

void 
WormholePredictor::updateConf(BranchInfo* bi, int entry,  bool taken, bool tage_pred)
{
        // std::cerr<< "Update Confindence\n";
        // std::cerr << "Outcome: " << taken << " wormhole prediction: " << bi->whPred << " tage prediction: " << tage_pred << std::endl;
        // std::cerr << "Confidence before: " << whTable[entry].conf; 
    if (bi->whPred  != tage_pred) {
        if (bi->whPred == taken) {
            whTable[entry].conf += (whTable[entry].conf < confMax) ? 1 : 0;
        } else {
            whTable[entry].conf += (whTable[entry].conf > confMin) ? -1 : 0;
        }
        if(bi->usedWhPred ){
            // std::cerr<< "Update Confindence\n";
          //  std::cerr << "Branch instance: "<< bi << std::endl;
        // std::cerr << "Outcome: " << taken << " wormhole prediction: " << bi->whPred << " tage prediction: " << tage_pred << std::endl;
        // std::cerr << "Confidence before: " << whTable[entry].conf; 
      //  std::cerr << "Confidence after: " << whTable[entry].conf <<std::endl;
        }
        
    }
    // std::cerr << "Confidence after: " << whTable[entry].conf <<std::endl;
}

 void
 WormholePredictor::condBranchUpdate(ThreadID tid, Addr branch_pc, bool taken,
                                 bool tage_pred, BranchInfo* bi, int lsum, int thres,  bool loopPredValid,
                                 unsigned instShiftAmt)

{
    // // std::cerr << "Update taken: "<< "LPtotal " << bi->whLPTotal << std::endl;
    UpdateRanking(tid, branch_pc,lsum, thres, loopPredValid, instShiftAmt, bi);
    int pcIndex;
    if((pcIndex = pctagsearch(branch_pc, instShiftAmt)) >=0){
        // std::cerr << "Conditional branch update\n";
        // std::cerr << "Dynamic PC: "<< bi << " PC: "<< branch_pc << std::endl;
        // std::cerr << "Update taken: "<< "LPtotal " << bi->whLPTotal << std::endl;
        // std::cerr << "branch PC tag: " << getPCtag(branch_pc, instShiftAmt) << " whLPTotal " << bi->whLPTotal << std::endl;
        if(bi->whLPTotal >= 7 && !recordingloophistory(pcIndex, bi)){
            if(bi->usedWhPred && bi->whPred != bi->whPrevPred){
              //  std::cerr << "Conditional branch update\n";
              //  std::cerr << "PCtag: "<< getPCtag(branch_pc,instShiftAmt) << " PC: " << branch_pc << " Branch instance: "<< bi << std::endl;
              //  std::cerr << "Update taken: "<< "LPtotal " << bi->whLPTotal << std::endl;                                   
              //  std::cerr << "Outcome: " << taken << " Prior prediction: " << bi->whPrevPred<< " wh prediction: " <<  bi->whPred << std::endl;
              //  std::cerr << "Sat Index: " << bi->whIdx << " Sat Value: "<< whTable[pcIndex].SatCtrs[bi->whIdx] << " conf: " << whTable[pcIndex].conf <<std::endl;

              //  std::cerr << "\n";
            }
            updateConf(bi, pcIndex,  taken, tage_pred);
            ctrUpdate(whTable[pcIndex].SatCtrs[bi->whIdx], taken,  SatCtrWidth);

        }
        updatehistorybits(bi,pcIndex,taken);
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