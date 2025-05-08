#include "Scheduler.hpp"
#include <algorithm>
#include <map>
#include <set>
#include <random>
#include <iostream>

Scheduler::Scheduler() {
    clear();
}

void Scheduler::addCourse(std::shared_ptr<Course> course) {
    if (std::find(courses.begin(), courses.end(), course) == courses.end()) {
        courses.push_back(course);
    }
}

void Scheduler::addTeacher(std::shared_ptr<Teacher> teacher) {
    if (std::find(teachers.begin(), teachers.end(), teacher) == teachers.end()) {
        teachers.push_back(teacher);
    }
}

void Scheduler::addSection(std::shared_ptr<Section> section) {
    if (std::find(sections.begin(), sections.end(), section) == sections.end()) {
        sections.push_back(section);
        
        // Add the section to its course
        section->getCourse()->addSection(section);
        
        // Add the course to the teacher's list of courses
        section->getTeacher()->addCourse(section->getCourse());
    }
}

void Scheduler::addRequirement(std::shared_ptr<Requirement> requirement) {
    if (std::find(requirements.begin(), requirements.end(), requirement) == requirements.end()) {
        requirements.push_back(requirement);
    }
}

void Scheduler::removeRequirement(std::shared_ptr<Requirement> requirement) {
    auto it = std::find(requirements.begin(), requirements.end(), requirement);
    if (it != requirements.end()) {
        requirements.erase(it);
    }
}

bool Scheduler::generateSchedule() {
    // Clear any existing schedules
    possibleSchedules.clear();
    currentSchedule = nullptr;
    
    // Instead of grouping sections by course, use all sections
    std::vector<std::shared_ptr<Section>> allSections = sections;
    
    // Create a PQ tree for time ordering
    PQTree tree;
    tree.buildTimeOrderedTree(allSections);
    
    // Get all valid permutations
    auto permutations = tree.getFrontiers();
    
    // For each permutation, try to assign start times
    for (const auto& permutation : permutations) {
        // Convert string permutation to section indices
        std::vector<int> sectionIndices;
        for (const auto& label : permutation) {
            // Find matching section
            for (size_t i = 0; i < allSections.size(); i++) {
                std::string sectionLabel = allSections[i]->getCourse()->getCode() + " (" + 
                                         allSections[i]->getTeacher()->getName() + ", " +
                                         allSections[i]->getTimeSlot()->toString() + ")";
                
                // Also check with "Leaf: " prefix (as PQ Tree might add this)
                if (label == sectionLabel || label == "Leaf: " + sectionLabel) {
                    sectionIndices.push_back(i);
                    break;
                }
            }
        }
        
        // Skip if we couldn't map all labels to sections
        if (sectionIndices.size() != permutation.size()) {
            continue;
        }
        
        // Create base schedule
        auto baseSchedule = tryCreateScheduleWithTimes(sectionIndices);
        if (baseSchedule.getSections().size() > 0) {
            // The base schedule is valid, so add it
            possibleSchedules.push_back(std::make_shared<Schedule>(baseSchedule));
            
            // Now create variations for sections without requirements
            createScheduleVariations(baseSchedule);
        }
    }
    
    // Debug output
    std::cout << "Generated " << possibleSchedules.size() << " valid schedules." << std::endl;
    
    // Print a summary of each schedule
    int idx = 1;
    for (const auto& schedule : possibleSchedules) {
        std::cout << "Schedule " << idx++ << ":\n";
        for (const auto& section : schedule->getSections()) {
            std::cout << "  " << section->getCourse()->getCode() 
                     << " (" << section->getTeacher()->getName() 
                     << ", " << section->getTimeSlot()->toString() << ")\n";
        }
        std::cout << std::endl;
    }
    
    // Find a schedule that satisfies all requirements
    return findSatisfyingSchedule();
}

// Helper method to generate all combinations of sections (one per course)
void Scheduler::generateCourseSelections(
    const std::map<std::string, std::vector<std::shared_ptr<Section>>>& sectionsByCourse,
    std::vector<std::shared_ptr<Section>> current,
    std::vector<std::vector<std::shared_ptr<Section>>>& result) {
    
    // If we've processed all courses, add this combination to the result
    if (current.size() == sectionsByCourse.size()) {
        result.push_back(current);
        return;
    }
    
    // Get the next course to process
    auto it = sectionsByCourse.begin();
    std::advance(it, current.size());
    
    // Try each section for this course
    for (const auto& section : it->second) {
        auto newCurrent = current;
        newCurrent.push_back(section);
        generateCourseSelections(sectionsByCourse, newCurrent, result);
    }
}

// Helper method to try creating a schedule with assigned start times
Schedule Scheduler::tryCreateScheduleWithTimes(const std::vector<int>& permutation) {
    Schedule schedule;
    std::map<std::string, std::shared_ptr<Section>> sectionsById;

    // Populate the map for quick section lookup
    for (const auto& section : sections) {
        sectionsById[section->getId()] = section;
    }

    std::vector<std::shared_ptr<Section>> selectedSections;
    for (int idx : permutation) {
        selectedSections.push_back(sections[idx]);
    }

    // First, identify sections that have specific time requirements
    std::vector<std::shared_ptr<Section>> sectionsWithRequirements;
    std::vector<std::shared_ptr<Section>> sectionsWithoutRequirements;

    // Track the latest end time for each day to avoid conflicts
    std::map<TimeSlot::Day, int> latestEndTimeByDay;
    for (auto day : {TimeSlot::MONDAY, TimeSlot::TUESDAY, TimeSlot::WEDNESDAY, TimeSlot::THURSDAY, TimeSlot::FRIDAY}) {
        latestEndTimeByDay[day] = 8 * 60; // Start at 8:00 AM
    }

    // Check requirements and separate sections that have specific time requirements
    for (const auto& section : selectedSections) {
        bool hasSpecificRequirement = false;
        
        for (const auto& req : requirements) {
            // Check if this is a SectionTimeSlotRequirement for this section
            auto sectionReq = std::dynamic_pointer_cast<SectionTimeSlotRequirement>(req);
            if (sectionReq && sectionReq->getSection()->getId() == section->getId()) {
                sectionsWithRequirements.push_back(section);
                hasSpecificRequirement = true;
                break;
            }
        }
        
        if (!hasSpecificRequirement) {
            sectionsWithoutRequirements.push_back(section);
        }
    }

    // First, schedule sections with specific time requirements
    for (const auto& section : sectionsWithRequirements) {
        // Find the matching requirement for this section
        for (const auto& req : requirements) {
            auto sectionReq = std::dynamic_pointer_cast<SectionTimeSlotRequirement>(req);
            if (sectionReq && sectionReq->getSection()->getId() == section->getId()) {
                auto reqTimeSlot = sectionReq->getTimeSlot();
                
                // Get the required day and time
                TimeSlot::Day day = reqTimeSlot->getDay();
                int startTime = reqTimeSlot->getStartHour() * 60 + reqTimeSlot->getStartMinute();
                int duration = section->getTimeSlot()->getDurationMinutes();
                int endTime = startTime + duration;
                
                // Update the latest end time for this day
                latestEndTimeByDay[day] = std::max(latestEndTimeByDay[day], endTime);
                
                // Create a new TimeSlot with the required day and time
                auto timeSlot = std::make_shared<TimeSlot>(
                    section->getTimeSlot()->getDurationMinutes(),
                    day, 
                    reqTimeSlot->getStartHour(), 
                    reqTimeSlot->getStartMinute()
                );
                
                // Create a new section with the assigned time slot
                auto scheduleSection = std::make_shared<Section>(
                    section->getId(),
                    section->getCourse(),
                    section->getTeacher(),
                    timeSlot
                );
                
                schedule.addSection(scheduleSection);
                break;
            }
        }
    }

    // Now schedule the remaining sections without specific requirements
    // Sort by duration (longest first) for better packing
    std::sort(sectionsWithoutRequirements.begin(), sectionsWithoutRequirements.end(),
        [](const auto& a, const auto& b) {
            return a->getTimeSlot()->getDurationMinutes() > b->getTimeSlot()->getDurationMinutes();
        }
    );

    for (const auto& section : sectionsWithoutRequirements) {
        // Find the day with the earliest end time (to balance the schedule)
        TimeSlot::Day selectedDay = TimeSlot::MONDAY;
        int earliestEndTime = INT_MAX;
        
        for (auto day : {TimeSlot::MONDAY, TimeSlot::TUESDAY, TimeSlot::WEDNESDAY, TimeSlot::THURSDAY, TimeSlot::FRIDAY}) {
            if (latestEndTimeByDay[day] < earliestEndTime) {
                earliestEndTime = latestEndTimeByDay[day];
                selectedDay = day;
            }
        }
        
        // Assign this section to the day with the earliest end time
        int startTime = latestEndTimeByDay[selectedDay];
        int duration = section->getTimeSlot()->getDurationMinutes();
        int endTime = startTime + duration;
        
        // Update the latest end time for this day
        latestEndTimeByDay[selectedDay] = endTime;
        
        // Calculate hours and minutes from start time
        int startHour = startTime / 60;
        int startMinute = startTime % 60;
        
        // Create a new TimeSlot with the assigned day and time
        auto timeSlot = std::make_shared<TimeSlot>(
            section->getTimeSlot()->getDurationMinutes(),
            selectedDay,
            startHour,
            startMinute
        );
        
        // Create a new section with the assigned time slot
        auto scheduleSection = std::make_shared<Section>(
            section->getId(),
            section->getCourse(),
            section->getTeacher(),
            timeSlot
        );
        
        schedule.addSection(scheduleSection);
    }
    
    // Check if the schedule has conflicts
    if (schedule.hasConflicts()) {
        // Return an empty schedule if there are conflicts
        return Schedule();
    }
    
    return schedule;
}

std::shared_ptr<Schedule> Scheduler::getCurrentSchedule() const {
    return currentSchedule;
}

std::vector<std::shared_ptr<Schedule>> Scheduler::getAllPossibleSchedules() const {
    return possibleSchedules;
}

void Scheduler::clear() {
    courses.clear();
    teachers.clear();
    sections.clear();
    requirements.clear();
    possibleSchedules.clear();
    currentSchedule = nullptr;
}

const std::vector<std::shared_ptr<Course>>& Scheduler::getCourses() const {
    return courses;
}

const std::vector<std::shared_ptr<Teacher>>& Scheduler::getTeachers() const {
    return teachers;
}

const std::vector<std::shared_ptr<Section>>& Scheduler::getSections() const {
    return sections;
}

const std::vector<std::shared_ptr<Requirement>>& Scheduler::getRequirements() const {
    return requirements;
}

// Helper method to find a schedule that satisfies all requirements
bool Scheduler::findSatisfyingSchedule() {
    // If no schedules were generated, return false
    if (possibleSchedules.empty()) {
        return false;
    }
    
    // Try to find a schedule that satisfies all requirements
    for (const auto& schedule : possibleSchedules) {
        bool satisfiesAll = true;
        
        for (const auto& requirement : requirements) {
            if (!requirement->isSatisfied(*schedule)) {
                satisfiesAll = false;
                break;
            }
        }
        
        if (satisfiesAll) {
            currentSchedule = schedule;
            return true;
        }
    }
    
    // If no schedule satisfies all requirements, just pick the first one
    if (!possibleSchedules.empty()) {
        currentSchedule = possibleSchedules[0];
    }
    
    return false;
}

// Helper to build a PQ tree for visualization of the current schedule
PQTree Scheduler::buildSchedulePQTree() const {
    PQTree tree;
    
    if (currentSchedule) {
        // Group sections by day
        std::map<TimeSlot::Day, std::vector<std::shared_ptr<Section>>> sectionsByDay;
        for (const auto& section : currentSchedule->getSections()) {
            sectionsByDay[section->getTimeSlot()->getDay()].push_back(section);
        }
        
        // Create root P-node
        auto rootNode = tree.createPNode("Schedule");
        tree.setRoot(rootNode);
        
        // Group days by the day value for cleaner naming
        std::map<int, std::string> dayNames = {
            {static_cast<int>(TimeSlot::MONDAY), "Monday"},
            {static_cast<int>(TimeSlot::TUESDAY), "Tuesday"},
            {static_cast<int>(TimeSlot::WEDNESDAY), "Wednesday"},
            {static_cast<int>(TimeSlot::THURSDAY), "Thursday"},
            {static_cast<int>(TimeSlot::FRIDAY), "Friday"}
        };
        
        // For each day, create a Q-node with sections
        for (const auto& pair : sectionsByDay) {
            TimeSlot::Day day = pair.first;
            const auto& sections = pair.second;
            
            if (!sections.empty()) {
                // Create a node for the day
                auto dayNode = tree.createQNode(dayNames[static_cast<int>(day)]);
                rootNode->addChild(dayNode);
                
                // Sort sections by time
                std::vector<std::shared_ptr<Section>> sortedSections = sections;
                std::sort(sortedSections.begin(), sortedSections.end(),
                    [](const std::shared_ptr<Section>& a, const std::shared_ptr<Section>& b) {
                        // Only sort by time if both have start times
                        if (a->getTimeSlot()->hasStartTime() && b->getTimeSlot()->hasStartTime()) {
                            return a->getTimeSlot()->getStartHour() < b->getTimeSlot()->getStartHour() ||
                                   (a->getTimeSlot()->getStartHour() == b->getTimeSlot()->getStartHour() &&
                                    a->getTimeSlot()->getStartMinute() < b->getTimeSlot()->getStartMinute());
                        }
                        return false;
                    });
                
                // Add section leaves
                for (const auto& section : sortedSections) {
                    // Format the label similar to the sample code
                    std::string label = section->getCourse()->getCode() + " (" + 
                                        section->getTeacher()->getName() + ", " +
                                        section->getTimeSlot()->toString() + ")";
                    
                    auto leaf = tree.createLeaf(label);
                    dayNode->addChild(leaf);
                }
            }
        }
        
        // Compute layout for visualization
        tree.computeLayout();
    }
    
    return tree;
}

// Helper to build a PQ tree for visualization of a specific schedule by index
PQTree Scheduler::buildSchedulePQTreeForIndex(int scheduleIndex) const {
    PQTree tree;
    
    // Check if the index is valid
    if (scheduleIndex >= 0 && scheduleIndex < static_cast<int>(possibleSchedules.size())) {
        auto schedule = possibleSchedules[scheduleIndex];
        
        // Group sections by day
        std::map<TimeSlot::Day, std::vector<std::shared_ptr<Section>>> sectionsByDay;
        for (const auto& section : schedule->getSections()) {
            sectionsByDay[section->getTimeSlot()->getDay()].push_back(section);
        }
        
        // Create root P-node
        auto rootNode = tree.createPNode("Schedule #" + std::to_string(scheduleIndex + 1));
        tree.setRoot(rootNode);
        
        // Group days by the day value for cleaner naming
        std::map<int, std::string> dayNames = {
            {static_cast<int>(TimeSlot::MONDAY), "Monday"},
            {static_cast<int>(TimeSlot::TUESDAY), "Tuesday"},
            {static_cast<int>(TimeSlot::WEDNESDAY), "Wednesday"},
            {static_cast<int>(TimeSlot::THURSDAY), "Thursday"},
            {static_cast<int>(TimeSlot::FRIDAY), "Friday"}
        };
        
        // For each day, create a Q-node with sections
        for (const auto& pair : sectionsByDay) {
            TimeSlot::Day day = pair.first;
            const auto& sections = pair.second;
            
            if (!sections.empty()) {
                // Create a node for the day
                auto dayNode = tree.createQNode(dayNames[static_cast<int>(day)]);
                rootNode->addChild(dayNode);
                
                // Sort sections by time
                std::vector<std::shared_ptr<Section>> sortedSections = sections;
                std::sort(sortedSections.begin(), sortedSections.end(),
                    [](const std::shared_ptr<Section>& a, const std::shared_ptr<Section>& b) {
                        // Only sort by time if both have start times
                        if (a->getTimeSlot()->hasStartTime() && b->getTimeSlot()->hasStartTime()) {
                            return a->getTimeSlot()->getStartHour() < b->getTimeSlot()->getStartHour() ||
                                   (a->getTimeSlot()->getStartHour() == b->getTimeSlot()->getStartHour() &&
                                    a->getTimeSlot()->getStartMinute() < b->getTimeSlot()->getStartMinute());
                        }
                        return false;
                    });
                
                // Add section leaves
                for (const auto& section : sortedSections) {
                    // Format the label similar to the sample code
                    std::string label = section->getCourse()->getCode() + " (" + 
                                        section->getTeacher()->getName() + ", " +
                                        section->getTimeSlot()->toString() + ")";
                    
                    auto leaf = tree.createLeaf(label);
                    dayNode->addChild(leaf);
                }
            }
        }
        
        // Compute layout for visualization
        tree.computeLayout();
    }
    
    return tree;
}

// Helper to check if two schedules are equivalent (have same sections)
bool Scheduler::areSchedulesEquivalent(const Schedule& a, const Schedule& b) const {
    // First check if they have the same number of sections
    if (a.getSections().size() != b.getSections().size()) {
        return false;
    }
    
    // For each section in schedule a, find a matching section in schedule b
    for (const auto& sectionA : a.getSections()) {
        bool foundMatch = false;
        
        for (const auto& sectionB : b.getSections()) {
            // Check that the section has the same course
            if (sectionA->getCourse()->getCode() != sectionB->getCourse()->getCode()) {
                continue;
            }
            
            // Check that the section has the same teacher
            if (sectionA->getTeacher()->getId() != sectionB->getTeacher()->getId()) {
                continue;
            }
            
            // Check that the section has the same day
            if (sectionA->getTimeSlot()->getDay() != sectionB->getTimeSlot()->getDay()) {
                continue;
            }
            
            // Check that the section has the same start time
            if (sectionA->getTimeSlot()->getStartHour() != sectionB->getTimeSlot()->getStartHour() ||
                sectionA->getTimeSlot()->getStartMinute() != sectionB->getTimeSlot()->getStartMinute()) {
                continue;
            }
            
            // If we get here, sections match
            foundMatch = true;
            break;
        }
        
        // If we couldn't find a matching section, schedules are different
        if (!foundMatch) {
            return false;
        }
    }
    
    // All sections had matches, schedules are equivalent
    return true;
}

bool Scheduler::scheduleSections(std::shared_ptr<PQNode> permutationTree) {
    // Clear any existing schedules
    possibleSchedules.clear();
    
    // Create a PQ tree and set the provided node as root
    PQTree tree;
    tree.setRoot(permutationTree);
    
    // Get all the permutations from the tree
    std::vector<std::vector<int>> permutations;
    tree.getAllPermutations(permutations);
    
    std::cout << "Found " << permutations.size() << " possible permutations" << std::endl;
    
    // Try to create a schedule for each permutation
    for (const auto& permutation : permutations) {
        // Create the schedule with assigned times
        auto schedule = tryCreateScheduleWithTimes(permutation);
        
        // Check if a valid schedule was created
        if (!schedule.hasConflicts() && schedule.getSections().size() > 0) {
            // Check if we already have an equivalent schedule
            bool isDuplicate = false;
            for (const auto& existingSchedule : possibleSchedules) {
                if (areSchedulesEquivalent(schedule, *existingSchedule)) {
                    isDuplicate = true;
                    break;
                }
            }
            
            // Only add if it's not a duplicate
            if (!isDuplicate) {
                possibleSchedules.push_back(std::make_shared<Schedule>(schedule));
            }
        }
    }
    
    // Return true if at least one valid schedule was found
    return !possibleSchedules.empty();
}

// New helper method to create variations of schedules
void Scheduler::createScheduleVariations(const Schedule& baseSchedule) {
    // Get all sections from the base schedule
    auto baseSections = baseSchedule.getSections();
    
    // Identify sections without specific requirements
    std::vector<std::shared_ptr<Section>> sectionsWithoutRequirements;
    
    for (const auto& section : baseSections) {
        bool hasSpecificRequirement = false;
        
        for (const auto& req : requirements) {
            auto sectionReq = std::dynamic_pointer_cast<SectionTimeSlotRequirement>(req);
            if (sectionReq && sectionReq->getSection()->getId() == section->getId()) {
                hasSpecificRequirement = true;
                break;
            }
        }
        
        if (!hasSpecificRequirement) {
            sectionsWithoutRequirements.push_back(section);
        }
    }
    
    // If no sections without requirements, nothing to vary
    if (sectionsWithoutRequirements.empty()) {
        return;
    }
    
    // Create variations for each section without requirements
    for (const auto& flexibleSection : sectionsWithoutRequirements) {
        // Get current day and time
        auto currentTimeSlot = flexibleSection->getTimeSlot();
        TimeSlot::Day currentDay = currentTimeSlot->getDay();
        int currentStartHour = currentTimeSlot->getStartHour();
        int currentStartMinute = currentTimeSlot->getStartMinute();
        int duration = currentTimeSlot->getDurationMinutes();
        
        // Create 3 variations with different days/times
        for (int variant = 1; variant <= 3; variant++) {
            // Create a copy of the base schedule
            Schedule newSchedule = baseSchedule;
            
            // Define new day and time - cycle through days
            TimeSlot::Day newDay;
            int newStartHour, newStartMinute;
            
            switch (variant) {
                case 1:
                    // Move to next day, same time
                    newDay = static_cast<TimeSlot::Day>((static_cast<int>(currentDay) + 1) % 5);
                    newStartHour = currentStartHour;
                    newStartMinute = currentStartMinute;
                    break;
                    
                case 2:
                    // Move to 2 days ahead, morning
                    newDay = static_cast<TimeSlot::Day>((static_cast<int>(currentDay) + 2) % 5);
                    newStartHour = 9;  // Morning
                    newStartMinute = 0;
                    break;
                    
                case 3:
                    // Move to 3 days ahead, afternoon
                    newDay = static_cast<TimeSlot::Day>((static_cast<int>(currentDay) + 3) % 5);
                    newStartHour = 14; // Afternoon
                    newStartMinute = 0;
                    break;
            }
            
            // Create a new TimeSlot with the new day and time
            auto newTimeSlot = std::make_shared<TimeSlot>(
                duration,
                newDay,
                newStartHour,
                newStartMinute
            );
            
            // Create a new Section with the new TimeSlot
            auto newSection = std::make_shared<Section>(
                flexibleSection->getId(),
                flexibleSection->getCourse(),
                flexibleSection->getTeacher(),
                newTimeSlot
            );
            
            // Find the section to replace in the new schedule
            for (size_t i = 0; i < newSchedule.getSections().size(); i++) {
                if (newSchedule.getSections()[i]->getId() == flexibleSection->getId()) {
                    // Replace the section with the new one
                    newSchedule.removeSection(newSchedule.getSections()[i]);
                    newSchedule.addSection(newSection);
                    break;
                }
            }
            
            // Check if the new schedule has conflicts
            if (!newSchedule.hasConflicts()) {
                // Check if this is a duplicate of an existing schedule
                bool isDuplicate = false;
                for (const auto& existingSchedule : possibleSchedules) {
                    if (areSchedulesEquivalent(newSchedule, *existingSchedule)) {
                        isDuplicate = true;
                        break;
                    }
                }
                
                // Only add if not a duplicate
                if (!isDuplicate) {
                    possibleSchedules.push_back(std::make_shared<Schedule>(newSchedule));
                }
            }
        }
    }
} 