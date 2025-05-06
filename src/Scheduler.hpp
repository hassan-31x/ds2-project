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
    
    // PQ tree used for generating schedules
    PQTree pqTree;
    
    // Helper method to convert courses and sections to a PQ tree representation
    void buildPQTree();
    
    // Helper method to create actual schedules from the PQ tree layout
    void extractSchedulesFromPQTree();
    
    // Helper method to find a schedule that satisfies all requirements
    bool findSatisfyingSchedule();
};

#endif // SCHEDULER_HPP 