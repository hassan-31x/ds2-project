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
    
    // Build the PQ tree from the course and section data
    buildPQTree();
    
    // Apply the PQ tree operations to generate schedules
    extractSchedulesFromPQTree();
    
    // Find a schedule that satisfies all requirements
    return findSatisfyingSchedule();
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

// Helper method to convert courses and sections to a PQ tree representation
void Scheduler::buildPQTree() {
    // Create a new PQ tree
    pqTree = PQTree();
    
    // Create a root P-node for the entire schedule
    auto rootNode = pqTree.createPNode("Schedule");
    pqTree.setRoot(rootNode);
    
    // Create P-nodes for each course
    for (const auto& course : courses) {
        auto courseNode = pqTree.createPNode(course->getCode());
        rootNode->addChild(courseNode);
        
        // For each course, create Q-nodes for sections
        // These are represented as a Q-node because we can only reverse the sections, not reorder them arbitrarily
        auto sectionsNode = pqTree.createQNode("Sections_" + course->getCode());
        courseNode->addChild(sectionsNode);
        
        // Add leaf nodes for each section
        for (const auto& section : course->getSections()) {
            auto sectionLeaf = pqTree.createLeaf(section->getId());
            sectionsNode->addChild(sectionLeaf);
        }
    }
    
    // Layout the tree for visualization
    pqTree.computeLayout();
}

// Helper method to create actual schedules from the PQ tree layout
void Scheduler::extractSchedulesFromPQTree() {
    // For a simple implementation, we'll create a random number of schedules
    // A real implementation would use more advanced algorithms
    const int numSchedulesToGenerate = 5;
    
    for (int i = 0; i < numSchedulesToGenerate; ++i) {
        // Create a new schedule
        auto schedule = std::make_shared<Schedule>();
        
        // Apply some random reordering of the PQ tree
        pqTree.reorder();
        
        // Now extract a schedule based on the tree's current state
        // This is a simplified version; a real implementation would look at the tree's structure
        // and determine the actual schedule from it
        
        // For each course, add one section to the schedule
        for (const auto& course : courses) {
            // Get all sections for this course
            const auto& sections = course->getSections();
            
            if (!sections.empty()) {
                // Just add a random section for now
                std::random_device rd;
                std::mt19937 g(rd());
                std::uniform_int_distribution<size_t> dist(0, sections.size() - 1);
                
                size_t sectionIndex = dist(g);
                schedule->addSection(sections[sectionIndex]);
            }
        }
        
        // If the schedule has no conflicts, add it to the possible schedules
        if (!schedule->hasConflicts()) {
            possibleSchedules.push_back(schedule);
        }
    }
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