#ifndef MODELS_HPP
#define MODELS_HPP

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <set>

// Forward declarations
class Course;
class Teacher;
class Section;
class TimeSlot;
class Requirement;
class Schedule;

// Class representing a time slot
class TimeSlot {
public:
    enum Day { MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, UNASSIGNED };
    
    // Constructor with optional day and start time (day is UNASSIGNED if not provided)
    TimeSlot(int durationMinutes, Day day = UNASSIGNED, int startHour = -1, int startMinute = -1);
    
    Day getDay() const;
    int getStartHour() const;
    int getStartMinute() const;
    int getDurationMinutes() const;
    std::string toString() const;
    
    bool overlaps(const TimeSlot& other) const;
    bool hasStartTime() const;
    bool hasDay() const;
    
    // Create a complete timeslot with day and start time from a partial one
    std::shared_ptr<TimeSlot> withStartTime(int startHour, int startMinute) const;
    std::shared_ptr<TimeSlot> withDay(Day day) const;
    std::shared_ptr<TimeSlot> withDayAndTime(Day day, int startHour, int startMinute) const;
    
private:
    Day day;
    int startHour;
    int startMinute;
    int durationMinutes;
};

// Class representing a teacher
class Teacher {
public:
    Teacher(const std::string& id, const std::string& name);
    
    std::string getId() const;
    std::string getName() const;
    const std::vector<std::shared_ptr<Course>>& getCourses() const;
    
    void addCourse(std::shared_ptr<Course> course);
    void removeCourse(std::shared_ptr<Course> course);
    
private:
    std::string id;
    std::string name;
    std::vector<std::shared_ptr<Course>> courses;
};

// Class representing a course
class Course {
public:
    Course(const std::string& code, const std::string& name, int credits);
    
    std::string getCode() const;
    std::string getName() const;
    int getCredits() const;
    
    const std::vector<std::shared_ptr<Section>>& getSections() const;
    
    void addSection(std::shared_ptr<Section> section);
    void removeSection(std::shared_ptr<Section> section);
    
private:
    std::string code;
    std::string name;
    int credits;
    std::vector<std::shared_ptr<Section>> sections;
};

// Class representing a section of a course
class Section {
public:
    // Constructor with optional pre-assigned time
    Section(const std::string& id, std::shared_ptr<Course> course, 
            std::shared_ptr<Teacher> teacher, std::shared_ptr<TimeSlot> timeSlot);
    
    std::string getId() const;
    std::shared_ptr<Course> getCourse() const;
    std::shared_ptr<Teacher> getTeacher() const;
    std::shared_ptr<TimeSlot> getTimeSlot() const;
    
    void setTeacher(std::shared_ptr<Teacher> teacher);
    void setTimeSlot(std::shared_ptr<TimeSlot> timeSlot);
    
    // Create a new section with an assigned start time
    std::shared_ptr<Section> withStartTime(int startHour, int startMinute) const;
    
private:
    std::string id;
    std::shared_ptr<Course> course;
    std::shared_ptr<Teacher> teacher;
    std::shared_ptr<TimeSlot> timeSlot;
};

// Base class for requirements
class Requirement {
public:
    virtual ~Requirement() = default;
    virtual bool isSatisfied(const Schedule& schedule) const = 0;
    virtual std::string getDescription() const = 0;
};

// Specific time slot requirement
class TimeSlotRequirement : public Requirement {
public:
    TimeSlotRequirement(std::shared_ptr<Course> course, std::shared_ptr<TimeSlot> timeSlot);
    bool isSatisfied(const Schedule& schedule) const override;
    std::string getDescription() const override;
    
private:
    std::shared_ptr<Course> course;
    std::shared_ptr<TimeSlot> timeSlot;
};

// Teacher preference requirement
class TeacherRequirement : public Requirement {
public:
    TeacherRequirement(std::shared_ptr<Course> course, std::shared_ptr<Teacher> teacher);
    bool isSatisfied(const Schedule& schedule) const override;
    std::string getDescription() const override;
    
private:
    std::shared_ptr<Course> course;
    std::shared_ptr<Teacher> teacher;
};

// Section-specific time slot requirement
class SectionTimeSlotRequirement : public Requirement {
public:
    SectionTimeSlotRequirement(std::shared_ptr<Section> section, std::shared_ptr<TimeSlot> timeSlot);
    bool isSatisfied(const Schedule& schedule) const override;
    std::string getDescription() const override;
    
    std::shared_ptr<Section> getSection() const { return section; }
    std::shared_ptr<TimeSlot> getTimeSlot() const { return timeSlot; }
    
private:
    std::shared_ptr<Section> section;
    std::shared_ptr<TimeSlot> timeSlot;
};

// Class representing a complete schedule
class Schedule {
public:
    Schedule();
    
    void addSection(std::shared_ptr<Section> section);
    void removeSection(std::shared_ptr<Section> section);
    
    const std::vector<std::shared_ptr<Section>>& getSections() const;
    std::vector<std::shared_ptr<Section>> getSectionsForCourse(const std::string& courseCode) const;
    
    bool hasConflicts() const;
    
private:
    std::vector<std::shared_ptr<Section>> sections;
};

#endif // MODELS_HPP 