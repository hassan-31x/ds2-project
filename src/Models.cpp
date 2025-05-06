#include "Models.hpp"
#include <sstream>
#include <algorithm>

// TimeSlot implementation
TimeSlot::TimeSlot(Day day, int startHour, int startMinute, int durationMinutes)
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

std::string TimeSlot::toString() const {
    const char* dayNames[] = {"Mon", "Tue", "Wed", "Thu", "Fri"};
    std::stringstream ss;
    ss << dayNames[static_cast<int>(day)] << " ";
    
    // Format hours in 12-hour format
    int hour12 = (startHour > 12) ? (startHour - 12) : startHour;
    if (hour12 == 0) hour12 = 12;
    
    ss << hour12 << ":";
    if (startMinute < 10) ss << "0";
    ss << startMinute;
    
    ss << (startHour >= 12 ? " PM" : " AM");
    
    // Calculate end time
    int endMinute = (startMinute + durationMinutes) % 60;
    int endHour = startHour + (startMinute + durationMinutes) / 60;
    
    ss << " - ";
    
    // Format end time
    int endHour12 = (endHour > 12) ? (endHour - 12) : endHour;
    if (endHour12 == 0) endHour12 = 12;
    
    ss << endHour12 << ":";
    if (endMinute < 10) ss << "0";
    ss << endMinute;
    
    ss << (endHour >= 12 ? " PM" : " AM");
    
    return ss.str();
}

bool TimeSlot::overlaps(const TimeSlot& other) const {
    if (day != other.day) return false;
    
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

// TimeSlotRequirement implementation
TimeSlotRequirement::TimeSlotRequirement(std::shared_ptr<Course> course, std::shared_ptr<TimeSlot> timeSlot)
    : course(course), timeSlot(timeSlot) {}

bool TimeSlotRequirement::isSatisfied(const Schedule& schedule) const {
    auto sections = schedule.getSectionsForCourse(course->getCode());
    for (const auto& section : sections) {
        if (section->getTimeSlot()->getDay() == timeSlot->getDay() &&
            section->getTimeSlot()->getStartHour() == timeSlot->getStartHour() &&
            section->getTimeSlot()->getStartMinute() == timeSlot->getStartMinute()) {
            return true;
        }
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
            if (sections[i]->getTimeSlot()->overlaps(*(sections[j]->getTimeSlot()))) {
                return true;
            }
        }
    }
    return false;
} 