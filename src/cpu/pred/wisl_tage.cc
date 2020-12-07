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
 * WISL-TAGE branch predictor 
 * It consits of a TAGE + a statistical corrector (SC) + a loop predictor (L) + wormhole predictor
 */

#include <iostream>

#include "cpu/pred/wisl_tage.hh"

#include "base/random.hh"


WISL_TAGE::WISL_TAGE(const WISL_TAGEParams *p)
  : TAGE_SC_L(p), wormholepredictor(p->wormholepredictor)
{
    // std::cerr << "Inside the constructor\n";
}

bool
WISL_TAGE::predict(ThreadID tid, Addr branch_pc, bool cond_branch, void* &b)
{
    WISLTageBranchInfo *bi = new WISLTageBranchInfo(*tage,
                                                  *statisticalCorrector,
                                                  *loopPredictor, *wormholepredictor);
    b = (void*)(bi);

    bool pred_taken = tage->tagePredict(tid, branch_pc, cond_branch,
                                        bi->tageBranchInfo);
    pred_taken = loopPredictor->loopPredict(tid, branch_pc, cond_branch,
                                            bi->lpBranchInfo, pred_taken,
                                            instShiftAmt);

    
    if (bi->lpBranchInfo->numIter > 0){
        // std::cerr<<"Wise Tage Prediciton\n";
        // // std::cerr << "PC: "<< branch_pc;
        //   std::cerr<<" .loop BranchInfo->numIter: " << bi->lpBranchInfo->numIter<< " Loop Valid: " <<bi->lpBranchInfo->loopPredValid<< std::endl;

    }

    if(bi->lpBranchInfo->loopPredValid){ // if loop hit, record number of iterations in loop
        bi->whBranchInfo->whLPTotal = 0;
        if(bi->lpBranchInfo->numIter <= wormholepredictor->getHistorySize()){
            bi->whBranchInfo->whLPTotal = bi->lpBranchInfo->numIter;
            // // std::cerr << "wormhole branchinfo->whLPTotal: " << bi->whBranchInfo->whLPTotal << std::endl;
        }
        
    }

    if (bi->lpBranchInfo->loopPredUsed) {
        bi->tageBranchInfo->provider = LOOP;
    }

    TAGE_SC_L_TAGE::BranchInfo* tage_scl_bi =
        static_cast<TAGE_SC_L_TAGE::BranchInfo *>(bi->tageBranchInfo);

    // Copy the confidences computed by TAGE
    bi->scBranchInfo->lowConf = tage_scl_bi->lowConf;
    bi->scBranchInfo->highConf = tage_scl_bi->highConf;
    bi->scBranchInfo->altConf = tage_scl_bi->altConf;
    bi->scBranchInfo->medConf = tage_scl_bi->medConf;

    bool use_tage_ctr = bi->tageBranchInfo->hitBank > 0;
    int8_t tage_ctr = use_tage_ctr ?
        tage->getCtr(tage_scl_bi->hitBank, tage_scl_bi->hitBankIndex) : 0;
    bool bias = (bi->tageBranchInfo->longestMatchPred !=
                 bi->tageBranchInfo->altTaken);

    pred_taken = statisticalCorrector->scPredict(tid, branch_pc, cond_branch,
            bi->scBranchInfo, pred_taken, bias, use_tage_ctr, tage_ctr,
            tage->getTageCtrBits(), bi->tageBranchInfo->hitBank,
            bi->tageBranchInfo->altBank, tage->getPathHist(tid));

    if (bi->scBranchInfo->usedScPred) {
        bi->tageBranchInfo->provider = SC;
    }

    //WH
    pred_taken = statisticalCorrector-> whPredict(tid, branch_pc, cond_branch,
        bi->scBranchInfo,pred_taken,bi->lpBranchInfo->numIter);
    pred_taken = wormholepredictor->WhPredict(tid, branch_pc, cond_branch, bi->whBranchInfo, pred_taken, instShiftAmt);

    if(bi->whBranchInfo->usedWhPred){
        // cerr << 
        bi->tageBranchInfo->provider = WH;
    }

    // record final prediction
    bi->lpBranchInfo->predTaken = pred_taken;

    return pred_taken;
}

void
WISL_TAGE::update(ThreadID tid, Addr branch_pc, bool taken, void *bp_history,
        bool squashed, const StaticInstPtr & inst, Addr corrTarget)
{
    assert(bp_history);

    WISLTageBranchInfo* bi = static_cast<WISLTageBranchInfo*>(bp_history);
    TAGE_SC_L_TAGE::BranchInfo* tage_bi =
        static_cast<TAGE_SC_L_TAGE::BranchInfo *>(bi->tageBranchInfo);

    if (squashed) {
        if (tage->isSpeculativeUpdateEnabled()) {
            // This restores the global history, then update it
            // and recomputes the folded histories.
            tage->squash(tid, taken, tage_bi, corrTarget);
            if (bi->tageBranchInfo->condBranch) {
                loopPredictor->squashLoop(bi->lpBranchInfo);
            }
        }
        return;
    }

    int nrand = random_mt.random<int>() & 3;
    if (tage_bi->condBranch) {
        // DPRINTF(TageSCL, "Updating tables for branch:%lx; taken?:%d\n",
        //         branch_pc, taken);
        tage->updateStats(taken, bi->tageBranchInfo);

        loopPredictor->updateStats(taken, bi->lpBranchInfo);

        statisticalCorrector->updateStats(taken, bi->scBranchInfo);
        wormholepredictor->updateStats(taken, bi->whBranchInfo);

        // statisticalCorrector->whUpdate(tid, branch_pc, taken, bi->scBranchInfo,
        //                                bi->lpBranchInfo->numIter);

        wormholepredictor->condBranchUpdate(tid, branch_pc, taken,
            bi->whBranchInfo->whPrevPred, bi->whBranchInfo, bi->scBranchInfo->lsum,
            bi->lpBranchInfo->loopPredValid, instShiftAmt);

        bool bias = (bi->tageBranchInfo->longestMatchPred !=
                     bi->tageBranchInfo->altTaken);
        statisticalCorrector->condBranchUpdate(tid, branch_pc, taken,
            bi->scBranchInfo, corrTarget, bias, bi->tageBranchInfo->hitBank,
            bi->tageBranchInfo->altBank, tage->getPathHist(tid));

        loopPredictor->condBranchUpdate(tid, branch_pc, taken,
                bi->tageBranchInfo->tagePred, bi->lpBranchInfo, instShiftAmt);

        tage->condBranchUpdate(tid, branch_pc, taken, bi->tageBranchInfo,
                               nrand, corrTarget, bi->lpBranchInfo->predTaken);
    }

    if (!tage->isSpeculativeUpdateEnabled()) {
        statisticalCorrector->scHistoryUpdate(branch_pc, inst, taken,
                                              bi->scBranchInfo, corrTarget);

        tage->updateHistories(tid, branch_pc, taken, bi->tageBranchInfo, false,
                              inst, corrTarget);
    }

    delete bi;
}

void
WISL_TAGE::regStats()
{
    TAGE_SC_L::regStats();
}

WISL_TAGE*
WISL_TAGEParams::create()
{
    return new WISL_TAGE(this);
}