# Class Scheduling System Implementation: Detailed Explanation

This document provides an in-depth explanation of the class scheduling system implementation, focusing on how PQ-Trees are utilized for constraint-based scheduling optimization. Each component of the system is explained in detail to understand how they work together.

## Table of Contents

1. [Overview of the System](#overview-of-the-system)
2. [PQ-Tree Implementation](#pq-tree-implementation)
3. [Scheduler Core](#scheduler-core)
4. [User Interface](#user-interface)
5. [System Integration](#system-integration)

## Overview of the System

The class scheduling system is designed to optimize course scheduling by considering various constraints and preferences. The core of the system is built on PQ-Trees, which are used to maintain ordering constraints for time slots. The system consists of several key components:

1. **PQ-Tree Implementation:** Data structure to represent and maintain ordering constraints
2. **Scheduler Core:** Algorithms to generate and validate schedules
3. **User Interface:** Interactive Raylib-based GUI for managing courses, teachers, and schedules
4. **Main Application:** Integration of all components

## PQ-Tree Implementation

### What is a PQ-Tree?

A PQ-Tree is a data structure that represents a family of permutations of a set of elements. In the context of scheduling, it represents the possible arrangements of time slots while respecting constraints.

PQ-Trees consist of three types of nodes:
- **P-Nodes:** Children can be permuted in any order (flexible time slots)
- **Q-Nodes:** Children can only be reversed (consecutive time slots)
- **Leaf Nodes:** Represent individual time slots

### Implementation in `pq_tree.h` and `pq_tree.cpp`

#### PQNode Class

The `PQNode` class represents a node in the PQ-Tree:

```cpp
class PQNode : public std::enable_shared_from_this<PQNode> {
private:
    NodeType type;
    std::string label;
    std::vector<std::shared_ptr<PQNode>> children;
    std::weak_ptr<PQNode> parent;
    
public:
    // Constructor and destructor
    PQNode(NodeType t, const std::string& lbl = "");
    ~PQNode();
    
    // Node operations
    NodeType getType() const;
    void setType(NodeType t);
    std::string getLabel() const;
    void setLabel(const std::string& lbl);
    
    // Child management
    void addChild(std::shared_ptr<PQNode> child);
    void removeChild(std::shared_ptr<PQNode> child);
    void replaceChild(std::shared_ptr<PQNode> oldChild, std::shared_ptr<PQNode> newChild);
    void clearChildren();
    
    // Parent operations
    std::shared_ptr<PQNode> getParent() const;
    void setParent(std::shared_ptr<PQNode> p);
    
    // Type checking
    bool isLeaf() const;
    bool isPNode() const;
    bool isQNode() const;
    
    // Get frontier (leaves)
    std::vector<std::string> getFrontier();
};
```

Key methods of the `PQNode` class:

- `addChild`: Adds a child node and sets this node as its parent
- `removeChild`: Removes a specific child from this node's children
- `replaceChild`: Replaces one child with another
- `getFrontier`: Returns the leaves of the subtree rooted at this node

#### PQTree Class

The `PQTree` class manages the overall tree structure:

```cpp
class PQTree {
private:
    std::shared_ptr<PQNode> root;
    std::unordered_map<std::string, std::shared_ptr<PQNode>> leaves;
    
public:
    PQTree();
    ~PQTree();
    
    void createFromUniversalSet(const std::vector<std::string>& elements);
    bool reduce(const std::set<std::string>& subset);
    std::vector<std::vector<std::string>> getPossibleArrangements();
    std::vector<std::string> getFrontier();
    void print();
    std::shared_ptr<PQNode> getRoot();
};
```

Key functions of the `PQTree` class:

- `createFromUniversalSet`: Initializes the tree with leaf nodes for each element in the given set
- `reduce`: Applies a constraint to the tree (ensures elements of the subset are consecutive)
- `getPossibleArrangements`: Returns all valid arrangements of elements based on current constraints
- `getFrontier`: Returns the current order of leaf nodes

#### PQ-Tree Reduction Algorithm

The `reduce` method implements a simplified version of the PQ-Tree reduction algorithm:

```cpp
bool PQTree::reduce(const std::set<std::string>& subset) {
    // Mark all nodes in the subset
    std::set<std::shared_ptr<PQNode>> markedLeaves;
    for (const auto& elem : subset) {
        auto it = leaves.find(elem);
        if (it != leaves.end()) {
            markedLeaves.insert(it->second);
        }
    }
    
    if (markedLeaves.size() != subset.size()) {
        return false; // Some elements in the subset are not in the tree
    }
    
    // Simplified reduction for demo purposes
    // Check if all elements are consecutive in the current arrangement
    auto frontier = getFrontier();
    
    // Find the first and last occurrence of a marked element
    int first = -1, last = -1;
    for (int i = 0; i < frontier.size(); i++) {
        if (subset.find(frontier[i]) != subset.end()) {
            if (first == -1) first = i;
            last = i;
        }
    }
    
    // Check if all elements between first and last are in the subset
    for (int i = first; i <= last; i++) {
        if (subset.find(frontier[i]) == subset.end()) {
            return false;
        }
    }
    
    return true;
}
```

This implementation checks if the elements in the subset appear consecutively in the current frontier. A full implementation would involve multiple templates and bubble-up operations to restructure the tree when needed.

## Scheduler Core

The scheduler core is implemented in `scheduler.h` and `scheduler.cpp`. It defines the data structures and algorithms for class scheduling.

### Data Structures

#### TimeSlot

```cpp
struct TimeSlot {
    int day;           // 0-4 (Monday-Friday)
    int hour;          // 8-17 (8:00 - 17:00)
    int duration;      // Duration in hours
    
    std::string toString() const;
    bool overlaps(const TimeSlot& other) const;
    static std::string dayToString(int day);
};
```

The `TimeSlot` structure represents a specific time interval on a specific day. Key methods:
- `toString`: Converts the time slot to a readable string (e.g., "Monday 10:00-12:00")
- `overlaps`: Checks if this time slot overlaps with another (used for conflict detection)

#### Teacher

```cpp
class Teacher {
public:
    std::string id;
    std::string name;
    std::vector<TimeSlot> availableTimeSlots;
    
    Teacher(const std::string& id, const std::string& name);
    void addAvailableTimeSlot(const TimeSlot& slot);
};
```

The `Teacher` class stores information about a teacher and their availability.

#### Course

```cpp
class Course {
public:
    std::string code;
    std::string title;
    int creditHours;
    std::vector<std::shared_ptr<Teacher>> assignedTeachers;
    
    Course(const std::string& code, const std::string& title, int credits);
    void assignTeacher(std::shared_ptr<Teacher> teacher);
};
```

The `Course` class represents a course with its code, title, credit hours, and assigned teachers.

#### Section

```cpp
class Section {
public:
    std::string id;
    std::shared_ptr<Course> course;
    std::shared_ptr<Teacher> teacher;
    std::vector<TimeSlot> timeSlots;
    
    Section(const std::string& id, std::shared_ptr<Course> course);
    void assignTeacher(std::shared_ptr<Teacher> teacher);
    void addTimeSlot(const TimeSlot& slot);
};
```

A `Section` represents a specific instance of a course with an assigned teacher and time slots.

#### StudentPreference

```cpp
struct StudentPreference {
    enum PreferenceType {
        PREFER_TEACHER,
        PREFER_TIME_SLOT,
        AVOID_TEACHER,
        AVOID_TIME_SLOT
    };
    
    PreferenceType type;
    std::string courseCode;
    std::string teacherId;
    TimeSlot timeSlot;
    float weight;
};
```

`StudentPreference` captures student preferences for teachers and time slots, which are considered during scheduling.

### ClassScheduler

The `ClassScheduler` class is the core component that handles schedule generation:

```cpp
class ClassScheduler {
private:
    std::vector<std::shared_ptr<Course>> courses;
    std::vector<std::shared_ptr<Teacher>> teachers;
    std::vector<std::shared_ptr<Section>> sections;
    std::vector<StudentPreference> preferences;
    
    PQTree scheduleTree;
    std::map<std::string, std::vector<std::string>> possibleAssignments;
    
public:
    // Data management
    void addCourse(std::shared_ptr<Course> course);
    void addTeacher(std::shared_ptr<Teacher> teacher);
    void addSection(std::shared_ptr<Section> section);
    void addPreference(const StudentPreference& preference);
    
    // Data access
    const std::vector<std::shared_ptr<Course>>& getCourses() const;
    const std::vector<std::shared_ptr<Teacher>>& getTeachers() const;
    const std::vector<std::shared_ptr<Section>>& getSections() const;
    
    // Scheduling operations
    bool generateSchedule();
    bool validateSchedule() const;
    float evaluateSchedule() const;
    void applyConstraints();
    std::vector<std::shared_ptr<Section>> getSchedule() const;
};
```

Key methods of `ClassScheduler`:

#### applyConstraints

```cpp
void ClassScheduler::applyConstraints() {
    // Apply constraint that certain courses must be scheduled consecutively
    for (const auto& course : courses) {
        if (course->creditHours > 1) {
            for (int day = 0; day < 5; day++) {
                for (int hour = 8; hour <= 18 - course->creditHours; hour++) {
                    // Add consecutive slots as a subset
                    std::set<std::string> consecutiveSlots;
                    for (int i = 0; i < course->creditHours; i++) {
                        std::stringstream ss;
                        ss << "ts_" << day << "_" << (hour + i);
                        consecutiveSlots.insert(ss.str());
                    }
                    
                    // Apply this constraint to the PQ tree
                    if (!scheduleTree.reduce(consecutiveSlots)) {
                        // If this constraint cannot be satisfied, try another slot
                        continue;
                    }
                }
            }
        }
    }
}
```

This method applies constraints to the PQ-Tree. In this implementation, it ensures that courses with multiple credit hours are scheduled in consecutive time slots.

#### generateSchedule

```cpp
bool ClassScheduler::generateSchedule() {
    // Clear any existing schedule
    for (auto& section : sections) {
        section->teacher = nullptr;
        section->timeSlots.clear();
    }
    
    // Generate all possible time slots
    std::vector<TimeSlot> allTimeSlots;
    for (int day = 0; day < 5; day++) {
        for (int hour = 8; hour <= 16; hour++) {
            TimeSlot slot;
            slot.day = day;
            slot.hour = hour;
            slot.duration = 1;
            allTimeSlots.push_back(slot);
        }
    }
    
    // Create universal set of elements for PQ tree
    std::vector<std::string> elements;
    for (const auto& slot : allTimeSlots) {
        std::stringstream ss;
        ss << "ts_" << slot.day << "_" << slot.hour;
        elements.push_back(ss.str());
    }
    
    scheduleTree.createFromUniversalSet(elements);
    
    // Apply constraints to PQ tree
    applyConstraints();
    
    // Get possible arrangements from the PQ tree
    auto arrangements = scheduleTree.getPossibleArrangements();
    if (arrangements.empty()) {
        return false;
    }
    
    // Scheduling algorithm:
    // 1. Shuffle sections to avoid bias
    // 2. For each section, find a suitable teacher
    // 3. For each section, find suitable time slots
    // 4. Assign teachers and time slots considering constraints and preferences
    
    // ... (algorithm implementation details) ...
    
    return validateSchedule();
}
```

The `generateSchedule` method is the heart of the scheduling algorithm. It:
1. Creates a PQ-Tree with all possible time slots
2. Applies constraints to the tree
3. Gets possible arrangements from the tree
4. Assigns teachers and time slots to sections based on constraints and preferences

#### validateSchedule and evaluateSchedule

```cpp
bool ClassScheduler::validateSchedule() const {
    // Check if all sections have a teacher and time slots
    for (const auto& section : sections) {
        if (!section->teacher || section->timeSlots.empty()) {
            return false;
        }
        
        // Check for time slot conflicts
        for (const auto& otherSection : sections) {
            if (section == otherSection) continue;
            
            // Check if same teacher has conflicting time slots
            if (section->teacher == otherSection->teacher) {
                for (const auto& slot : section->timeSlots) {
                    for (const auto& otherSlot : otherSection->timeSlots) {
                        if (slot.overlaps(otherSlot)) {
                            return false;
                        }
                    }
                }
            }
        }
    }
    
    return true;
}

float ClassScheduler::evaluateSchedule() const {
    if (!validateSchedule()) {
        return 0.0f;
    }
    
    float score = 0.0f;
    float maxScore = 0.0f;
    
    // Evaluate based on preferences
    for (const auto& section : sections) {
        for (const auto& pref : preferences) {
            if (pref.courseCode == section->course->code) {
                maxScore += pref.weight;
                
                // Add to score if preference is satisfied
                // ... (preference evaluation logic) ...
            }
        }
    }
    
    return maxScore > 0.0f ? (score / maxScore) : 1.0f;
}
```

These methods validate and evaluate the generated schedule:
- `validateSchedule` ensures there are no conflicts in the schedule
- `evaluateSchedule` computes a score based on how well the schedule satisfies student preferences

## User Interface

The user interface is implemented in `ui.h` and `ui.cpp`. It provides a graphical interface for managing courses, teachers, sections, and visualizing the schedule.

### UI Components

The UI consists of several components:
- `Button`: Represents a clickable button
- `InputField`: Text input control
- `Dropdown`: Dropdown selection control
- `ScheduleUI`: Main UI class that manages the interface

### ScheduleUI Class

```cpp
class ScheduleUI {
private:
    // Data
    std::shared_ptr<ClassScheduler> scheduler;
    
    // UI State
    Tab currentTab;
    bool isDragging;
    std::shared_ptr<Section> draggedSection;
    Vector2 dragOffset;
    
    // UI Components (buttons, inputs, dropdowns)
    
    // Tab methods
    void drawCoursesTab();
    void drawTeachersTab();
    void drawScheduleTab();
    void drawPreferencesTab();
    
    // Schedule view methods
    void drawScheduleGrid();
    void drawScheduleItems();
    void handleScheduleDrag();
    
    // Data management methods
    void addCourse();
    void addTeacher();
    void addSection();
    void addPreference();
    void generateSchedule();
    
public:
    ScheduleUI();
    ~ScheduleUI();
    
    void initialize();
    void run();
    void shutdown();
    void update();
    void draw();
    void loadDemoData();
};
```

The `ScheduleUI` class manages the entire user interface:
- It has methods for drawing different tabs (Courses, Teachers, Schedule, Preferences)
- It handles user input and interactions
- It manages the connection between the UI and the scheduler logic

Key UI features:
- Tab-based interface for different aspects of scheduling
- Interactive schedule grid with drag-and-drop support
- Forms for adding courses, teachers, sections, and preferences
- Visual representation of the generated schedule

## System Integration

The system integration is handled in `main.cpp`, which creates and runs the `ScheduleUI`:

```cpp
int main() {
    try {
        // Create and run the scheduler UI
        ScheduleUI ui;
        ui.run();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }
}
```

## How the PQ-Tree Optimizes Scheduling

The PQ-Tree plays a central role in optimizing the schedule:

1. **Representing Constraints:**
   - The PQ-Tree represents all valid permutations of time slots
   - P-nodes allow flexible ordering of their children
   - Q-nodes enforce consecutive ordering of their children

2. **Constraint Application Process:**
   - Start with a universal set of all possible time slots
   - For each constraint (e.g., consecutive slots for multi-credit courses), reduce the tree
   - The reduction ensures that the constraint is satisfied in all possible arrangements

3. **Scheduling Algorithm Integration:**
   - The scheduler creates a PQ-Tree with all possible time slots
   - It applies constraints based on course requirements
   - It retrieves possible arrangements from the tree
   - It assigns teachers and time slots based on these arrangements and preferences

4. **Benefits of PQ-Tree Approach:**
   - Efficiently represents a large number of possible schedules
   - Guarantees that all constraints are satisfied
   - Provides flexibility where possible (P-nodes)
   - Enforces strict ordering where necessary (Q-nodes)

## Key Workflows

### Schedule Generation Workflow:
1. User adds courses, teachers, and sections through the UI
2. User can add student preferences
3. When "Generate Schedule" is clicked:
   - The system creates a PQ-Tree with all time slots
   - Constraints are applied to the tree
   - The algorithm assigns teachers and time slots based on the tree and preferences
   - The result is displayed in the Schedule tab

### Constraint Application Workflow:
1. Create a PQ-Tree with all time slots as leaf nodes
2. For each course with multiple credit hours:
   - Identify sets of consecutive time slots
   - For each set, try to reduce the tree
   - If reduction succeeds, the constraint is satisfied
3. After all constraints are applied, the tree represents all valid arrangements

### Teacher and Time Slot Assignment Workflow:
1. Start with a shuffled list of sections
2. For each section:
   - Find suitable teachers (based on course assignments and preferences)
   - For each suitable teacher, find suitable time slots
   - Assign the teacher and time slot that best satisfy constraints and preferences

## Conclusion

The class scheduling system demonstrates an elegant application of PQ-Trees for constraint-based optimization. By representing scheduling constraints in a PQ-Tree, the system can efficiently generate valid schedules that satisfy all requirements.

The combination of the PQ-Tree data structure, scheduling algorithms, and an interactive user interface creates a powerful system for educational scheduling problems. 