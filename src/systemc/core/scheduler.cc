/*
 * Copyright 2018 Google, Inc.
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
 * Authors: Gabe Black
 */

#include "systemc/core/scheduler.hh"

#include "base/fiber.hh"
#include "base/logging.hh"
#include "sim/eventq.hh"

namespace sc_gem5
{

Scheduler::Scheduler() :
    eq(nullptr), readyEvent(this, false, EventBase::Default_Pri + 1),
    _numCycles(0), _current(nullptr), initReady(false)
{}

void
Scheduler::prepareForInit()
{
    for (Process *p = toFinalize.getNext(); p; p = toFinalize.getNext()) {
        p->finalize();
        p->popListNode();
    }

    for (Process *p = initList.getNext(); p; p = initList.getNext()) {
        p->finalize();
        ready(p);
    }

    initReady = true;
}

void
Scheduler::reg(Process *p)
{
    if (initReady) {
        // If we're past initialization, finalize static sensitivity.
        p->finalize();
        // Mark the process as ready.
        ready(p);
    } else {
        // Otherwise, record that this process should be initialized once we
        // get there.
        initList.pushLast(p);
    }
}

void
Scheduler::dontInitialize(Process *p)
{
    if (initReady) {
        // Pop this process off of the ready list.
        p->popListNode();
    } else {
        // Push this process onto the list of processes which still need
        // their static sensitivity to be finalized. That implicitly pops it
        // off the list of processes to be initialized/marked ready.
        toFinalize.pushLast(p);
    }
}

void
Scheduler::yield()
{
    _current = readyList.getNext();
    if (!_current) {
        // There are no more processes, so return control to evaluate.
        Fiber::primaryFiber()->run();
    } else {
        _current->popListNode();
        // Switch to whatever Fiber is supposed to run this process. All
        // Fibers which aren't running should be parked at this line.
        _current->fiber()->run();
        // If the current process hasn't been started yet, start it. This
        // should always be true for methods, but may not be true for threads.
        if (_current && !_current->running())
            _current->run();
    }
}

void
Scheduler::ready(Process *p)
{
    // Clump methods together to minimize context switching.
    if (p->procKind() == ::sc_core::SC_METHOD_PROC_)
        readyList.pushFirst(p);
    else
        readyList.pushLast(p);

    scheduleReadyEvent();
}

void
Scheduler::requestUpdate(Channel *c)
{
    updateList.pushLast(c);
    scheduleReadyEvent();
}

void
Scheduler::scheduleReadyEvent()
{
    // Schedule the evaluate and update phases.
    if (!readyEvent.scheduled()) {
        panic_if(!eq, "Need to schedule ready, but no event manager.\n");
        eq->schedule(&readyEvent, eq->getCurTick());
    }
}

void
Scheduler::runReady()
{
    bool empty = readyList.empty();

    // The evaluation phase.
    do {
        yield();
    } while (!readyList.empty());

    if (!empty)
        _numCycles++;

    // The update phase.
    update();

    // The delta phase will happen naturally through the event queue.
}

void
Scheduler::update()
{
    Channel *channel = updateList.getNext();
    while (channel) {
        channel->popListNode();
        channel->update();
        channel = updateList.getNext();
    }
}

Scheduler scheduler;

} // namespace sc_gem5
