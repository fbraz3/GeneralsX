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

#include "GameClient/ObjectiveTracker.h"
#include "Common/Xfer.h"
#include "Common/Debug.h"
#include <stdio.h>

ObjectiveTracker *TheObjectiveTracker = NULL;

ObjectiveTracker::ObjectiveTracker()
{
    printf("Phase 47: ObjectiveTracker created\n");
}

ObjectiveTracker::~ObjectiveTracker()
{
    clearAllObjectives();
    printf("Phase 47: ObjectiveTracker destroyed\n");
}

void ObjectiveTracker::xfer(Xfer *xfer)
{
    if (!xfer) return;
    
    const XferVersion currentVersion = 1;
    XferVersion version = currentVersion;
    xfer->xferVersion(&version, currentVersion);
    
    int objectiveCount = m_objectives.size();
    xfer->xferInt(&objectiveCount);
    
    if (xfer->getXferMode() == XFER_LOAD) {
        clearAllObjectives();
        for (int i = 0; i < objectiveCount; i++) {
            ObjectiveInfo objective;
            int status;
            xfer->xferAsciiString(&objective.identifier);
            xfer->xferAsciiString(&objective.displayString);
            xfer->xferInt(&status);
            objective.status = (ObjectiveStatus)status;
            xfer->xferInt(&objective.priority);
            Bool critical_bool = FALSE;
            xfer->xferBool(&critical_bool);
            objective.critical = critical_bool;
            m_objectives.push_back(objective);
        }
    } else {
        for (ObjectiveListIt it = m_objectives.begin(); it != m_objectives.end(); ++it) {
            int status = (int)it->status;
            Bool critical_bool = it->critical ? TRUE : FALSE;
            xfer->xferAsciiString(&it->identifier);
            xfer->xferAsciiString(&it->displayString);
            xfer->xferInt(&status);
            xfer->xferInt(&it->priority);
            xfer->xferBool(&critical_bool);
        }
    }
    
    printf("Phase 47: ObjectiveTracker xfer complete (%d objectives)\n", objectiveCount);
}

void ObjectiveTracker::addObjective(const AsciiString &id, const AsciiString &displayStr, int priority, Bool critical)
{
    if (id.isEmpty()) {
        printf("Phase 47: ObjectiveTracker - Cannot add objective with empty ID\n");
        return;
    }
    
    if (findObjective(id) != m_objectives.end()) {
        printf("Phase 47: ObjectiveTracker - Objective already exists: %s\n", id.str());
        return;
    }
    
    ObjectiveInfo obj;
    obj.identifier = id;
    obj.displayString = displayStr;
    obj.status = OBJECTIVE_ACTIVE;
    obj.priority = priority;
    obj.critical = critical;
    
    m_objectives.push_back(obj);
    printf("Phase 47: ObjectiveTracker - Added objective: %s\n", id.str());
}

void ObjectiveTracker::removeObjective(const AsciiString &id)
{
    ObjectiveListIt it = findObjective(id);
    if (it != m_objectives.end()) {
        m_objectives.erase(it);
        printf("Phase 47: ObjectiveTracker - Removed objective: %s\n", id.str());
    }
}

void ObjectiveTracker::setObjectiveStatus(const AsciiString &id, ObjectiveStatus status)
{
    ObjectiveListIt it = findObjective(id);
    if (it != m_objectives.end()) {
        it->status = status;
        const char *statusStr = "";
        switch (status) {
            case OBJECTIVE_INACTIVE: statusStr = "INACTIVE"; break;
            case OBJECTIVE_ACTIVE: statusStr = "ACTIVE"; break;
            case OBJECTIVE_COMPLETED: statusStr = "COMPLETED"; break;
            case OBJECTIVE_FAILED: statusStr = "FAILED"; break;
        }
        printf("Phase 47: ObjectiveTracker - Objective status changed: %s -> %s\n", id.str(), statusStr);
    }
}

ObjectiveStatus ObjectiveTracker::getObjectiveStatus(const AsciiString &id) const
{
    ObjectiveListConstIt it = findObjective(id);
    if (it != m_objectives.end()) {
        return it->status;
    }
    return OBJECTIVE_INACTIVE;
}

Bool ObjectiveTracker::isObjectiveComplete(const AsciiString &id) const
{
    return getObjectiveStatus(id) == OBJECTIVE_COMPLETED;
}

Bool ObjectiveTracker::isObjectiveFailed(const AsciiString &id) const
{
    return getObjectiveStatus(id) == OBJECTIVE_FAILED;
}

Bool ObjectiveTracker::isObjectiveActive(const AsciiString &id) const
{
    return getObjectiveStatus(id) == OBJECTIVE_ACTIVE;
}

int ObjectiveTracker::getObjectiveCount() const
{
    return m_objectives.size();
}

int ObjectiveTracker::getCompletedObjectiveCount() const
{
    int count = 0;
    for (ObjectiveListConstIt it = m_objectives.begin(); it != m_objectives.end(); ++it) {
        if (it->status == OBJECTIVE_COMPLETED) {
            count++;
        }
    }
    return count;
}

int ObjectiveTracker::getFailedObjectiveCount() const
{
    int count = 0;
    for (ObjectiveListConstIt it = m_objectives.begin(); it != m_objectives.end(); ++it) {
        if (it->status == OBJECTIVE_FAILED) {
            count++;
        }
    }
    return count;
}

int ObjectiveTracker::getActiveObjectiveCount() const
{
    int count = 0;
    for (ObjectiveListConstIt it = m_objectives.begin(); it != m_objectives.end(); ++it) {
        if (it->status == OBJECTIVE_ACTIVE) {
            count++;
        }
    }
    return count;
}

void ObjectiveTracker::clearAllObjectives()
{
    m_objectives.clear();
    printf("Phase 47: ObjectiveTracker - All objectives cleared\n");
}

Bool ObjectiveTracker::allObjectivesCompleted() const
{
    if (m_objectives.empty()) {
        return FALSE;
    }
    
    for (ObjectiveListConstIt it = m_objectives.begin(); it != m_objectives.end(); ++it) {
        if (it->status != OBJECTIVE_COMPLETED) {
            return FALSE;
        }
    }
    return TRUE;
}

Bool ObjectiveTracker::anyObjectivesFailed() const
{
    for (ObjectiveListConstIt it = m_objectives.begin(); it != m_objectives.end(); ++it) {
        if (it->status == OBJECTIVE_FAILED) {
            return TRUE;
        }
    }
    return FALSE;
}

void ObjectiveTracker::printObjectiveStatus() const
{
    printf("Phase 47: ObjectiveTracker Status Report\n");
    printf("  Total objectives: %d\n", (int)m_objectives.size());
    printf("  Active: %d\n", getActiveObjectiveCount());
    printf("  Completed: %d\n", getCompletedObjectiveCount());
    printf("  Failed: %d\n", getFailedObjectiveCount());
    
    int idx = 0;
    for (ObjectiveListConstIt it = m_objectives.begin(); it != m_objectives.end(); ++it) {
        const char *statusStr = "";
        switch (it->status) {
            case OBJECTIVE_INACTIVE: statusStr = "INACTIVE"; break;
            case OBJECTIVE_ACTIVE: statusStr = "ACTIVE"; break;
            case OBJECTIVE_COMPLETED: statusStr = "COMPLETED"; break;
            case OBJECTIVE_FAILED: statusStr = "FAILED"; break;
        }
        printf("  [%d] %s (%s) - Priority: %d, Critical: %s\n", 
               idx++, it->identifier.str(), statusStr, it->priority,
               it->critical ? "YES" : "NO");
    }
}

ObjectiveTracker::ObjectiveListIt ObjectiveTracker::findObjective(const AsciiString &id)
{
    for (ObjectiveListIt it = m_objectives.begin(); it != m_objectives.end(); ++it) {
        if (it->identifier.compare(id) == 0) {
            return it;
        }
    }
    return m_objectives.end();
}

ObjectiveTracker::ObjectiveListConstIt ObjectiveTracker::findObjective(const AsciiString &id) const
{
    for (ObjectiveListConstIt it = m_objectives.begin(); it != m_objectives.end(); ++it) {
        if (it->identifier.compare(id) == 0) {
            return it;
        }
    }
    return m_objectives.end();
}
