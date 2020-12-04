/*
 * Copyright (c) 2014 The University of Wisconsin
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
 */

#ifndef __CPU_PRED_WORMHOLE_PREDICTOR_HH
#define __CPU_PRED_WORMHOLE_PREDICTOR_HH

#include "base/statistics.hh"
#include "base/types.hh"
#include "cpu/static_inst.hh"
#include "params/WormholePredictor.hh"
#include "sim/sim_object.hh"

struct WormholePredictorParams;

class WormholePredictor : public SimObject
{
    // parameters for entries in multi-dimensional history table
    public:
        struct BranchInfo
        {
            int whLPTotal;
            bool usedWhPred;

            // variables for propagating prediction info to the update function
            bool whPred;
            unsigned int whIdx;
            BranchInfo(): whLPTotal(0), whPred(false), whIdx(0) {}
            
        };

        WormholePredictor(const WormholePredictorParams *p);

        bool WhPredict(
        ThreadID tid, Addr branch_pc, bool cond_branch,
        BranchInfo* bi, bool prev_pred_taken, unsigned instShiftAmt);

        void condBranchUpdate(ThreadID tid, Addr branch_pc, bool taken,
                                 bool tage_pred, BranchInfo* bi, int lsum, bool loopPredValid,
                                 unsigned instShiftAmt);

        void updateStats(bool taken, BranchInfo *bi);

        virtual BranchInfo *makeBranchInfo();


    protected:
        const int confMin;
        const int confMax;
        const unsigned HistoryVectorSize;
        const int SatCtrMin;
        const int SatCtrMax;
        const unsigned SatCtrThres;
        const unsigned SatCtrSize;
        const unsigned whtsize;
        const unsigned stats_threshold; 

        // entry in the multi dimensional table
        struct WhEntry {

            uint64_t PCtag; // N-bit tag of PC 
            int conf; // N-bit condidence
            std::vector<bool> localhist; // local history bits
            std::vector<int> SatCtrs; // N-bit saturating counters
            WhEntry(): PCtag(0), conf(0) {}
        };

        uint64_t getPCtag(Addr branch_pc, unsigned instShiftAmt){
            uint64_t PC = (branch_pc >>instShiftAmt) & 0x3ffff;
            return PC;
        }

        unsigned int getSatIndex(int pcIndex, BranchInfo* bi);
        
        std::vector<WhEntry> whTable;


        // stats
        Stats::Scalar whPredictorCorrect;
        Stats::Scalar whPredictorWrong;

    
        void regStats() override;

    public:

        unsigned getHistorySize(){
            return HistoryVectorSize;
        }

    

    protected:
        void updatePrediction(Addr pc, bool Taken, BranchInfo* bi, bool tage_pred);
        void UpdateRanking(ThreadID tid, Addr branch_pc,int lsum, bool loopPredValid, unsigned int instShiftAmt);
        void updateConf(BranchInfo* bi, int entry,  bool taken, bool tage_pred);
        void updateSatCtrs(BranchInfo* bi, int entry,  bool taken);
        void updatehistorybits(BranchInfo* bi, int entry,  bool taken );

};  


#endif//__CPU_PRED_WORMHOLE_PREDICTOR_HH
