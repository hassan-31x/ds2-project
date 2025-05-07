#include "Scheduler.hpp"
#include <algorithm>
#include <map>
#include <set>
#include <random>

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

bool Scheduler::generateSchedule() {
    // Clear any existing schedules
    possibleSchedules.clear();
    currentSchedule = nullptr;
    
    // Organize sections by day
    std::map<TimeSlot::Day, std::vector<std::shared_ptr<Section>>> sectionsByDay;
    for (const auto& section : sections) {
        sectionsByDay[section->getTimeSlot()->getDay()].push_back(section);
    }
    
    // Generate all possible permutations for each day using PQ tree
    std::map<TimeSlot::Day, std::vector<std::vector<std::shared_ptr<Section>>>> permutationsByDay;
    
    for (const auto& pair : sectionsByDay) {
        TimeSlot::Day day = pair.first;
        const auto& daySections = pair.second;
        
        if (!daySections.empty()) {
            // Create a PQ tree for this day's sections
            PQTree tree;
            tree.buildTimeOrderedTree(daySections);
            
            // Get all valid permutations of sections for this day
            auto frontiers = tree.getFrontiers();
            
            // Convert string frontiers back to section permutations
            std::vector<std::vector<std::shared_ptr<Section>>> dayPermutations;
            
            // Map to quickly look up sections by their label
            std::map<std::string, std::shared_ptr<Section>> sectionsByLabel;
            for (const auto& section : daySections) {
                std::string label = section->getCourse()->getCode() + " (" + 
                                section->getTeacher()->getName() + ", " +
                                section->getTimeSlot()->toString() + ")";
                sectionsByLabel[label] = section;
            }
            
            // Create section permutations from string permutations
            for (const auto& frontier : frontiers) {
                std::vector<std::shared_ptr<Section>> perm;
                for (const auto& label : frontier) {
                    // Extract section ID from label if it's a leaf
                    if (label.find("Leaf:") == 0) {
                        std::string actualLabel = label.substr(6); // Skip "Leaf: "
                        if (sectionsByLabel.find(actualLabel) != sectionsByLabel.end()) {
                            perm.push_back(sectionsByLabel[actualLabel]);
                        }
                    } else if (sectionsByLabel.find(label) != sectionsByLabel.end()) {
                        perm.push_back(sectionsByLabel[label]);
                    }
                }
                if (!perm.empty()) {
                    dayPermutations.push_back(perm);
                }
            }
            
            permutationsByDay[day] = dayPermutations;
        }
    }
    
    // Generate all possible schedules from combinations of daily permutations
    generateAllScheduleCombinations(permutationsByDay);
    
    // Find a schedule that satisfies all requirements
    return findSatisfyingSchedule();
}

void Scheduler::generateAllScheduleCombinations(
    const std::map<TimeSlot::Day, std::vector<std::vector<std::shared_ptr<Section>>>>& permutationsByDay) {
    
    // Create a vector of day/permutation pairs for easier processing
    std::vector<std::pair<TimeSlot::Day, std::vector<std::vector<std::shared_ptr<Section>>>>> dayPermList;
    for (const auto& pair : permutationsByDay) {
        dayPermList.push_back(pair);
    }
    
    // Use a recursive helper to generate all combinations
    std::shared_ptr<Schedule> currentBuildingSchedule = std::make_shared<Schedule>();
    generateScheduleRecursive(dayPermList, 0, currentBuildingSchedule);
    
    // Remove duplicate and conflicting schedules
    std::vector<std::shared_ptr<Schedule>> filteredSchedules;
    for (const auto& schedule : possibleSchedules) {
        if (!schedule->hasConflicts()) {
            filteredSchedules.push_back(schedule);
        }
    }
    
    possibleSchedules = filteredSchedules;
}

void Scheduler::generateScheduleRecursive(
    const std::vector<std::pair<TimeSlot::Day, std::vector<std::vector<std::shared_ptr<Section>>>>>& dayPermList,
    size_t dayIndex,
    std::shared_ptr<Schedule> currentSchedule) {
    
    // Base case: we've processed all days
    if (dayIndex >= dayPermList.size()) {
        // Make a copy of the final schedule
        auto finalSchedule = std::make_shared<Schedule>();
        for (const auto& section : currentSchedule->getSections()) {
            finalSchedule->addSection(section);
        }
        
        // Check if this schedule covers all courses
        std::set<std::string> coveredCourses;
        for (const auto& section : finalSchedule->getSections()) {
            coveredCourses.insert(section->getCourse()->getCode());
        }
        
        // Only add the schedule if it covers all courses
        if (coveredCourses.size() == courses.size()) {
            possibleSchedules.push_back(finalSchedule);
        }
        
        return;
    }
    
    // Get the current day's permutations
    const auto& currentDayPerms = dayPermList[dayIndex].second;
    
    // For each permutation of the current day
    for (const auto& perm : currentDayPerms) {
        // Try this permutation
        auto tempSchedule = std::make_shared<Schedule>();
        
        // Copy existing schedule
        for (const auto& section : currentSchedule->getSections()) {
            tempSchedule->addSection(section);
        }
        
        // Add this day's sections
        for (const auto& section : perm) {
            // Check if this course is already in the schedule
            std::string courseCode = section->getCourse()->getCode();
            auto existing = tempSchedule->getSectionsForCourse(courseCode);
            if (existing.empty()) {
                tempSchedule->addSection(section);
            }
        }
        
        // Recursively process the next day
        generateScheduleRecursive(dayPermList, dayIndex + 1, tempSchedule);
    }
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
        
        // For each day, create a Q-node with sections
        for (const auto& pair : sectionsByDay) {
            const auto& day = pair.first;
            const auto& daySections = pair.second;
            
            // Create a node for the day
            const char* dayNames[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};
            auto dayNode = tree.createQNode(dayNames[static_cast<int>(day)]);
            rootNode->addChild(dayNode);
            
            // Sort sections by time
            std::vector<std::shared_ptr<Section>> sortedSections = daySections;
            std::sort(sortedSections.begin(), sortedSections.end(),
                [](const std::shared_ptr<Section>& a, const std::shared_ptr<Section>& b) {
                    return a->getTimeSlot()->getStartHour() < b->getTimeSlot()->getStartHour() ||
                           (a->getTimeSlot()->getStartHour() == b->getTimeSlot()->getStartHour() &&
                            a->getTimeSlot()->getStartMinute() < b->getTimeSlot()->getStartMinute());
                });
            
            // Add section leaves
            for (const auto& section : sortedSections) {
                std::string label = section->getCourse()->getCode() + " (" + 
                                   section->getTeacher()->getName() + ")";
                auto leaf = tree.createLeaf(label);
                dayNode->addChild(leaf);
            }
        }
        
        // Compute layout for visualization
        tree.computeLayout();
    }
    
    return tree;
} 