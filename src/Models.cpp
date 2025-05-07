#include "Models.hpp"
#include <sstream>
#include <algorithm>
#include <iomanip>

// TimeSlot implementation
TimeSlot::TimeSlot(int durationMinutes, Day day, int startHour, int startMinute)
    : day(day), startHour(startHour), startMinute(startMinute), durationMinutes(durationMinutes) {}

TimeSlot::Day TimeSlot::getDay() const {
    return day;
}

int TimeSlot::getStartHour() const {
    return startHour;
}

int TimeSlot::getStartMinute() const {
    return startMinute;
}

int TimeSlot::getDurationMinutes() const {
    return durationMinutes;
}

bool TimeSlot::hasStartTime() const {
    return startHour >= 0 && startMinute >= 0;
}

bool TimeSlot::hasDay() const {
    return day != UNASSIGNED;
}

std::shared_ptr<TimeSlot> TimeSlot::withStartTime(int startHour, int startMinute) const {
    return std::make_shared<TimeSlot>(durationMinutes, day, startHour, startMinute);
}

std::shared_ptr<TimeSlot> TimeSlot::withDay(Day day) const {
    return std::make_shared<TimeSlot>(durationMinutes, day, startHour, startMinute);
}

std::shared_ptr<TimeSlot> TimeSlot::withDayAndTime(Day day, int startHour, int startMinute) const {
    return std::make_shared<TimeSlot>(durationMinutes, day, startHour, startMinute);
}

std::string TimeSlot::toString() const {
    // If day isn't assigned, just show the duration
    if (!hasDay()) {
        return "Unassigned (" + std::to_string(durationMinutes) + " min)";
    }
    
    const char* dayNames[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "?"};
    std::stringstream ss;
    ss << dayNames[static_cast<int>(day)];
    
    // If no start time is set, just show the duration
    if (!hasStartTime()) {
        ss << " (" << durationMinutes << " min)";
        return ss.str();
    }
    
    // Format hours in 12-hour format
    int hour12 = (startHour > 12) ? (startHour - 12) : startHour;
    if (hour12 == 0) hour12 = 12;
    
    // Calculate end time
    int endMinute = (startMinute + durationMinutes) % 60;
    int endHour = startHour + (startMinute + durationMinutes) / 60;
    
    // Format end time in 12-hour format
    int endHour12 = (endHour > 12) ? (endHour - 12) : endHour;
    if (endHour12 == 0) endHour12 = 12;
    
    // Create a format similar to "8-9 AM" or "1-2 PM"
    ss << " " << hour12;
    
    // Only include minutes if they're not zero
    if (startMinute > 0) {
        ss << ":" << std::setw(2) << std::setfill('0') << startMinute;
    }
    
    ss << "-" << endHour12;
    
    // Only include minutes if they're not zero
    if (endMinute > 0) {
        ss << ":" << std::setw(2) << std::setfill('0') << endMinute;
    }
    
    // AM/PM marker
    ss << " " << (startHour < 12 ? "AM" : "PM");
    
    return ss.str();
}

bool TimeSlot::overlaps(const TimeSlot& other) const {
    // If either timeslot doesn't have a day assigned, we can't determine overlap
    if (!hasDay() || !other.hasDay()) {
        return false;
    }
    
    // If days are different, no overlap
    if (day != other.day) return false;
    
    // If either timeslot doesn't have a start time, we can't determine overlap
    if (!hasStartTime() || !other.hasStartTime()) {
        return false;
    }
    
    int thisStartTotalMinutes = startHour * 60 + startMinute;
    int thisEndTotalMinutes = thisStartTotalMinutes + durationMinutes;
    
    int otherStartTotalMinutes = other.startHour * 60 + other.startMinute;
    int otherEndTotalMinutes = otherStartTotalMinutes + other.durationMinutes;
    
    return (thisStartTotalMinutes < otherEndTotalMinutes && thisEndTotalMinutes > otherStartTotalMinutes);
}

// Teacher implementation
Teacher::Teacher(const std::string& id, const std::string& name)
    : id(id), name(name) {}

std::string Teacher::getId() const {
    return id;
}

std::string Teacher::getName() const {
    return name;
}

const std::vector<std::shared_ptr<Course>>& Teacher::getCourses() const {
    return courses;
}

void Teacher::addCourse(std::shared_ptr<Course> course) {
    if (std::find(courses.begin(), courses.end(), course) == courses.end()) {
        courses.push_back(course);
    }
}

void Teacher::removeCourse(std::shared_ptr<Course> course) {
    courses.erase(std::remove(courses.begin(), courses.end(), course), courses.end());
}

// Course implementation
Course::Course(const std::string& code, const std::string& name, int credits)
    : code(code), name(name), credits(credits) {}

std::string Course::getCode() const {
    return code;
}

std::string Course::getName() const {
    return name;
}

int Course::getCredits() const {
    return credits;
}

const std::vector<std::shared_ptr<Section>>& Course::getSections() const {
    return sections;
}

void Course::addSection(std::shared_ptr<Section> section) {
    if (std::find(sections.begin(), sections.end(), section) == sections.end()) {
        sections.push_back(section);
    }
}

void Course::removeSection(std::shared_ptr<Section> section) {
    sections.erase(std::remove(sections.begin(), sections.end(), section), sections.end());
}

// Section implementation
Section::Section(const std::string& id, std::shared_ptr<Course> course, 
                 std::shared_ptr<Teacher> teacher, std::shared_ptr<TimeSlot> timeSlot)
    : id(id), course(course), teacher(teacher), timeSlot(timeSlot) {}

std::string Section::getId() const {
    return id;
}

std::shared_ptr<Course> Section::getCourse() const {
    return course;
}

std::shared_ptr<Teacher> Section::getTeacher() const {
    return teacher;
}

std::shared_ptr<TimeSlot> Section::getTimeSlot() const {
    return timeSlot;
}

void Section::setTeacher(std::shared_ptr<Teacher> teacher) {
    this->teacher = teacher;
}

void Section::setTimeSlot(std::shared_ptr<TimeSlot> timeSlot) {
    this->timeSlot = timeSlot;
}

std::shared_ptr<Section> Section::withStartTime(int startHour, int startMinute) const {
    // Create a new TimeSlot with the assigned start time
    auto newTimeSlot = timeSlot->withStartTime(startHour, startMinute);
    
    // Create a new Section with the new TimeSlot
    return std::make_shared<Section>(id, course, teacher, newTimeSlot);
}

// TimeSlotRequirement implementation
TimeSlotRequirement::TimeSlotRequirement(std::shared_ptr<Course> course, std::shared_ptr<TimeSlot> timeSlot)
    : course(course), timeSlot(timeSlot) {}

bool TimeSlotRequirement::isSatisfied(const Schedule& schedule) const {
    auto sections = schedule.getSectionsForCourse(course->getCode());
    for (const auto& section : sections) {
        auto sectionTimeSlot = section->getTimeSlot();
        auto requiredTimeSlot = timeSlot;
        
        // Check if requirement specifies a day
        if (requiredTimeSlot->hasDay()) {
            // If section doesn't have a day assigned or has a different day, it doesn't match
            if (!sectionTimeSlot->hasDay() || sectionTimeSlot->getDay() != requiredTimeSlot->getDay()) {
                continue;
            }
        }
        
        // Check if requirement specifies a start time
        if (requiredTimeSlot->hasStartTime()) {
            // If section doesn't have start time assigned or has a different start time, it doesn't match
            if (!sectionTimeSlot->hasStartTime() || 
                sectionTimeSlot->getStartHour() != requiredTimeSlot->getStartHour() ||
                sectionTimeSlot->getStartMinute() != requiredTimeSlot->getStartMinute()) {
                continue;
            }
        }
        
        // If we get here, the section satisfies the requirement
        return true;
    }
    return false;
}

std::string TimeSlotRequirement::getDescription() const {
    return "Course " + course->getCode() + " must be in time slot " + timeSlot->toString();
}

// TeacherRequirement implementation
TeacherRequirement::TeacherRequirement(std::shared_ptr<Course> course, std::shared_ptr<Teacher> teacher)
    : course(course), teacher(teacher) {}

bool TeacherRequirement::isSatisfied(const Schedule& schedule) const {
    auto sections = schedule.getSectionsForCourse(course->getCode());
    for (const auto& section : sections) {
        if (section->getTeacher()->getId() == teacher->getId()) {
            return true;
        }
    }
    return false;
}

std::string TeacherRequirement::getDescription() const {
    return "Course " + course->getCode() + " must be taught by " + teacher->getName();
}

// SectionTimeSlotRequirement implementation
SectionTimeSlotRequirement::SectionTimeSlotRequirement(std::shared_ptr<Section> section, std::shared_ptr<TimeSlot> timeSlot)
    : section(section), timeSlot(timeSlot) {}

bool SectionTimeSlotRequirement::isSatisfied(const Schedule& schedule) const {
    for (const auto& scheduleSection : schedule.getSections()) {
        // Find the matching section by ID
        if (scheduleSection->getId() == section->getId()) {
            auto sectionTimeSlot = scheduleSection->getTimeSlot();
            
            // Check if requirement specifies a day
            if (timeSlot->hasDay()) {
                // If section doesn't have a day assigned or has a different day, it doesn't match
                if (!sectionTimeSlot->hasDay() || sectionTimeSlot->getDay() != timeSlot->getDay()) {
                    return false;
                }
            }
            
            // Check if requirement specifies a start time
            if (timeSlot->hasStartTime()) {
                // If section doesn't have start time assigned or has a different start time, it doesn't match
                if (!sectionTimeSlot->hasStartTime() || 
                    sectionTimeSlot->getStartHour() != timeSlot->getStartHour() ||
                    sectionTimeSlot->getStartMinute() != timeSlot->getStartMinute()) {
                    return false;
                }
            }
            
            // If we get here, the section satisfies the requirement
            return true;
        }
    }
    
    // Section not found in schedule
    return false;
}

std::string SectionTimeSlotRequirement::getDescription() const {
    return "Section " + section->getId() + " must be in time slot " + timeSlot->toString();
}

// Schedule implementation
Schedule::Schedule() {}

void Schedule::addSection(std::shared_ptr<Section> section) {
    if (std::find(sections.begin(), sections.end(), section) == sections.end()) {
        sections.push_back(section);
    }
}

void Schedule::removeSection(std::shared_ptr<Section> section) {
    sections.erase(std::remove(sections.begin(), sections.end(), section), sections.end());
}

const std::vector<std::shared_ptr<Section>>& Schedule::getSections() const {
    return sections;
}

std::vector<std::shared_ptr<Section>> Schedule::getSectionsForCourse(const std::string& courseCode) const {
    std::vector<std::shared_ptr<Section>> result;
    for (const auto& section : sections) {
        if (section->getCourse()->getCode() == courseCode) {
            result.push_back(section);
        }
    }
    return result;
}

bool Schedule::hasConflicts() const {
    for (size_t i = 0; i < sections.size(); ++i) {
        for (size_t j = i + 1; j < sections.size(); ++j) {
            // Check if two sections with the same teacher are assigned to the same time
            if (sections[i]->getTeacher()->getId() == sections[j]->getTeacher()->getId()) {
                // If both sections have assigned days and times, check for overlap
                if (sections[i]->getTimeSlot()->hasDay() && sections[i]->getTimeSlot()->hasStartTime() &&
                    sections[j]->getTimeSlot()->hasDay() && sections[j]->getTimeSlot()->hasStartTime()) {
                    if (sections[i]->getTimeSlot()->overlaps(*(sections[j]->getTimeSlot()))) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
} 