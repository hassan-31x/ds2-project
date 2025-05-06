#ifndef UI_HPP
#define UI_HPP

#include "..\raylib\include\raylib.h" // windows
// #include "/usr/local/opt/raylib/include/raylib.h" // macos
// #include "/opt/homebrew/include/raylib.h" // macos


// #include "../raylib/include/raylib.h"

#include "Scheduler.hpp"
#include "PQTree.hpp"
#include <memory>
#include <string>
#include <vector>
#include <functional>

// Forward declarations
class Screen;

// Screen states for the application
enum class ScreenState {
    MAIN_MENU,
    COURSE_MANAGEMENT,
    TEACHER_MANAGEMENT,
    SECTION_MANAGEMENT,
    REQUIREMENT_MANAGEMENT,
    SCHEDULE_VIEWER,
    PQ_TREE_VIEWER
};

// UI component base class
class UIComponent {
public:
    UIComponent(int x, int y, int width, int height);
    virtual ~UIComponent() = default;
    
    virtual void draw() = 0;
    virtual bool handleInput() = 0;
    
    bool isMouseOver() const;
    
protected:
    int x, y, width, height;
};

// Button component
class Button : public UIComponent {
public:
    Button(int x, int y, int width, int height, const std::string& text, Color color);
    
    void draw() override;
    bool handleInput() override;
    
    void setOnClick(std::function<void()> onClick);
    
private:
    std::string text;
    Color color;
    Color hoverColor;
    std::function<void()> onClick;
    bool isPressed;
};

// Text input component
class TextInput : public UIComponent {
public:
    TextInput(int x, int y, int width, int height, const std::string& placeholder);
    
    void draw() override;
    bool handleInput() override;
    
    std::string getText() const;
    void setText(const std::string& text);
    void clear();
    
private:
    std::string text;
    std::string placeholder;
    bool isFocused;
    int cursorPos;
    float cursorTimer;
};

// Dropdown component
class Dropdown : public UIComponent {
public:
    Dropdown(int x, int y, int width, int height, const std::vector<std::string>& options);
    
    void draw() override;
    bool handleInput() override;
    
    std::string getSelectedOption() const;
    int getSelectedIndex() const;
    void setSelectedIndex(int index);
    void setOptions(const std::vector<std::string>& options);
    
private:
    std::vector<std::string> options;
    int selectedIndex;
    bool isOpen;
};

// Application screen base class
class Screen {
public:
    Screen(std::shared_ptr<Scheduler> scheduler);
    virtual ~Screen();
    
    virtual void initialize() = 0;
    virtual void update() = 0;
    virtual void draw() = 0;
    virtual ScreenState processInput() = 0;
    
protected:
    std::shared_ptr<Scheduler> scheduler;
    std::vector<std::unique_ptr<UIComponent>> components;
};

// Main UI class that manages screens and the application flow
class UI {
public:
    UI();
    ~UI();
    
    void initialize();
    void run();
    
private:
    bool isRunning;
    ScreenState currentState;
    std::shared_ptr<Scheduler> scheduler;
    std::unique_ptr<Screen> currentScreen;
    
    // Window properties
    const int screenWidth = 1280;
    const int screenHeight = 720;
    const char* windowTitle = "Habib University Course Scheduler";
    
    // Helper methods
    void changeScreen(ScreenState newState);
    std::unique_ptr<Screen> createScreen(ScreenState state);
};

// Specific screen implementations
class MainMenuScreen : public Screen {
public:
    MainMenuScreen(std::shared_ptr<Scheduler> scheduler);
    
    void initialize() override;
    void update() override;
    void draw() override;
    ScreenState processInput() override;
    
private:
    // Removed unused Texture2D logo; field
};

class CourseManagementScreen : public Screen {
public:
    CourseManagementScreen(std::shared_ptr<Scheduler> scheduler);
    
    void initialize() override;
    void update() override;
    void draw() override;
    ScreenState processInput() override;
    
private:
    std::vector<std::shared_ptr<Course>> displayedCourses;
    int selectedCourseIndex;
    TextInput* codeInput;
    TextInput* nameInput;
    TextInput* creditsInput;
    
    void refreshCourseList();
    void addCourse();
};

class TeacherManagementScreen : public Screen {
public:
    TeacherManagementScreen(std::shared_ptr<Scheduler> scheduler);
    
    void initialize() override;
    void update() override;
    void draw() override;
    ScreenState processInput() override;
    
private:
    std::vector<std::shared_ptr<Teacher>> displayedTeachers;
    int selectedTeacherIndex;
    TextInput* idInput;
    TextInput* nameInput;
    Dropdown* courseDropdown;
    
    void refreshTeacherList();
    void refreshCourseDropdown();
    void addTeacher();
    void assignCourseToTeacher();
};

class SectionManagementScreen : public Screen {
public:
    SectionManagementScreen(std::shared_ptr<Scheduler> scheduler);
    
    void initialize() override;
    void update() override;
    void draw() override;
    ScreenState processInput() override;
    
private:
    std::vector<std::shared_ptr<Section>> displayedSections;
    int selectedSectionIndex;
    TextInput* idInput;
    Dropdown* courseDropdown;
    Dropdown* teacherDropdown;
    Dropdown* dayDropdown;
    TextInput* startHourInput;
    TextInput* startMinuteInput;
    TextInput* durationInput;
    
    void refreshSectionList();
    void refreshDropdowns();
    void addSection();
};

class RequirementManagementScreen : public Screen {
public:
    RequirementManagementScreen(std::shared_ptr<Scheduler> scheduler);
    
    void initialize() override;
    void update() override;
    void draw() override;
    ScreenState processInput() override;
    
private:
    std::vector<std::shared_ptr<Requirement>> displayedRequirements;
    int selectedRequirementIndex;
    Dropdown* requirementTypeDropdown;
    Dropdown* courseDropdown;
    Dropdown* teacherDropdown;
    Dropdown* dayDropdown;
    TextInput* startHourInput;
    TextInput* startMinuteInput;
    TextInput* durationInput;
    
    void refreshRequirementList();
    void refreshDropdowns();
    void addRequirement();
};

class ScheduleViewerScreen : public Screen {
public:
    ScheduleViewerScreen(std::shared_ptr<Scheduler> scheduler);
    
    void initialize() override;
    void update() override;
    void draw() override;
    ScreenState processInput() override;
    
private:
    std::vector<std::shared_ptr<Schedule>> displayedSchedules;
    int currentScheduleIndex;
    
    void generateSchedules();
    void drawScheduleGrid();
    void drawSelectedSchedule();
};

class PQTreeViewerScreen : public Screen {
public:
    PQTreeViewerScreen(std::shared_ptr<Scheduler> scheduler);
    
    void initialize() override;
    void update() override;
    void draw() override;
    ScreenState processInput() override;
    
private:
    float zoomLevel;
    Vector2 panOffset;
    bool dragging;
    Vector2 lastMousePos;
    
    void drawPQTree();
    void drawNode(std::shared_ptr<PQNode> node, Vector2 position, float scale);
};

#endif // UI_HPP 