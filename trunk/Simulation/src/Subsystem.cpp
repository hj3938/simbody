/* -------------------------------------------------------------------------- *
 *                      SimTK Core: SimTKcommon                               *
 * -------------------------------------------------------------------------- *
 * This is part of the SimTK Core biosimulation toolkit originating from      *
 * Simbios, the NIH National Center for Physics-Based Simulation of           *
 * Biological Structures at Stanford, funded under the NIH Roadmap for        *
 * Medical Research, grant U54 GM072970. See https://simtk.org.               *
 *                                                                            *
 * Portions copyright (c) 2006-7 Stanford University and the Authors.         *
 * Authors: Michael Sherman                                                   *
 * Contributors:                                                              *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person obtaining a    *
 * copy of this software and associated documentation files (the "Software"), *
 * to deal in the Software without restriction, including without limitation  *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
 * and/or sell copies of the Software, and to permit persons to whom the      *
 * Software is furnished to do so, subject to the following conditions:       *
 *                                                                            *
 * The above copyright notice and this permission notice shall be included in *
 * all copies or substantial portions of the Software.                        *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
 * THE AUTHORS, CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,    *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR      *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE  *
 * USE OR OTHER DEALINGS IN THE SOFTWARE.                                     *
 * -------------------------------------------------------------------------- */


/**@file
 *
 * Implementation of Subsystem, Subsystem::Guts and DefaultSystemSubsystem.
 */

#include "SimTKcommon/basics.h"
#include "SimTKcommon/Simmatrix.h"
#include "SimTKcommon/internal/EventHandler.h"
#include "SimTKcommon/internal/EventReporter.h"
#include "SimTKcommon/internal/System.h"
#include "SimTKcommon/internal/Subsystem.h"

#include "SystemGutsRep.h"
#include "SubsystemGutsRep.h"

#include <cassert>
#include <map>
#include <set>

namespace SimTK {

    ///////////////
    // SUBSYSTEM //
    ///////////////

bool Subsystem::isEmptyHandle() const {return guts==0;}
bool Subsystem::isOwnerHandle() const {return guts==0 || &guts->getOwnerSubsystemHandle()==this;}
bool Subsystem::isSameSubsystem(const Subsystem& otherSubsystem) const {
    return guts && (guts==otherSubsystem.guts);
}


Subsystem::Subsystem(const Subsystem& src) : guts(0) {
    if (src.guts) {
        guts = src.guts->clone();
        guts->setOwnerSubsystemHandle(*this);
    }
}

Subsystem& Subsystem::operator=(const Subsystem& src) {
    if (!isSameSubsystem(src)) {
        if (isOwnerHandle())
            Subsystem::Guts::destruct(guts); 
        guts=0;
        if (src.guts) {
            guts = src.guts->clone();
            guts->setOwnerSubsystemHandle(*this);
        }
    }
    return *this;
}

Subsystem::~Subsystem() {
    // Must delete using the library-side VFT, so that we can get access
    // to the client side virtual destructor to destruct this client-side
    // System::Guts object.
    if (guts && isOwnerHandle())
        Subsystem::Guts::destruct(guts);
    guts=0;
}

void Subsystem::adoptSubsystemGuts(Subsystem::Guts* g) {
    SimTK_ASSERT_ALWAYS(g, "Subsystem::adoptSubsystemGuts(): can't adopt null Guts");
    SimTK_ASSERT_ALWAYS(!guts,
        "Subsystem::adoptSubsystemGuts(): this Subsystem handle is already in use");
    guts = g;
    guts->setOwnerSubsystemHandle(*this);
}

void Subsystem::setSystem(System& sys, SubsystemId id) {
    updSubsystemGuts().setSystem(sys,id);
}

const String& Subsystem::getName()    const {return getSubsystemGuts().getName();}
const String& Subsystem::getVersion() const {return getSubsystemGuts().getVersion();}

bool Subsystem::subsystemTopologyHasBeenRealized() const {
    return getSubsystemGuts().subsystemTopologyHasBeenRealized();
}

void Subsystem::invalidateSubsystemTopologyCache() const {
    return getSubsystemGuts().invalidateSubsystemTopologyCache(); // mutable
}

bool Subsystem::isInSystem() const {return getSubsystemGuts().isInSystem();}
bool Subsystem::isInSameSystem(const Subsystem& otherSubsystem) const {
    return getSubsystemGuts().isInSameSystem(otherSubsystem);
}

const System& Subsystem::getSystem() const {return getSubsystemGuts().getSystem();}
System&       Subsystem::updSystem()       {return updSubsystemGuts().updSystem();}

SubsystemId Subsystem::getMySubsystemId() const {
    return getSubsystemGuts().getMySubsystemId();
}

const Vector& Subsystem::getQ(const State& s) const {return getSubsystemGuts().getQ(s);}
const Vector& Subsystem::getU(const State& s) const {return getSubsystemGuts().getU(s);}
const Vector& Subsystem::getZ(const State& s) const {return getSubsystemGuts().getZ(s);}
const Vector& Subsystem::getQDot(const State& s) const {return getSubsystemGuts().getQDot(s);}
const Vector& Subsystem::getUDot(const State& s) const {return getSubsystemGuts().getUDot(s);}
const Vector& Subsystem::getZDot(const State& s) const {return getSubsystemGuts().getZDot(s);}
const Vector& Subsystem::getQDotDot(const State& s) const {return getSubsystemGuts().getQDotDot(s);}
const Vector& Subsystem::getQErr(const State& s) const {return getSubsystemGuts().getQErr(s);}
const Vector& Subsystem::getUErr(const State& s) const {return getSubsystemGuts().getUErr(s);}
const Vector& Subsystem::getUDotErr(const State& s) const {return getSubsystemGuts().getUDotErr(s);}
const Vector& Subsystem::getMultipliers(const State& s) const {return getSubsystemGuts().getMultipliers(s);}

Vector& Subsystem::updQ(State& s) const {return getSubsystemGuts().updQ(s);}
Vector& Subsystem::updU(State& s) const {return getSubsystemGuts().updU(s);}
Vector& Subsystem::updZ(State& s) const {return getSubsystemGuts().updZ(s);}

Vector& Subsystem::updQDot(const State& s) const {return getSubsystemGuts().updQDot(s);}
Vector& Subsystem::updUDot(const State& s) const {return getSubsystemGuts().updUDot(s);}
Vector& Subsystem::updZDot(const State& s) const {return getSubsystemGuts().updZDot(s);}
Vector& Subsystem::updQDotDot(const State& s) const {return getSubsystemGuts().updQDotDot(s);}
Vector& Subsystem::updQErr(const State& s) const {return getSubsystemGuts().updQErr(s);}
Vector& Subsystem::updUErr(const State& s) const {return getSubsystemGuts().updUErr(s);}
Vector& Subsystem::updUDotErr(const State& s) const {return getSubsystemGuts().updUDotErr(s);}
Vector& Subsystem::updMultipliers(const State& s) const {return getSubsystemGuts().updMultipliers(s);}

Stage Subsystem::getStage(const State& s) const {return getSubsystemGuts().getStage(s);}
const AbstractValue& Subsystem::getDiscreteVariable(const State& s, int index) const {
    return getSubsystemGuts().getDiscreteVariable(s, index);
}
AbstractValue& Subsystem::updDiscreteVariable(State& s, int index) const {
    return getSubsystemGuts().updDiscreteVariable(s, index);
}
const AbstractValue& Subsystem::getCacheEntry(const State& s, int index) const {
    return getSubsystemGuts().getCacheEntry(s, index);
}
AbstractValue& Subsystem::updCacheEntry(const State& s, int index) const {
    return getSubsystemGuts().updCacheEntry(s, index);
}

int Subsystem::getQStart      (const State& s) const {return getSubsystemGuts().getQStart(s);}
int Subsystem::getNQ          (const State& s) const {return getSubsystemGuts().getNQ(s);}
int Subsystem::getUStart      (const State& s) const {return getSubsystemGuts().getUStart(s);}
int Subsystem::getNU          (const State& s) const {return getSubsystemGuts().getNU(s);}
int Subsystem::getZStart      (const State& s) const {return getSubsystemGuts().getZStart(s);}
int Subsystem::getNZ          (const State& s) const {return getSubsystemGuts().getNZ(s);}
int Subsystem::getQErrStart   (const State& s) const {return getSubsystemGuts().getQErrStart(s);}
int Subsystem::getNQErr       (const State& s) const {return getSubsystemGuts().getNQErr(s);}
int Subsystem::getUErrStart   (const State& s) const {return getSubsystemGuts().getUErrStart(s);}
int Subsystem::getNUErr       (const State& s) const {return getSubsystemGuts().getNUErr(s);}
int Subsystem::getUDotErrStart(const State& s) const {return getSubsystemGuts().getUDotErrStart(s);}
int Subsystem::getNUDotErr    (const State& s) const {return getSubsystemGuts().getNUDotErr(s);}
int Subsystem::getMultipliersStart(const State& s) const {return getSubsystemGuts().getMultipliersStart(s);}
int Subsystem::getNMultipliers    (const State& s) const {return getSubsystemGuts().getNMultipliers(s);}

    /////////////////////
    // SUBSYSTEM::GUTS //
    /////////////////////

// Default constructor is inline, but calls librarySideConstuction() here.
void Subsystem::Guts::librarySideConstruction(const String& name, const String& version) {
    rep = new GutsRep(name,version);
    // note that the GutsRep object currently has no owner handle
}

// Destructor is inline, but calls librarySideDestruction() here.
void Subsystem::Guts::librarySideDestruction() {
    delete rep; 
    rep=0;
}


// Copy constructor
Subsystem::Guts::Guts(const Guts& src) : rep(0) {
    if (src.rep) {
        rep = new Subsystem::Guts::GutsRep(*src.rep);
        // note that the GutsRep object currently has no owner handle
    }
}

// Copy assignment is suppressed
    

const Subsystem& Subsystem::Guts::getOwnerSubsystemHandle() const {
    assert(rep->myHandle);
    return *rep->myHandle;
}

Subsystem& Subsystem::Guts::updOwnerSubsystemHandle() {
    assert(rep->myHandle);
    return *rep->myHandle;
}

void Subsystem::Guts::setOwnerSubsystemHandle(Subsystem& sys) {
    // might be the first owner or a replacement
    rep->myHandle = &sys;
}

bool Subsystem::Guts::hasOwnerSubsystemHandle() const {
    return rep->myHandle != 0;
}

void Subsystem::Guts::setSystem(System& sys, SubsystemId id) {
    updRep().setSystem(sys,id);
}

const String& Subsystem::Guts::getName()    const {return getRep().getName();}
const String& Subsystem::Guts::getVersion() const {return getRep().getVersion();}


void Subsystem::Guts::registerDestructImpl(DestructImplLocator f) {
    updRep().destructp = f;
}
void Subsystem::Guts::registerCloneImpl(CloneImplLocator f) {
    updRep().clonep = f;
}

void Subsystem::Guts::registerRealizeTopologyImpl(RealizeWritableStateImplLocator f) {
    updRep().realizeTopologyp = f;
}
void Subsystem::Guts::registerRealizeModelImpl(RealizeWritableStateImplLocator f) {
    updRep().realizeModelp = f;
}
void Subsystem::Guts::registerRealizeInstanceImpl(RealizeConstStateImplLocator f) {
    updRep().realizeInstancep = f;
}
void Subsystem::Guts::registerRealizeTimeImpl(RealizeConstStateImplLocator f) {
    updRep().realizeTimep = f;
}
void Subsystem::Guts::registerRealizePositionImpl(RealizeConstStateImplLocator f) {
    updRep().realizePositionp = f;
}
void Subsystem::Guts::registerRealizeVelocityImpl(RealizeConstStateImplLocator f) {
    updRep().realizeVelocityp = f;
}
void Subsystem::Guts::registerRealizeDynamicsImpl(RealizeConstStateImplLocator f) {
    updRep().realizeDynamicsp = f;
}
void Subsystem::Guts::registerRealizeAccelerationImpl(RealizeConstStateImplLocator f) {
    updRep().realizeAccelerationp = f;
}
void Subsystem::Guts::registerRealizeReportImpl(RealizeConstStateImplLocator f) {
    updRep().realizeReportp = f;
}

void Subsystem::Guts::registerCalcQUnitWeightsImpl(CalcUnitWeightsImplLocator f) {
    updRep().calcQUnitWeightsp = f;
}
void Subsystem::Guts::registerCalcUUnitWeightsImpl(CalcUnitWeightsImplLocator f) {
    updRep().calcUUnitWeightsp = f;
}
void Subsystem::Guts::registerCalcZUnitWeightsImpl(CalcUnitWeightsImplLocator f) {
    updRep().calcZUnitWeightsp = f;
}
void Subsystem::Guts::registerCalcQErrUnitTolerancesImpl(CalcUnitWeightsImplLocator f) {
    updRep().calcQErrUnitTolerancesp = f;
}
void Subsystem::Guts::registerCalcUErrUnitTolerancesImpl(CalcUnitWeightsImplLocator f) {
    updRep().calcUErrUnitTolerancesp = f;
}
void Subsystem::Guts::registerCalcDecorativeGeometryAndAppendImpl(CalcDecorativeGeometryAndAppendImplLocator f) {
    updRep().calcDecorativeGeometryAndAppendp = f;
}

bool Subsystem::Guts::isInSystem() const {return getRep().isInSystem();}
bool Subsystem::Guts::isInSameSystem(const Subsystem& otherSubsystem) const {
	return getRep().isInSameSystem(otherSubsystem);
}
const System& Subsystem::Guts::getSystem() const {return getRep().getSystem();}
System&       Subsystem::Guts::updSystem()	     {return updRep().updSystem();}
SubsystemId   Subsystem::Guts::getMySubsystemId() const {return getRep().getMySubsystemId();}

int Subsystem::Guts::allocateQ(State& s, const Vector& qInit) const {
    return s.allocateQ(getRep().getMySubsystemId(), qInit);
}

int Subsystem::Guts::allocateU(State& s, const Vector& uInit) const {
    return s.allocateU(getRep().getMySubsystemId(), uInit);
}

int Subsystem::Guts::allocateZ(State& s, const Vector& zInit) const {
    return s.allocateZ(getRep().getMySubsystemId(), zInit);
}

int Subsystem::Guts::allocateQErr(State& s, int nqerr) const {
    return s.allocateQErr(getRep().getMySubsystemId(), nqerr);
}

int Subsystem::Guts::allocateUErr(State& s, int nuerr) const {
    return s.allocateUErr(getRep().getMySubsystemId(), nuerr);
}

// Multipliers are added as a side effect.
int Subsystem::Guts::allocateUDotErr(State& s, int nudoterr) const {
    return s.allocateUDotErr(getRep().getMySubsystemId(), nudoterr);
}

int Subsystem::Guts::allocateDiscreteVariable(State& s, Stage g, AbstractValue* v) const {
    return s.allocateDiscreteVariable(getRep().getMySubsystemId(), g, v);
}

int Subsystem::Guts::allocateCacheEntry(State& s, Stage g, AbstractValue* v) const {
    return s.allocateCacheEntry(getRep().getMySubsystemId(), g, v);
}

void Subsystem::Guts::advanceToStage(const State& s, Stage g) const {
    s.advanceSubsystemToStage(getRep().getMySubsystemId(), g);
}

Stage Subsystem::Guts::getStage(const State& s) const {
    return s.getSubsystemStage(getRep().getMySubsystemId());
}
const AbstractValue& Subsystem::Guts::getDiscreteVariable(const State& s, int index) const {
    return s.getDiscreteVariable(getRep().getMySubsystemId(), index);
}

AbstractValue& Subsystem::Guts::updDiscreteVariable(State& s, int index) const {
    return s.updDiscreteVariable(getRep().getMySubsystemId(), index);
}

const AbstractValue& Subsystem::Guts::getCacheEntry(const State& s, int index) const {
    return s.getCacheEntry(getRep().getMySubsystemId(), index);
}

AbstractValue& Subsystem::Guts::updCacheEntry(const State& s, int index) const {
    return s.updCacheEntry(getRep().getMySubsystemId(), index);
}

const Vector& Subsystem::Guts::getQ(const State& s) const {return s.getQ(getRep().getMySubsystemId());}
const Vector& Subsystem::Guts::getU(const State& s) const {return s.getU(getRep().getMySubsystemId());}
const Vector& Subsystem::Guts::getZ(const State& s) const {return s.getZ(getRep().getMySubsystemId());}

Vector& Subsystem::Guts::updQ(State& s) const {return s.updQ(getRep().getMySubsystemId());}
Vector& Subsystem::Guts::updU(State& s) const {return s.updU(getRep().getMySubsystemId());}
Vector& Subsystem::Guts::updZ(State& s) const {return s.updZ(getRep().getMySubsystemId());}

const Vector& Subsystem::Guts::getQDot   (const State& s) const {return s.getQDot(getRep().getMySubsystemId());}
const Vector& Subsystem::Guts::getUDot   (const State& s) const {return s.getUDot(getRep().getMySubsystemId());}
const Vector& Subsystem::Guts::getZDot   (const State& s) const {return s.getZDot(getRep().getMySubsystemId());}
const Vector& Subsystem::Guts::getQDotDot(const State& s) const {return s.getQDotDot(getRep().getMySubsystemId());}

Vector& Subsystem::Guts::updQDot   (const State& s) const {return s.updQDot(getRep().getMySubsystemId());}
Vector& Subsystem::Guts::updUDot   (const State& s) const {return s.updUDot(getRep().getMySubsystemId());}
Vector& Subsystem::Guts::updZDot   (const State& s) const {return s.updZDot(getRep().getMySubsystemId());}
Vector& Subsystem::Guts::updQDotDot(const State& s) const {return s.updQDotDot(getRep().getMySubsystemId());}

const Vector& Subsystem::Guts::getQErr(const State& s) const {return s.getQErr(getRep().getMySubsystemId());}
const Vector& Subsystem::Guts::getUErr(const State& s) const {return s.getUErr(getRep().getMySubsystemId());}
const Vector& Subsystem::Guts::getUDotErr(const State& s) const {return s.getUDotErr(getRep().getMySubsystemId());}
const Vector& Subsystem::Guts::getMultipliers(const State& s) const {return s.getMultipliers(getRep().getMySubsystemId());}

Vector& Subsystem::Guts::updQErr(const State& s) const {return s.updQErr(getRep().getMySubsystemId());}
Vector& Subsystem::Guts::updUErr(const State& s) const {return s.updUErr(getRep().getMySubsystemId());}
Vector& Subsystem::Guts::updUDotErr(const State& s) const {return s.updUDotErr(getRep().getMySubsystemId());}
Vector& Subsystem::Guts::updMultipliers(const State& s) const {return s.updMultipliers(getRep().getMySubsystemId());}

int Subsystem::Guts::getQStart(const State& s) const {return s.getQStart(getRep().getMySubsystemId());}
int Subsystem::Guts::getNQ(const State& s)     const {return s.getNQ(getRep().getMySubsystemId());}

int Subsystem::Guts::getUStart(const State& s) const {return s.getUStart(getRep().getMySubsystemId());}
int Subsystem::Guts::getNU(const State& s)     const {return s.getNU(getRep().getMySubsystemId());}

int Subsystem::Guts::getZStart(const State& s) const {return s.getZStart(getRep().getMySubsystemId());}
int Subsystem::Guts::getNZ(const State& s)     const {return s.getNZ(getRep().getMySubsystemId());}

int Subsystem::Guts::getQErrStart(const State& s) const {return s.getQErrStart(getRep().getMySubsystemId());}
int Subsystem::Guts::getNQErr(const State& s)     const {return s.getNQErr(getRep().getMySubsystemId());}

int Subsystem::Guts::getUErrStart(const State& s) const {return s.getUErrStart(getRep().getMySubsystemId());}
int Subsystem::Guts::getNUErr(const State& s)     const {return s.getNUErr(getRep().getMySubsystemId());}

int Subsystem::Guts::getUDotErrStart(const State& s) const {return s.getUDotErrStart(getRep().getMySubsystemId());}
int Subsystem::Guts::getNUDotErr(const State& s)     const {return s.getNUDotErr(getRep().getMySubsystemId());}

int Subsystem::Guts::getMultipliersStart(const State& s) const {return s.getMultipliersStart(getRep().getMySubsystemId());}
int Subsystem::Guts::getNMultipliers(const State& s)     const {return s.getNMultipliers(getRep().getMySubsystemId());}

void Subsystem::Guts::invalidateSubsystemTopologyCache() const {
    getRep().invalidateSubsystemTopologyCache();
}

bool Subsystem::Guts::subsystemTopologyHasBeenRealized() const {
    return getRep().subsystemTopologyHasBeenRealized();
}

/**
 * A Subsystem should invoke this method during Model stage for each scheduled event it defines.
 * It allocates a global event ID for the event, and registers that ID as belonging to this Subsystem.
 * 
 * @param state     the State which is being realized
 * @param eventId   on exit, the newly allocated event ID is stored here
 */

void Subsystem::Guts::createScheduledEvent(State& state, int& eventId) const {
    eventId = getSystem().getDefaultSubsystem().createEventId(getMySubsystemId(), state);
}

/**
 * A Subsystem should invoke this method during Model stage for each triggered event it defines.
 * It allocates a global event ID for the event, registers that ID as belonging to this Subsystem,
 * and allocates space in the State for the event trigger function.
 * 
 * @param state     the State which is being realized
 * @param eventId   on exit, the newly allocated event ID is stored here
 * @param triggerFunctionIndex  on exit, the index corresponding to the event's trigger function
 *                              is stored here
 * @param stage     the Stage at which the event will be evaluated
 */

void Subsystem::Guts::createTriggeredEvent(State& state, int& eventId, int& triggerFunctionIndex, Stage stage) const {
    eventId = getSystem().getDefaultSubsystem().createEventId(getMySubsystemId(), state);
    triggerFunctionIndex = state.allocateEvent(getMySubsystemId(), stage, 1);
}

    // wrappers for Subsystem::Guts virtuals


Subsystem::Guts* Subsystem::Guts::clone() const {
    return getRep().clonep(*this);
}


/*static*/void Subsystem::Guts::destruct(Subsystem::Guts* gutsp) {
    if (gutsp)
        gutsp->getRep().destructp(gutsp);
}

void Subsystem::Guts::realizeSubsystemTopology(State& s) const {
    SimTK_STAGECHECK_EQ_ALWAYS(getStage(s), Stage::Empty, 
        "Subsystem::Guts::realizeSubsystemTopology()");
    getRep().realizeTopologyp(*this,s);
    getRep().subsystemTopologyRealized = true; // mark the subsystem itself (mutable)
    advanceToStage(s, Stage::Topology);  // mark the State as well
}
void Subsystem::Guts::realizeSubsystemModel(State& s) const {
    SimTK_STAGECHECK_TOPOLOGY_REALIZED_ALWAYS(subsystemTopologyHasBeenRealized(),
        "Subsystem", getName(), "Subsystem::Guts::realizeSubsystemModel()");

    SimTK_STAGECHECK_GE_ALWAYS(getStage(s), Stage::Topology, 
        "Subsystem::Guts::realizeSubsystemModel()");
    if (getStage(s) < Stage::Model) {
        getRep().realizeModelp(*this,s);
        advanceToStage(s, Stage::Model);
    }
}
void Subsystem::Guts::realizeSubsystemInstance(const State& s) const { 
    SimTK_STAGECHECK_GE_ALWAYS(getStage(s), Stage(Stage::Instance).prev(), 
        "Subsystem::Guts::realizeSubsystemInstance()");
    if (getStage(s) < Stage::Instance) {
        getRep().realizeInstancep(*this,s);
        advanceToStage(s, Stage::Instance);
    }
}
void Subsystem::Guts::realizeSubsystemTime(const State& s) const { 
    SimTK_STAGECHECK_GE_ALWAYS(getStage(s), Stage(Stage::Time).prev(), 
        "Subsystem::Guts::realizeTime()");
    if (getStage(s) < Stage::Time) {
        getRep().realizeTimep(*this,s);
        advanceToStage(s, Stage::Time);
    }
}
void Subsystem::Guts::realizeSubsystemPosition(const State& s) const { 
    SimTK_STAGECHECK_GE_ALWAYS(getStage(s), Stage(Stage::Position).prev(), 
        "Subsystem::Guts::realizeSubsystemPosition()");
    if (getStage(s) < Stage::Position) {
        getRep().realizePositionp(*this,s);
        advanceToStage(s, Stage::Position);
    }
}
void Subsystem::Guts::realizeSubsystemVelocity(const State& s) const { 
    SimTK_STAGECHECK_GE_ALWAYS(getStage(s), Stage(Stage::Velocity).prev(), 
        "Subsystem::Guts::realizeSubsystemVelocity()");
    if (getStage(s) < Stage::Velocity) {
        getRep().realizeVelocityp(*this,s);
        advanceToStage(s, Stage::Velocity);
    }
}
void Subsystem::Guts::realizeSubsystemDynamics(const State& s) const { 
    SimTK_STAGECHECK_GE_ALWAYS(getStage(s), Stage(Stage::Dynamics).prev(), 
        "Subsystem::Guts::realizeSubsystemDynamics()");
    if (getStage(s) < Stage::Dynamics) {
        getRep().realizeDynamicsp(*this,s);
        advanceToStage(s, Stage::Dynamics);
    }
}
void Subsystem::Guts::realizeSubsystemAcceleration(const State& s) const { 
    SimTK_STAGECHECK_GE_ALWAYS(getStage(s), Stage(Stage::Acceleration).prev(), 
        "Subsystem::Guts::realizeSubsystemAcceleration()");
    if (getStage(s) < Stage::Acceleration) {
        getRep().realizeAccelerationp(*this,s);
        advanceToStage(s, Stage::Acceleration);
    }
}
void Subsystem::Guts::realizeSubsystemReport(const State& s) const { 
    SimTK_STAGECHECK_GE_ALWAYS(getStage(s), Stage(Stage::Report).prev(), 
        "Subsystem::Guts::realizeSubsystemReport()");
    if (getStage(s) < Stage::Report) {
        getRep().realizeReportp(*this,s);
        advanceToStage(s, Stage::Report);
    }
}


void Subsystem::Guts::calcQUnitWeights(const State& s, Vector& weights) const {
    getRep().calcQUnitWeightsp(*this,s,weights);
}
void Subsystem::Guts::calcUUnitWeights(const State& s, Vector& weights) const {
    getRep().calcUUnitWeightsp(*this,s,weights);
}
void Subsystem::Guts::calcZUnitWeights(const State& s, Vector& weights) const {
    getRep().calcZUnitWeightsp(*this,s,weights);
}
void Subsystem::Guts::calcQErrUnitTolerances(const State& s, Vector& tolerances) const {
    getRep().calcQErrUnitTolerancesp(*this,s,tolerances);
}
void Subsystem::Guts::calcUErrUnitTolerances(const State& s, Vector& tolerances) const {
    getRep().calcUErrUnitTolerancesp(*this,s,tolerances);
}

void Subsystem::Guts::calcDecorativeGeometryAndAppend(const State& s, Stage stage, Array<DecorativeGeometry>& geom) const {
    getRep().calcDecorativeGeometryAndAppendp(*this,s,stage,geom);
}



    // default implementations for Subsystem::Guts virtuals
/*virtual*/ int Subsystem::Guts::realizeSubsystemTopologyImpl(State& s) const {
    return 0;
}
/*virtual*/ int Subsystem::Guts::realizeSubsystemModelImpl(State& s) const {
    return 0; 
}
/*virtual*/ int Subsystem::Guts::realizeSubsystemInstanceImpl(const State& s) const {
    return 0; 
}
/*virtual*/ int Subsystem::Guts::realizeSubsystemTimeImpl(const State& s) const {
    return 0; 
}
/*virtual*/ int Subsystem::Guts::realizeSubsystemPositionImpl(const State& s) const {
    return 0; 
}
/*virtual*/ int Subsystem::Guts::realizeSubsystemVelocityImpl(const State& s) const {
    return 0; 
}
/*virtual*/ int Subsystem::Guts::realizeSubsystemDynamicsImpl(const State& s) const {
    return 0; 
}
/*virtual*/ int Subsystem::Guts::realizeSubsystemAccelerationImpl(const State& s) const {
    return 0; 
}
/*virtual*/ int Subsystem::Guts::realizeSubsystemReportImpl(const State& s) const {
    return 0; 
}

/*virtual*/ int Subsystem::Guts::calcQUnitWeightsImpl(const State& s, Vector& weights) const {
    weights.resize(getNQ(s));
    weights = 1; // default says everyone's opinion is just as valid
    return 0;
}
/*virtual*/ int Subsystem::Guts::calcUUnitWeightsImpl(const State& s, Vector& weights) const {
    weights.resize(getNU(s));
    weights = 1;
    return 0;
}
/*virtual*/ int Subsystem::Guts::calcZUnitWeightsImpl(const State& s, Vector& weights) const {
    weights.resize(getNZ(s));
    weights = 1;
    return 0;
}
/*virtual*/ int Subsystem::Guts::calcQErrUnitTolerancesImpl(const State& s, Vector& tolerances) const {
    tolerances.resize(getNQErr(s));
    tolerances = 1;
    return 0;
}
/*virtual*/ int Subsystem::Guts::calcUErrUnitTolerancesImpl(const State& s, Vector& tolerances) const {
    tolerances.resize(getNUErr(s));
    tolerances = 1;
    return 0;
}
/*virtual*/ int Subsystem::Guts::calcDecorativeGeometryAndAppendImpl
                                (const State&, Stage, Array<DecorativeGeometry>&) const
{
    return 0;
}
void Subsystem::Guts::handleEvents(State&, System::EventCause, const Array<int>& eventIds,
    Real accuracy, const Vector& yWeights, const Vector& ooConstraintTols,
    Stage& lowestModified, bool& shouldTerminate) const
{
    SimTK_THROW2(Exception::UnimplementedVirtualMethod, "Subsystem", "handleEvents"); 
}
void Subsystem::Guts::calcEventTriggerInfo(const State& s, Array<System::EventTriggerInfo>& info) const {
}
void Subsystem::Guts::calcTimeOfNextScheduledEvent(const State&, Real& tNextEvent, Array<int>& eventIds) const {
    tNextEvent = Infinity;
    eventIds.clear();
}

    ///////////////////
    // SUBSYSTEM REP //
    ///////////////////

void Subsystem::Guts::GutsRep::invalidateSubsystemTopologyCache() const {
    subsystemTopologyRealized = false;
    if (isInSystem()) 
        getSystem().getSystemGuts().invalidateSystemTopologyCache();
}

    //////////////////////////////
    // DEFAULT SYSTEM SUBSYSTEM //
    //////////////////////////////

class DefaultSystemSubsystemGuts : public Subsystem::Guts {
public:
    /**
     * This class stores various information used by the default subsystem that needs to be stored in the state.
     */

    class CacheInfo {
    public:
        CacheInfo() : eventIdCounter(0) {}
        mutable int eventIdCounter;
        mutable std::map<int, SubsystemId> eventOwnerMap;
        Array<int> scheduledEventIds;
        Array<int> triggeredEventIndices;
        Array<int> triggeredEventIds;
        Array<int> scheduledReportIds;
        Array<int> triggeredReportIndices;
        Array<int> triggeredReportIds;
    };
    DefaultSystemSubsystemGuts() : Guts("DefaultSystemSubsystemGuts", "0.0.1") { }
    DefaultSystemSubsystemGuts* cloneImpl() const {
        return new DefaultSystemSubsystemGuts(*this);
    }
        
    const List<ScheduledEventHandler>& getScheduledEventHandlers() const {
        return scheduledEventHandlers;
    }
    
    List<ScheduledEventHandler>& updScheduledEventHandlers() {
        return scheduledEventHandlers;
    }
    
    const List<TriggeredEventHandler>& getTriggeredEventHandlers() const {
        return triggeredEventHandlers;
    }
    
    List<TriggeredEventHandler>& updTriggeredEventHandlers() {
        return triggeredEventHandlers;
    }
    
    const List<ScheduledEventReporter>& getScheduledEventReporters() const {
        return scheduledEventReporters;
    }
    
    List<ScheduledEventReporter>& updScheduledEventReporters() const {
        return scheduledEventReporters;
    }
    
    const List<TriggeredEventReporter>& getTriggeredEventReporters() const {
        return triggeredEventReporters;
    }
    
    List<TriggeredEventReporter>& updTriggeredEventReporters() const {
        return triggeredEventReporters;
    }
    
    const CacheInfo& getCacheInfo(const State& s) const {
        return Value<CacheInfo>::downcast(getCacheEntry(s, cacheInfoIndex)).get();
    }
    
    CacheInfo& updCacheInfo(State& s) const {
        return Value<CacheInfo>::downcast(updCacheEntry(s, cacheInfoIndex)).upd();
    }

    int realizeSubsystemTopologyImpl(State& s) const {
        cacheInfoIndex = s.allocateCacheEntry(getMySubsystemId(), Stage::Topology, new Value<CacheInfo>());
        return 0;
    }
    
    int realizeSubsystemModelImpl(State& s) const {
        CacheInfo& info = updCacheInfo(s);
        info.scheduledEventIds.clear();
        info.triggeredEventIndices.clear();
        info.triggeredEventIds.clear();
        info.scheduledReportIds.clear();
        info.triggeredReportIndices.clear();
        info.triggeredReportIds.clear();
        if (scheduledEventHandlers.size() > 0)
            for (List<ScheduledEventHandler>::const_iterator e = scheduledEventHandlers.begin(); e != scheduledEventHandlers.end(); e++) {
                int id;
                createScheduledEvent(s, id);
                info.scheduledEventIds += id;
            }
        if (triggeredEventHandlers.size() > 0)
            for (List<TriggeredEventHandler>::const_iterator e = triggeredEventHandlers.begin(); e != triggeredEventHandlers.end(); e++) {
                int id, index;
                createTriggeredEvent(s, id, index, (*e)->getRequiredStage());
                info.triggeredEventIds += id;
                info.triggeredEventIndices += index;
            }
        if (scheduledEventReporters.size() > 0)
            for (List<ScheduledEventReporter>::const_iterator e = scheduledEventReporters.begin(); e != scheduledEventReporters.end(); e++) {
                int id;
                createScheduledEvent(s, id);
                info.scheduledReportIds += id;
            }
        if (triggeredEventReporters.size() > 0)
            for (List<TriggeredEventReporter>::const_iterator e = triggeredEventReporters.begin(); e != triggeredEventReporters.end(); e++) {
                int id, index;
                createTriggeredEvent(s, id, index, (*e)->getRequiredStage());
                info.triggeredReportIds += id;
                info.triggeredReportIndices += index;
            }
        return 0;
    }

    int realizeEvents(const State& s, Stage g) const {
        const CacheInfo& info = getCacheInfo(s);
        Vector& events = s.updEventsByStage(getMySubsystemId(), g);
        for (int i = 0; i < triggeredEventHandlers.size(); ++i) {
            if (g == triggeredEventHandlers[i]->getRequiredStage())
                events[info.triggeredEventIndices[i]] = triggeredEventHandlers[i]->getValue(s);
        }
        for (int i = 0; i < triggeredEventReporters.size(); ++i) {
            if (g == triggeredEventReporters[i]->getRequiredStage())
                events[info.triggeredReportIndices[i]] = triggeredEventReporters[i]->getValue(s);
        }
        return 0;
    }
    
    int realizeSubsystemInstanceImpl(const State& s) const {
        return realizeEvents(s, Stage::Instance);
    }
    int realizeSubsystemTimeImpl(const State& s) const {
        return realizeEvents(s, Stage::Time);
    }
    int realizeSubsystemPositionImpl(const State& s) const {
        return realizeEvents(s, Stage::Position);
    }
    int realizeSubsystemVelocityImpl(const State& s) const {
        return realizeEvents(s, Stage::Velocity);
    }
    int realizeSubsystemDynamicsImpl(const State& s) const {
        return realizeEvents(s, Stage::Dynamics);
    }
    int realizeSubsystemAccelerationImpl(const State& s) const {
        return realizeEvents(s, Stage::Acceleration);
    }
    int realizeSubsystemReportImpl(const State& s) const {
        return realizeEvents(s, Stage::Report);
    }
    void calcEventTriggerInfo(const State& s, Array<System::EventTriggerInfo>& triggers) const {
        
        // Loop over all registered TriggeredEventHandlers and TriggeredEventReporters, and ask
        // each one for its EventTriggerInfo.
        
        const CacheInfo& info = getCacheInfo(s);
        for (int i = 0; i < triggeredEventHandlers.size(); ++i) {
            triggers += triggeredEventHandlers[i]->getTriggerInfo();
            triggers[triggers.size()-1].setEventId(info.triggeredEventIds[i]);
        }
        for (int i = 0; i < triggeredEventReporters.size(); ++i) {
            triggers += triggeredEventReporters[i]->getTriggerInfo();
            triggers[triggers.size()-1].setEventId(info.triggeredReportIds[i]);
        }
    }
    void calcTimeOfNextScheduledEvent(const State& s, Real& tNextEvent, Array<int>& eventIds) const {
        
        // Loop over all registered ScheduledEventHandlers and ScheduledEventReporters, and ask
        // each one when its next event occurs.
        
        const CacheInfo& info = getCacheInfo(s);
        tNextEvent = Infinity;
        for (int i = 0; i < scheduledEventHandlers.size(); ++i) {
            Real time = scheduledEventHandlers[i]->getNextEventTime(s);
            if (time <= tNextEvent && time >= s.getTime()) {
                if (time < tNextEvent)
                    eventIds.clear();
                tNextEvent = time;
                eventIds.push_back(info.scheduledEventIds[i]);
            }
        }
        for (int i = 0; i < scheduledEventReporters.size(); ++i) {
            Real time = scheduledEventReporters[i]->getNextEventTime(s);
            if (time <= tNextEvent && time >= s.getTime()) {
                if (time < tNextEvent)
                    eventIds.clear();
                tNextEvent = time;
                eventIds.push_back(info.scheduledReportIds[i]);
            }
        }
    }
    void handleEvents(State& s, System::EventCause cause, const Array<int>& eventIds, Real accuracy, const Vector& yWeights,
            const Vector& ooConstraintTols, Stage& lowestModified, bool& shouldTerminate) const {
        const CacheInfo& info = getCacheInfo(s);
        lowestModified = Stage::HighestValid;
        shouldTerminate = false;
        
        // Build a set of the ids for quick lookup.
        
        std::set<int> idSet;
        for (int i = 0; i < eventIds.size(); ++i)
            idSet.insert(eventIds[i]);
        
        // Process triggered events.
        
        if (cause == System::TriggeredEvents) {
            for (int i = 0; i < triggeredEventHandlers.size(); ++i) {
                if (idSet.find(info.triggeredEventIds[i]) != idSet.end()) {
                    Stage eventLowestModified = Stage::HighestValid;
                    bool eventShouldTerminate = false;
                    triggeredEventHandlers[i]->handleEvent(s, accuracy, yWeights, ooConstraintTols, eventLowestModified, eventShouldTerminate);
                    if (eventLowestModified < lowestModified)
                        lowestModified = eventLowestModified;
                    if (eventShouldTerminate)
                        shouldTerminate = true;
                }
            }
            for (int i = 0; i < triggeredEventReporters.size(); ++i) {
                if (idSet.find(info.triggeredReportIds[i]) != idSet.end())
                    triggeredEventReporters[i]->handleEvent(s);
            }
        }
        
        // Process scheduled events.
        
        if (cause == System::ScheduledEvents) {
            for (int i = 0; i < scheduledEventHandlers.size(); ++i) {
                if (idSet.find(info.scheduledEventIds[i]) != idSet.end()) {
                    Stage eventLowestModified = Stage::HighestValid;
                    bool eventShouldTerminate = false;
                    scheduledEventHandlers[i]->handleEvent(s, accuracy, yWeights, ooConstraintTols, eventLowestModified, eventShouldTerminate);
                    if (eventLowestModified < lowestModified)
                        lowestModified = eventLowestModified;
                    if (eventShouldTerminate)
                        shouldTerminate = true;
                }
            }
            for (int i = 0; i < scheduledEventReporters.size(); ++i) {
                if (idSet.find(info.scheduledReportIds[i]) != idSet.end())
                    scheduledEventReporters[i]->handleEvent(s);
            }
        }
    }

private:
    mutable int cacheInfoIndex;
    mutable List<ScheduledEventHandler> scheduledEventHandlers;
    mutable List<TriggeredEventHandler> triggeredEventHandlers;
    mutable List<ScheduledEventReporter> scheduledEventReporters;
    mutable List<TriggeredEventReporter> triggeredEventReporters;
};

std::ostream& operator<<(std::ostream& o, const DefaultSystemSubsystemGuts::CacheInfo& info) {
    o << "DefaultSystemSubsystemGuts::CacheInfo";
    return o;
}

DefaultSystemSubsystem::DefaultSystemSubsystem(System& sys) {
    adoptSubsystemGuts(new DefaultSystemSubsystemGuts());
    sys.adoptSubsystem(*this);
}

const DefaultSystemSubsystemGuts& DefaultSystemSubsystem::getGuts() const {
    return dynamic_cast<const DefaultSystemSubsystemGuts&>(getSubsystemGuts());
}

DefaultSystemSubsystemGuts& DefaultSystemSubsystem::updGuts() {
    return dynamic_cast<DefaultSystemSubsystemGuts&>(updSubsystemGuts());
}

/**
 * Add a ScheduledEventHandler to the System.  This must be called before the Model stage is realized.
 */

void DefaultSystemSubsystem::addEventHandler(const ScheduledEventHandler& handler) {
    updGuts().updScheduledEventHandlers().push_back(handler);
}

/**
 * Add a TriggeredEventHandler to the System.  This must be called before the Model stage is realized.
 */

void DefaultSystemSubsystem::addEventHandler(const TriggeredEventHandler& handler) {
    updGuts().updTriggeredEventHandlers().push_back(handler);
}

/**
 * Add a ScheduledEventReporter to the System.  This must be called before the Model stage is realized.
 * 
 * Note that this method is const.  Because an EventReporter cannot affect the behavior of the system
 * being simulated, it is permitted to add one to a const System.
 */

void DefaultSystemSubsystem::addEventReporter(const ScheduledEventReporter& handler) const {
    getGuts().updScheduledEventReporters().push_back(handler);
}

/**
 * Add a TriggeredEventReporter to the System.  This must be called before the Model stage is realized.
 * 
 * Note that this method is const.  Because an EventReporter cannot affect the behavior of the system
 * being simulated, it is permitted to add one to a const System.
 */

void DefaultSystemSubsystem::addEventReporter(const TriggeredEventReporter& handler) const {
    getGuts().updTriggeredEventReporters().push_back(handler);
}

/**
 * Generate a new, globally unique event ID.  Typically you will not call this directly.  When a Subsystem
 * needs to obtain an event ID for an event it defines, it should do so by calling
 * Subsystem::Guts::createScheduledEvent() or Subsystem::Guts::createTriggeredEvent().
 */

int DefaultSystemSubsystem::createEventId(SubsystemId subsys, State& state) const {
    const DefaultSystemSubsystemGuts::CacheInfo& info = getGuts().getCacheInfo(state);
    int id = info.eventIdCounter++;
    info.eventOwnerMap[id] = subsys;
    return id;
}

/**
 * Given a list of event IDs, filter it to produce a list of those events belonging to a particular Subsystem.
 * 
 * @param subsys       the Subsystem for which to find events
 * @param state        the State which produced the events
 * @param allEvents    a list of event IDs to filter
 * @param eventsForSubsystem    on exit, this Array contains the filtered list of event IDs belonging to the
 *                              specified Subsystem.
 */

void DefaultSystemSubsystem::findSubsystemEventIds(SubsystemId subsys, const State& state, const Array<int>& allEvents, Array<int>& eventsForSubsystem) const {
    const DefaultSystemSubsystemGuts::CacheInfo& info = getGuts().getCacheInfo(state);
    eventsForSubsystem.clear();
    for (int i = 0; i < allEvents.size(); ++i) {
        if (info.eventOwnerMap[allEvents[i]] == subsys)
            eventsForSubsystem += allEvents[i];
    }
}

} // namespace SimTK

