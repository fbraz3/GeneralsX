/*
**  Command & Conquer Generals Zero Hour(tm)
**  Copyright 2025 Electronic Arts Inc.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
**  Phase 47: Objective Tracking System
**  
**  Purpose: Track mission objectives during campaign gameplay
**  - Display active objectives
**  - Track objective completion status
**  - Notify player of objective changes
**  - Persist objective state
*/

#ifndef OBJECTIVETRACKER_H
#define OBJECTIVETRACKER_H

#include "Lib/BaseType.h"
#include "Common/Snapshot.h"
#include "Common/AsciiString.h"
#include <list>

enum ObjectiveStatus {
    OBJECTIVE_INACTIVE = 0,
    OBJECTIVE_ACTIVE = 1,
    OBJECTIVE_COMPLETED = 2,
    OBJECTIVE_FAILED = 3
};

typedef struct {
    AsciiString identifier;
    AsciiString displayString;
    ObjectiveStatus status;
    int priority;
    Bool critical;
} ObjectiveInfo;

class ObjectiveTracker : public Snapshot {
public:
    ObjectiveTracker();
    ~ObjectiveTracker();
    
    /* Snapshot interface */
    virtual void crc(Xfer *xfer) {}
    virtual void xfer(Xfer *xfer);
    virtual void loadPostProcess() {}
    
    /* Objective management */
    void addObjective(const AsciiString &id, const AsciiString &displayStr, int priority, Bool critical);
    void removeObjective(const AsciiString &id);
    void setObjectiveStatus(const AsciiString &id, ObjectiveStatus status);
    ObjectiveStatus getObjectiveStatus(const AsciiString &id) const;
    
    /* Query methods */
    Bool isObjectiveComplete(const AsciiString &id) const;
    Bool isObjectiveFailed(const AsciiString &id) const;
    Bool isObjectiveActive(const AsciiString &id) const;
    int getObjectiveCount() const;
    int getCompletedObjectiveCount() const;
    int getFailedObjectiveCount() const;
    int getActiveObjectiveCount() const;
    
    /* Get all objectives */
    const std::list<ObjectiveInfo> &getAllObjectives() const { return m_objectives; }
    
    /* Clear all objectives */
    void clearAllObjectives();
    
    /* Check if all objectives completed */
    Bool allObjectivesCompleted() const;
    Bool anyObjectivesFailed() const;
    
    /* Utility */
    void printObjectiveStatus() const;

private:
    typedef std::list<ObjectiveInfo> ObjectiveList;
    typedef ObjectiveList::iterator ObjectiveListIt;
    typedef ObjectiveList::const_iterator ObjectiveListConstIt;
    
    ObjectiveList m_objectives;
    
    /* Helper methods */
    ObjectiveListIt findObjective(const AsciiString &id);
    ObjectiveListConstIt findObjective(const AsciiString &id) const;
};

extern ObjectiveTracker *TheObjectiveTracker;

#endif /* OBJECTIVETRACKER_H */
