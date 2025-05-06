#include "scheduler.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <random>

// TimeSlot implementation
std::string TimeSlot::toString() const
{
    std::stringstream ss;
    ss << dayToString(day) << " " << hour << ":00-" << (hour + duration) << ":00";
    return ss.str();
}

bool TimeSlot::overlaps(const TimeSlot &other) const
{
    if (day != other.day)
        return false;

    int thisEnd = hour + duration;
    int otherEnd = other.hour + other.duration;

    return (hour < otherEnd && other.hour < thisEnd);
}

std::string TimeSlot::dayToString(int day)
{
    switch (day)
    {
    case 0:
        return "Monday";
    case 1:
        return "Tuesday";
    case 2:
        return "Wednesday";
    case 3:
        return "Thursday";
    case 4:
        return "Friday";
    default:
        return "Unknown";
    }
}

// Teacher implementation
Teacher::Teacher(const std::string &id, const std::string &name)
    : id(id), name(name) {}

void Teacher::addAvailableTimeSlot(const TimeSlot &slot)
{
    availableTimeSlots.push_back(slot);
}

// Course implementation
Course::Course(const std::string &code, const std::string &title, int credits)
    : code(code), title(title), creditHours(credits) {}

void Course::assignTeacher(std::shared_ptr<Teacher> teacher)
{
    assignedTeachers.push_back(teacher);
}

// Section implementation
Section::Section(const std::string &id, std::shared_ptr<Course> course)
    : id(id), course(course), teacher(nullptr) {}

void Section::assignTeacher(std::shared_ptr<Teacher> t)
{
    teacher = t;
}

void Section::addTimeSlot(const TimeSlot &slot)
{
    timeSlots.push_back(slot);
}

// ClassScheduler implementation
ClassScheduler::ClassScheduler() {}

ClassScheduler::~ClassScheduler()
{
    courses.clear();
    teachers.clear();
    sections.clear();
}

void ClassScheduler::addCourse(std::shared_ptr<Course> course)
{
    courses.push_back(course);
}

void ClassScheduler::addTeacher(std::shared_ptr<Teacher> teacher)
{
    teachers.push_back(teacher);
}

void ClassScheduler::addSection(std::shared_ptr<Section> section)
{
    sections.push_back(section);
}

void ClassScheduler::addPreference(const StudentPreference &preference)
{
    preferences.push_back(preference);
}

const std::vector<std::shared_ptr<Course>> &ClassScheduler::getCourses() const
{
    return courses;
}

const std::vector<std::shared_ptr<Teacher>> &ClassScheduler::getTeachers() const
{
    return teachers;
}

const std::vector<std::shared_ptr<Section>> &ClassScheduler::getSections() const
{
    return sections;
}

bool ClassScheduler::generateSchedule()
{
    // Clear any existing schedule
    for (auto &section : sections)
    {
        section->teacher = nullptr;
        section->timeSlots.clear();
    }

    // Generate all possible time slots
    std::vector<TimeSlot> allTimeSlots;
    for (int day = 0; day < 5; day++)
    {
        for (int hour = 8; hour <= 16; hour++)
        {
            TimeSlot slot;
            slot.day = day;
            slot.hour = hour;
            slot.duration = 1;
            allTimeSlots.push_back(slot);
        }
    }

    // Create universal set of elements for PQ tree
    std::vector<std::string> elements;
    for (const auto &slot : allTimeSlots)
    {
        std::stringstream ss;
        ss << "ts_" << slot.day << "_" << slot.hour;
        elements.push_back(ss.str());
    }

    scheduleTree.createFromUniversalSet(elements);

    // Apply constraints to PQ tree
    applyConstraints();

    // Get possible arrangements from the PQ tree
    auto arrangements = scheduleTree.getPossibleArrangements();
    if (arrangements.empty())
    {
        return false;
    }

    // Simple scheduling algorithm:
    // 1. For each section, find a suitable teacher
    // 2. For each section, find a suitable time slot

    // Create a random generator for shuffling
    std::random_device rd;
    std::mt19937 g(rd());

    // Shuffle sections to avoid bias
    std::vector<std::shared_ptr<Section>> shuffledSections = sections;
    std::shuffle(shuffledSections.begin(), shuffledSections.end(), g);

    for (auto &section : shuffledSections)
    {
        // Find a suitable teacher for this section
        std::vector<std::shared_ptr<Teacher>> suitableTeachers;
        for (auto &teacher : section->course->assignedTeachers)
        {
            // Check if this teacher is preferred or to be avoided
            bool teacherPreferred = false;
            bool teacherAvoided = false;

            for (const auto &pref : preferences)
            {
                if (pref.courseCode == section->course->code && pref.teacherId == teacher->id)
                {
                    if (pref.type == StudentPreference::PREFER_TEACHER)
                    {
                        teacherPreferred = true;
                    }
                    else if (pref.type == StudentPreference::AVOID_TEACHER)
                    {
                        teacherAvoided = true;
                    }
                }
            }

            if (!teacherAvoided || teacherPreferred)
            {
                suitableTeachers.push_back(teacher);
            }
        }

        // If no suitable teachers, try any assigned teacher
        if (suitableTeachers.empty() && !section->course->assignedTeachers.empty())
        {
            suitableTeachers = section->course->assignedTeachers;
        }

        // If still no suitable teachers, skip this section
        if (suitableTeachers.empty())
        {
            continue;
        }

        // Shuffle teachers to avoid bias
        std::shuffle(suitableTeachers.begin(), suitableTeachers.end(), g);

        // Try to assign a teacher and find a time slot
        bool assigned = false;
        for (auto &teacher : suitableTeachers)
        {
            // Find suitable time slots
            std::vector<TimeSlot> suitableTimeSlots;

            // Start with teacher's available slots
            for (const auto &slot : teacher->availableTimeSlots)
            {
                // Check if this slot is preferred or to be avoided
                bool slotPreferred = false;
                bool slotAvoided = false;

                for (const auto &pref : preferences)
                {
                    if (pref.courseCode == section->course->code)
                    {
                        if (pref.type == StudentPreference::PREFER_TIME_SLOT &&
                            pref.timeSlot.day == slot.day &&
                            pref.timeSlot.hour == slot.hour)
                        {
                            slotPreferred = true;
                        }
                        else if (pref.type == StudentPreference::AVOID_TIME_SLOT &&
                                 pref.timeSlot.day == slot.day &&
                                 pref.timeSlot.hour == slot.hour)
                        {
                            slotAvoided = true;
                        }
                    }
                }

                if (!slotAvoided || slotPreferred)
                {
                    // Check if this slot is already assigned to another section with this teacher
                    bool alreadyAssigned = false;
                    for (auto &otherSection : sections)
                    {
                        if (otherSection->teacher == teacher)
                        {
                            for (const auto &otherSlot : otherSection->timeSlots)
                            {
                                if (slot.overlaps(otherSlot))
                                {
                                    alreadyAssigned = true;
                                    break;
                                }
                            }
                        }
                        if (alreadyAssigned)
                            break;
                    }

                    if (!alreadyAssigned)
                    {
                        suitableTimeSlots.push_back(slot);
                    }
                }
            }

            // If no suitable time slots, try next teacher
            if (suitableTimeSlots.empty())
            {
                continue;
            }

            // Shuffle time slots to avoid bias
            std::shuffle(suitableTimeSlots.begin(), suitableTimeSlots.end(), g);

            // Try to assign a time slot
            for (const auto &slot : suitableTimeSlots)
            {
                // Try to assign this time slot for the required duration
                bool canAssign = true;
                TimeSlot assignSlot = slot;

                // Ensure we have enough consecutive slots for the course duration
                for (int i = 1; i < section->course->creditHours; i++)
                {
                    assignSlot.duration++;

                    // Check if this extended slot is valid
                    if (assignSlot.hour + assignSlot.duration > 18)
                    {
                        canAssign = false;
                        break;
                    }

                    // Check if this extended slot conflicts with other sections
                    for (auto &otherSection : sections)
                    {
                        if (otherSection->teacher == teacher)
                        {
                            for (const auto &otherSlot : otherSection->timeSlots)
                            {
                                if (assignSlot.overlaps(otherSlot))
                                {
                                    canAssign = false;
                                    break;
                                }
                            }
                        }
                        if (!canAssign)
                            break;
                    }

                    if (!canAssign)
                        break;
                }

                if (canAssign)
                {
                    // Assign this teacher and time slot
                    section->assignTeacher(teacher);
                    section->addTimeSlot(assignSlot);
                    assigned = true;
                    break;
                }
            }

            if (assigned)
                break;
        }
    }

    return validateSchedule();
}

bool ClassScheduler::validateSchedule() const
{
    // Check if all sections have a teacher and time slots
    for (const auto &section : sections)
    {
        if (!section->teacher || section->timeSlots.empty())
        {
            return false;
        }

        // Check for time slot conflicts
        for (const auto &otherSection : sections)
        {
            if (section == otherSection)
                continue;

            // Check if same teacher has conflicting time slots
            if (section->teacher == otherSection->teacher)
            {
                for (const auto &slot : section->timeSlots)
                {
                    for (const auto &otherSlot : otherSection->timeSlots)
                    {
                        if (slot.overlaps(otherSlot))
                        {
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}

float ClassScheduler::evaluateSchedule() const
{
    if (!validateSchedule())
    {
        return 0.0f;
    }

    float score = 0.0f;
    float maxScore = 0.0f;

    // Evaluate based on preferences
    for (const auto &section : sections)
    {
        for (const auto &pref : preferences)
        {
            if (pref.courseCode == section->course->code)
            {
                maxScore += pref.weight;

                switch (pref.type)
                {
                case StudentPreference::PREFER_TEACHER:
                    if (section->teacher && section->teacher->id == pref.teacherId)
                    {
                        score += pref.weight;
                    }
                    break;
                case StudentPreference::AVOID_TEACHER:
                    if (!section->teacher || section->teacher->id != pref.teacherId)
                    {
                        score += pref.weight;
                    }
                    break;
                case StudentPreference::PREFER_TIME_SLOT:
                    for (const auto &slot : section->timeSlots)
                    {
                        if (slot.day == pref.timeSlot.day && slot.hour == pref.timeSlot.hour)
                        {
                            score += pref.weight;
                            break;
                        }
                    }
                    break;
                case StudentPreference::AVOID_TIME_SLOT:
                    bool avoided = true;
                    for (const auto &slot : section->timeSlots)
                    {
                        if (slot.day == pref.timeSlot.day && slot.hour == pref.timeSlot.hour)
                        {
                            avoided = false;
                            break;
                        }
                    }
                    if (avoided)
                    {
                        score += pref.weight;
                    }
                    break;
                }
            }
        }
    }

    return maxScore > 0.0f ? (score / maxScore) : 1.0f;
}

void ClassScheduler::applyConstraints()
{
    // Example: Apply constraint that certain courses must be scheduled consecutively
    for (const auto &course : courses)
    {
        if (course->creditHours > 1)
        {
            // Find all time slots that could be used for this course
            std::vector<std::string> potentialSlots;

            for (int day = 0; day < 5; day++)
            {
                for (int hour = 8; hour <= 18 - course->creditHours; hour++)
                {
                    // Add consecutive slots as a subset
                    std::set<std::string> consecutiveSlots;
                    for (int i = 0; i < course->creditHours; i++)
                    {
                        std::stringstream ss;
                        ss << "ts_" << day << "_" << (hour + i);
                        consecutiveSlots.insert(ss.str());
                    }

                    // Apply this constraint to the PQ tree
                    if (!scheduleTree.reduce(consecutiveSlots))
                    {
                        // If this constraint cannot be satisfied, try another slot
                        continue;
                    }
                }
            }
        }
    }
}

std::vector<std::shared_ptr<Section>> ClassScheduler::getSchedule() const
{
    std::vector<std::shared_ptr<Section>> scheduledSections;
    for (const auto &section : sections)
    {
        if (section->teacher && !section->timeSlots.empty())
        {
            scheduledSections.push_back(section);
        }
    }
    return scheduledSections;
}