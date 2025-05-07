#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "PQTree.hpp"
#include "Models.hpp"
#include <vector>
#include <memory>
#include <map>

class Scheduler {
public:
    Scheduler();
    
    // Add data to the scheduler
    void addCourse(std::shared_ptr<Course> course);
    void addTeacher(std::shared_ptr<Teacher> teacher);
    void addSection(std::shared_ptr<Section> section);
    
    // Add requirements/constraints
    void addRequirement(std::shared_ptr<Requirement> requirement);
    
    // Generate and get schedules
    bool generateSchedule();
    std::shared_ptr<Schedule> getCurrentSchedule() const;
    std::vector<std::shared_ptr<Schedule>> getAllPossibleSchedules() const;
    
    // Build a PQ tree for the current schedule (for visualization)
    PQTree buildSchedulePQTree() const;
    
    // Clear all data
    void clear();
    
    // Get data
    const std::vector<std::shared_ptr<Course>>& getCourses() const;
    const std::vector<std::shared_ptr<Teacher>>& getTeachers() const;
    const std::vector<std::shared_ptr<Section>>& getSections() const;
    const std::vector<std::shared_ptr<Requirement>>& getRequirements() const;
    
private:
    std::vector<std::shared_ptr<Course>> courses;
    std::vector<std::shared_ptr<Teacher>> teachers;
    std::vector<std::shared_ptr<Section>> sections;
    std::vector<std::shared_ptr<Requirement>> requirements;
    
    // The current generated schedule
    std::shared_ptr<Schedule> currentSchedule;
    
    // All possible schedules generated
    std::vector<std::shared_ptr<Schedule>> possibleSchedules;
    
    // Helper method to find a schedule that satisfies all requirements
    bool findSatisfyingSchedule();
    
    // Helper method to generate all combinations of sections (one per course)
    void generateCourseSelections(
        const std::map<std::string, std::vector<std::shared_ptr<Section>>>& sectionsByCourse,
        std::vector<std::shared_ptr<Section>> current,
        std::vector<std::vector<std::shared_ptr<Section>>>& result);
        
    // Helper method to schedule sections based on a PQ Tree
    bool scheduleSections(std::shared_ptr<PQNode> permutationTree);
        
    // Helper method to create a schedule with assigned start times
    Schedule tryCreateScheduleWithTimes(const std::vector<int>& permutation);
        
    // Helper to check if two schedules are equivalent (have same sections)
    bool areSchedulesEquivalent(const Schedule& a, const Schedule& b) const;
};

#endif // SCHEDULER_HPP 