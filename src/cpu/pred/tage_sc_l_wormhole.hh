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
 * TAGE-SC-L branch predictor base class (devised by Andre Seznec)
 * It consits of a TAGE + a statistical corrector (SC) + a loop predictor (L)
 */

#ifndef __CPU_PRED_TAGE_SC_WISE
#define __CPU_PRED_TAGE_SC_WISE

#include "cpu/pred/tage_sc_l.hh"
#include "cpu/pred/tage_sc_l_64KB.hh"
#include "cpu/pred/wormhole_predictor.hh"
#include "params/TAGE_SC_L_WISE.hh"
#include "params/TAGE_SC_L_WISE_TAGE.hh"
#include "params/TAGE_SC_L_WISE_StatisticalCorrector.hh"


class TAGE_SC_L_WISE_TAGE : public TAGE_SC_L_TAGE_64KB {

    TAGE_SC_L_WISE_TAGE(const TAGE_SC_L_WISE_TAGE *p)
      : TAGE_SC_L_TAGE_64KB(p)
    {}

};

class TAGE_SC_L_WISE_StatisticalCorrector : public TAGE_SC_L_64KB_StatisticalCorrector
{
    public:
    TAGE_SC_L_WISE_StatisticalCorrector(TAGE_SC_L_WISE_StatisticalCorrectorParams *p) : TAGE_SC_L_64KB_StatisticalCorrector(p) {};

};

class TAGE_SC_L_WISE : public TAGE_SC_L
{
  WormholePredictor *wormholepredictor;
  public:
    TAGE_SC_L_WISE(const TAGE_SC_L_WISEParams *params);

  bool predict(
        ThreadID tid, Addr branch_pc, bool cond_branch, void* &b) override;

    void regStats() override;

    void update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history,
                bool squashed, const StaticInstPtr & inst,
                Addr corrTarget = MaxAddr) override;

  protected:

    struct TageSCLWISEBranchInfo : public TageSCLBranchInfo
    {
        WormholePredictor::BranchInfo *whBranchInfo;

        TageSCLWISEBranchInfo(TAGEBase &tage, StatisticalCorrector &sc,
                          LoopPredictor &lp, WormholePredictor &wh )
          : TageSCLBranchInfo(tage, sc, lp), whBranchInfo(wh.makeBranchInfo())
        {}

        virtual ~TageSCLWISEBranchInfo()
        {
            delete whBranchInfo;
        }
    };
};


#endif // __CPU_PRED_TAGE_SC_WISE