#include "UI.hpp"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <cstring>  // For strcpy and strlen
#include <cstdio>   // For sprintf

// UIComponent implementation
UIComponent::UIComponent(int x, int y, int width, int height)
    : x(x), y(y), width(width), height(height) {}

bool UIComponent::isMouseOver() const {
    Vector2 mousePos = GetMousePosition();
    return (mousePos.x >= x && mousePos.x <= x + width && 
            mousePos.y >= y && mousePos.y <= y + height);
}

// Button implementation
Button::Button(int x, int y, int width, int height, const std::string& text, Color color)
    : UIComponent(x, y, width, height), text(text), color(color), isPressed(false) {
    
    // Create a slightly darker hover color
    hoverColor = {
        static_cast<unsigned char>(color.r * 0.8f),
        static_cast<unsigned char>(color.g * 0.8f),
        static_cast<unsigned char>(color.b * 0.8f),
        color.a
    };
}

void Button::draw() {
    // Draw the button
    Color currentColor = isMouseOver() ? hoverColor : color;
    DrawRectangle(x, y, width, height, currentColor);
    DrawRectangleLines(x, y, width, height, BLACK);
    
    // Draw the text centered in the button
    int fontSize = 20;
    int textWidth = MeasureText(text.c_str(), fontSize);
    int textX = x + (width - textWidth) / 2;
    int textY = y + (height - fontSize) / 2;
    
    DrawText(text.c_str(), textX, textY, fontSize, WHITE);
}

bool Button::handleInput() {
    bool clicked = false;
    
    if (isMouseOver()) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            isPressed = true;
        }
        
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && isPressed) {
            if (onClick) {
                onClick();
            }
            clicked = true;
        }
    }
    
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        isPressed = false;
    }
    
    return clicked;
}

void Button::setOnClick(std::function<void()> onClick) {
    this->onClick = onClick;
}

// TextInput implementation
TextInput::TextInput(int x, int y, int width, int height, const std::string& placeholder)
    : UIComponent(x, y, width, height), text(""), placeholder(placeholder),
      isFocused(false), cursorPos(0), cursorTimer(0) {}

void TextInput::draw() {
    // Draw the input field
    DrawRectangle(x, y, width, height, WHITE);
    DrawRectangleLines(x, y, width, height, isFocused ? BLUE : BLACK);
    
    // Draw the text or placeholder
    int fontSize = 20;
    std::string displayText = text.empty() ? placeholder : text;
    Color textColor = text.empty() ? GRAY : BLACK;
    
    DrawText(displayText.c_str(), x + 5, y + (height - fontSize) / 2, fontSize, textColor);
    
    // Draw the cursor if focused
    if (isFocused) {
        cursorTimer += GetFrameTime();
        if (std::fmod(cursorTimer, 1.0f) < 0.5f) {
            int cursorX = x + 5;
            if (!text.empty()) {
                cursorX += MeasureText(text.substr(0, cursorPos).c_str(), fontSize);
            }
            
            DrawLine(cursorX, y + 5, cursorX, y + height - 5, BLACK);
        }
    }
}

bool TextInput::handleInput() {
    bool wasHandled = false;
    
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        isFocused = isMouseOver();
        if (isFocused) {
            cursorPos = static_cast<int>(text.length());
            wasHandled = true;
        }
    }
    
    if (isFocused) {
        int key = GetCharPressed();
        while (key > 0) {
            // Check if the key is printable
            if (key >= 32 && key <= 125) {
                text.insert(cursorPos, 1, static_cast<char>(key));
                cursorPos++;
                wasHandled = true;
            }
            
            key = GetCharPressed(); // Check for more keys
        }
        
        // Special key handling (backspace, delete, etc.)
        if (IsKeyPressed(KEY_BACKSPACE) && cursorPos > 0) {
            text.erase(cursorPos - 1, 1);
            cursorPos--;
            wasHandled = true;
        }
        
        if (IsKeyPressed(KEY_DELETE) && cursorPos < static_cast<int>(text.length())) {
            text.erase(cursorPos, 1);
            wasHandled = true;
        }
        
        if (IsKeyPressed(KEY_LEFT) && cursorPos > 0) {
            cursorPos--;
            wasHandled = true;
        }
        
        if (IsKeyPressed(KEY_RIGHT) && cursorPos < static_cast<int>(text.length())) {
            cursorPos++;
            wasHandled = true;
        }
        
        if (IsKeyPressed(KEY_HOME)) {
            cursorPos = 0;
            wasHandled = true;
        }
        
        if (IsKeyPressed(KEY_END)) {
            cursorPos = static_cast<int>(text.length());
            wasHandled = true;
        }
    }
    
    return wasHandled;
}

std::string TextInput::getText() const {
    return text;
}

void TextInput::setText(const std::string& text) {
    this->text = text;
    cursorPos = static_cast<int>(text.length());
}

void TextInput::clear() {
    text = "";
    cursorPos = 0;
}

// Dropdown implementation
Dropdown::Dropdown(int x, int y, int width, int height, const std::vector<std::string>& options)
    : UIComponent(x, y, width, height), options(options), selectedIndex(0), isOpen(false) {}

void Dropdown::draw() {
    // Draw the dropdown box
    DrawRectangle(x, y, width, height, WHITE);
    DrawRectangleLines(x, y, width, height, BLACK);
    
    // Draw the selected option
    std::string displayText = (selectedIndex >= 0 && static_cast<size_t>(selectedIndex) < options.size()) ? 
                              options[selectedIndex] : "Select...";
    
    int fontSize = 20;
    DrawText(displayText.c_str(), x + 5, y + (height - fontSize) / 2, fontSize, BLACK);
    
    // Draw the dropdown arrow
    DrawTriangle(
        {static_cast<float>(x + width - 20), static_cast<float>(y + height / 3)},
        {static_cast<float>(x + width - 10), static_cast<float>(y + 2 * height / 3)},
        {static_cast<float>(x + width - 30), static_cast<float>(y + 2 * height / 3)},
        BLACK
    );
    
    // Draw the dropdown list if open
    if (isOpen) {
        for (size_t i = 0; i < options.size(); i++) {
            int itemY = y + height + static_cast<int>(i) * height;
            
            // Draw the item background
            Color bgColor = (static_cast<int>(i) == selectedIndex) ? LIGHTGRAY : WHITE;
            DrawRectangle(x, itemY, width, height, bgColor);
            DrawRectangleLines(x, itemY, width, height, BLACK);
            
            // Draw the item text
            DrawText(options[i].c_str(), x + 5, itemY + (height - fontSize) / 2, fontSize, BLACK);
        }
    }
}

bool Dropdown::handleInput() {
    bool wasHandled = false;
    
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (isMouseOver()) {
            isOpen = !isOpen;
            wasHandled = true;
        } else if (isOpen) {
            // Check if clicked on an item in the dropdown
            Vector2 mousePos = GetMousePosition();
            
            for (size_t i = 0; i < options.size(); i++) {
                int itemY = y + height + static_cast<int>(i) * height;
                
                if (mousePos.x >= x && mousePos.x <= x + width &&
                    mousePos.y >= itemY && mousePos.y <= itemY + height) {
                    
                    selectedIndex = i;
                    isOpen = false;
                    wasHandled = true;
                    break;
                }
            }
            
            // If clicked outside the dropdown, close it
            if (!wasHandled) {
                isOpen = false;
                wasHandled = true;
            }
        }
    }
    
    return wasHandled;
}

std::string Dropdown::getSelectedOption() const {
    return (selectedIndex >= 0 && static_cast<size_t>(selectedIndex) < options.size()) ? 
           options[selectedIndex] : "";
}

int Dropdown::getSelectedIndex() const {
    return selectedIndex;
}

void Dropdown::setSelectedIndex(int index) {
    if (index >= 0 && static_cast<size_t>(index) < options.size()) {
        selectedIndex = index;
    }
}

void Dropdown::setOptions(const std::vector<std::string>& options) {
    this->options = options;
    if (static_cast<size_t>(selectedIndex) >= options.size()) {
        selectedIndex = options.empty() ? -1 : 0;
    }
}

// Screen implementation
Screen::Screen(std::shared_ptr<Scheduler> scheduler)
    : scheduler(scheduler) {}

Screen::~Screen() {
    components.clear();
}

// UI implementation
UI::UI()
    : isRunning(false), currentState(ScreenState::MAIN_MENU) {
    scheduler = std::make_shared<Scheduler>();
}

UI::~UI() {
    // Clean up any resources
}

void UI::initialize() {
    // Initialize Raylib
    InitWindow(screenWidth, screenHeight, windowTitle);
    SetTargetFPS(60);
    
    // Add dummy data
    addDummyData();
    
    // Create the initial screen
    changeScreen(ScreenState::MAIN_MENU);
    
    isRunning = true;
}

void UI::run() {
    while (isRunning && !WindowShouldClose()) {
        // Process input
        ScreenState newState = currentScreen->processInput();
        if (newState != currentState) {
            changeScreen(newState);
        }
        
        // Update
        currentScreen->update();
        
        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        currentScreen->draw();
        EndDrawing();
    }
    
    CloseWindow();
}

void UI::changeScreen(ScreenState newState) {
    currentState = newState;
    currentScreen = createScreen(newState);
    currentScreen->initialize();
}

std::unique_ptr<Screen> UI::createScreen(ScreenState state) {
    switch (state) {
        case ScreenState::MAIN_MENU:
            return std::unique_ptr<Screen>(new MainMenuScreen(scheduler));
        
        case ScreenState::COURSE_MANAGEMENT:
            return std::unique_ptr<Screen>(new CourseManagementScreen(scheduler));
            
        case ScreenState::TEACHER_MANAGEMENT:
            return std::unique_ptr<Screen>(new TeacherManagementScreen(scheduler));
            
        case ScreenState::SECTION_MANAGEMENT:
            return std::unique_ptr<Screen>(new SectionManagementScreen(scheduler));
            
        case ScreenState::REQUIREMENT_MANAGEMENT:
            return std::unique_ptr<Screen>(new RequirementManagementScreen(scheduler));
            
        case ScreenState::SCHEDULE_VIEWER:
            return std::unique_ptr<Screen>(new ScheduleViewerScreen(scheduler));
            
        case ScreenState::PQ_TREE_VIEWER:
            return std::unique_ptr<Screen>(new PQTreeViewerScreen(scheduler));
            
        default:
            return std::unique_ptr<Screen>(new MainMenuScreen(scheduler));
    }
}

// MainMenuScreen implementation
MainMenuScreen::MainMenuScreen(std::shared_ptr<Scheduler> scheduler)
    : Screen(scheduler) {}

void MainMenuScreen::initialize() {
    // Create buttons for the main menu
    int buttonWidth = 300;
    int buttonHeight = 50;
    int buttonSpacing = 20;
    int startY = 250;
    int centerX = GetScreenWidth() / 2 - buttonWidth / 2;
    
    // Button for course management
    auto courseButton = std::unique_ptr<Button>(new Button(
        centerX, startY, buttonWidth, buttonHeight, 
        "Manage Courses", BLUE
    ));
    courseButton->setOnClick([]() {
        // Will switch to course management screen in processInput
    });
    components.push_back(std::move(courseButton));
    
    // Button for teacher management
    auto teacherButton = std::unique_ptr<Button>(new Button(
        centerX, startY + buttonHeight + buttonSpacing, 
        buttonWidth, buttonHeight, "Manage Teachers", BLUE
    ));
    teacherButton->setOnClick([]() {
        // Will switch to teacher management screen in processInput
    });
    components.push_back(std::move(teacherButton));
    
    // Button for section management
    auto sectionButton = std::unique_ptr<Button>(new Button(
        centerX, startY + 2 * (buttonHeight + buttonSpacing), 
        buttonWidth, buttonHeight, "Manage Sections", BLUE
    ));
    sectionButton->setOnClick([]() {
        // Will switch to section management screen in processInput
    });
    components.push_back(std::move(sectionButton));
    
    // Button for requirement management
    auto requirementButton = std::unique_ptr<Button>(new Button(
        centerX, startY + 3 * (buttonHeight + buttonSpacing), 
        buttonWidth, buttonHeight, "Manage Requirements", BLUE
    ));
    requirementButton->setOnClick([]() {
        // Will switch to requirement management screen in processInput
    });
    components.push_back(std::move(requirementButton));
    
    // Button for schedule viewer
    auto scheduleButton = std::unique_ptr<Button>(new Button(
        centerX, startY + 4 * (buttonHeight + buttonSpacing), 
        buttonWidth, buttonHeight, "View Schedules", GREEN
    ));
    scheduleButton->setOnClick([]() {
        // Will switch to schedule viewer screen in processInput
    });
    components.push_back(std::move(scheduleButton));
    
    // Button for PQ tree viewer
    auto pqTreeButton = std::unique_ptr<Button>(new Button(
        centerX, startY + 5 * (buttonHeight + buttonSpacing), 
        buttonWidth, buttonHeight, "View PQ Tree", PURPLE
    ));
    pqTreeButton->setOnClick([]() {
        // Will switch to PQ tree viewer screen in processInput
    });
    components.push_back(std::move(pqTreeButton));
    
    // Load logo
    // In a real application, you would load an actual image
    // Since we're focusing on code structure, we'll just create a placeholder
}

void MainMenuScreen::update() {
    // Nothing to update in the main menu
}

void MainMenuScreen::draw() {
    // Draw the title
    const char* title = "Habib University Course Scheduler";
    int fontSize = 40;
    int titleWidth = MeasureText(title, fontSize);
    DrawText(title, GetScreenWidth() / 2 - titleWidth / 2, 100, fontSize, DARKBLUE);
    
    // Draw a fancy subtitle
    const char* subtitle = "Powered by PQ Trees";
    int subtitleFontSize = 20;
    int subtitleWidth = MeasureText(subtitle, subtitleFontSize);
    DrawText(subtitle, GetScreenWidth() / 2 - subtitleWidth / 2, 150, subtitleFontSize, DARKGRAY);
    
    // Draw all UI components
    for (const auto& component : components) {
        component->draw();
    }
    
    // Draw the footer
    const char* footer = "Data Structures Project";
    int footerFontSize = 15;
    int footerWidth = MeasureText(footer, footerFontSize);
    DrawText(footer, GetScreenWidth() / 2 - footerWidth / 2, GetScreenHeight() - 30, footerFontSize, DARKGRAY);
}

ScreenState MainMenuScreen::processInput() {
    // Check each component for input
    for (size_t i = 0; i < components.size(); i++) {
        if (components[i]->handleInput()) {
            // Return the appropriate screen state based on which button was clicked
            switch (i) {
                case 0: return ScreenState::COURSE_MANAGEMENT;
                case 1: return ScreenState::TEACHER_MANAGEMENT;
                case 2: return ScreenState::SECTION_MANAGEMENT;
                case 3: return ScreenState::REQUIREMENT_MANAGEMENT;
                case 4: return ScreenState::SCHEDULE_VIEWER;
                case 5: return ScreenState::PQ_TREE_VIEWER;
                default: break;
            }
        }
    }
    
    return ScreenState::MAIN_MENU;
}

// Implement other screen classes similarly
// For brevity, I'll just include a simplified implementation of CourseManagementScreen

CourseManagementScreen::CourseManagementScreen(std::shared_ptr<Scheduler> scheduler)
    : Screen(scheduler), selectedCourseIndex(-1) {}

void CourseManagementScreen::initialize() {
    // Create UI components
    
    // Back button
    auto backButton = std::unique_ptr<Button>(new Button(
        20, 20, 100, 40, "Back", GRAY
    ));
    backButton->setOnClick([]() {
        // Will return to main menu in processInput
    });
    components.push_back(std::move(backButton));
    
    // Course input fields
    int inputWidth = 200;
    int inputHeight = 40;
    int inputX = 150;
    int inputY = 100;
    int spacing = 60;
    
    // Code input
    codeInput = new TextInput(inputX, inputY, inputWidth, inputHeight, "Course Code");
    components.push_back(std::unique_ptr<UIComponent>(codeInput));
    
    // Name input
    nameInput = new TextInput(inputX, inputY + spacing, inputWidth, inputHeight, "Course Name");
    components.push_back(std::unique_ptr<UIComponent>(nameInput));
    
    // Credits input
    creditsInput = new TextInput(inputX, inputY + 2 * spacing, inputWidth, inputHeight, "Credits");
    components.push_back(std::unique_ptr<UIComponent>(creditsInput));
    
    // Add button
    auto addButton = std::unique_ptr<Button>(new Button(
        inputX, inputY + 3 * spacing, inputWidth, inputHeight, "Add Course", GREEN
    ));
    addButton->setOnClick([this]() {
        addCourse();
    });
    components.push_back(std::move(addButton));
    
    // Refresh the course list
    refreshCourseList();
}

void CourseManagementScreen::update() {
    // Nothing to update continuously
}

void CourseManagementScreen::draw() {
    // Draw the title
    DrawText("Course Management", 20, 70, 30, DARKBLUE);
    
    // Draw the input field labels
    DrawText("Code:", 30, 110, 20, BLACK);
    DrawText("Name:", 30, 170, 20, BLACK);
    DrawText("Credits:", 30, 230, 20, BLACK);
    
    // Draw all UI components
    for (const auto& component : components) {
        component->draw();
    }
    
    // Draw the course list
    DrawText("Courses:", 400, 70, 30, DARKBLUE);
    
    int listX = 400;
    int listY = 110;
    int itemHeight = 30;
    
    for (size_t i = 0; i < displayedCourses.size(); i++) {
        Color textColor = (static_cast<int>(i) == selectedCourseIndex) ? RED : BLACK;
        std::string courseText = displayedCourses[i]->getCode() + " - " + 
                                 displayedCourses[i]->getName() + " (" + 
                                 std::to_string(displayedCourses[i]->getCredits()) + " credits)";
        
        DrawText(courseText.c_str(), listX, listY + static_cast<int>(i) * itemHeight, 20, textColor);
    }
}

ScreenState CourseManagementScreen::processInput() {
    // Check each component for input
    for (size_t i = 0; i < components.size(); i++) {
        if (components[i]->handleInput()) {
            // If the back button was clicked
            if (i == 0) {
                return ScreenState::MAIN_MENU;
            }
        }
    }
    
    // Check for course selection
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        
        int listX = 400;
        int listY = 110;
        int itemHeight = 30;
        int itemWidth = 500;
        
        for (size_t i = 0; i < displayedCourses.size(); i++) {
            if (mousePos.x >= listX && mousePos.x <= listX + itemWidth &&
                mousePos.y >= listY + i * itemHeight && mousePos.y <= listY + (i + 1) * itemHeight) {
                
                selectedCourseIndex = i;
                break;
            }
        }
    }
    
    return ScreenState::COURSE_MANAGEMENT;
}

void CourseManagementScreen::refreshCourseList() {
    displayedCourses = scheduler->getCourses();
}

void CourseManagementScreen::addCourse() {
    std::string code = codeInput->getText();
    std::string name = nameInput->getText();
    std::string creditsStr = creditsInput->getText();
    
    if (code.empty() || name.empty() || creditsStr.empty()) {
        return;
    }
    
    int credits = 0;
    try {
        credits = std::stoi(creditsStr);
    } catch (const std::exception&) {
        return;
    }
    
    auto course = std::make_shared<Course>(code, name, credits);
    scheduler->addCourse(course);
    
    // Clear the input fields
    codeInput->clear();
    nameInput->clear();
    creditsInput->clear();
    
    // Refresh the course list
    refreshCourseList();
}

// Add stub implementations for the missing screen classes

// TeacherManagementScreen implementation
TeacherManagementScreen::TeacherManagementScreen(std::shared_ptr<Scheduler> scheduler)
    : Screen(scheduler), selectedTeacherIndex(-1) {}

void TeacherManagementScreen::initialize() {
    // Create a back button
    auto backButton = std::unique_ptr<Button>(new Button(
        20, 20, 100, 40, "Back", GRAY
    ));
    backButton->setOnClick([]() {
        // Will return to main menu in processInput
    });
    components.push_back(std::move(backButton));
    
    // Teacher input fields
    int inputWidth = 200;
    int inputHeight = 40;
    int inputX = 150;
    int inputY = 100;
    int spacing = 60;
    
    // ID input
    idInput = new TextInput(inputX, inputY, inputWidth, inputHeight, "Teacher ID");
    components.push_back(std::unique_ptr<UIComponent>(idInput));
    
    // Name input
    nameInput = new TextInput(inputX, inputY + spacing, inputWidth, inputHeight, "Teacher Name");
    components.push_back(std::unique_ptr<UIComponent>(nameInput));
    
    // Add button
    auto addButton = std::unique_ptr<Button>(new Button(
        inputX, inputY + 2 * spacing, inputWidth, inputHeight, "Add Teacher", GREEN
    ));
    addButton->setOnClick([this]() {
        addTeacher();
    });
    components.push_back(std::move(addButton));
    
    // Courses dropdown (becomes active when a teacher is selected)
    std::vector<std::string> courseOptions;
    for (const auto& course : scheduler->getCourses()) {
        courseOptions.push_back(course->getCode() + " - " + course->getName());
    }
    
    if (courseOptions.empty()) {
        courseOptions.push_back("No courses available");
    }
    
    courseDropdown = new Dropdown(inputX, inputY + 3 * spacing, inputWidth, inputHeight, courseOptions);
    components.push_back(std::unique_ptr<UIComponent>(courseDropdown));
    
    // Assign course button
    auto assignButton = std::unique_ptr<Button>(new Button(
        inputX, inputY + 4 * spacing, inputWidth, inputHeight, "Assign Course", BLUE
    ));
    assignButton->setOnClick([this]() {
        assignCourseToTeacher();
    });
    components.push_back(std::move(assignButton));
    
    // Refresh the teacher list
    refreshTeacherList();
}

void TeacherManagementScreen::update() {
    // Nothing to update continuously
}

void TeacherManagementScreen::draw() {
    DrawText("Teacher Management", 20, 70, 30, DARKBLUE);
    
    // Draw the input field labels
    DrawText("ID:", 30, 110, 20, BLACK);
    DrawText("Name:", 30, 170, 20, BLACK);
    
    // If a teacher is selected, draw the course assignment section
    if (selectedTeacherIndex >= 0 && static_cast<size_t>(selectedTeacherIndex) < displayedTeachers.size()) {
        DrawText("Assign Course:", 30, 290, 20, BLACK);
    }
    
    // Draw all UI components
    for (const auto& component : components) {
        component->draw();
    }
    
    // Draw the teacher list
    DrawText("Teachers:", 400, 70, 30, DARKBLUE);
    
    int listX = 400;
    int listY = 110;
    int itemHeight = 30;
    
    for (size_t i = 0; i < displayedTeachers.size(); i++) {
        Color textColor = (static_cast<int>(i) == selectedTeacherIndex) ? RED : BLACK;
        std::string teacherText = displayedTeachers[i]->getId() + " - " + 
                                 displayedTeachers[i]->getName();
        
        DrawText(teacherText.c_str(), listX, listY + static_cast<int>(i) * itemHeight, 20, textColor);
    }
    
    // If a teacher is selected, draw their assigned courses
    if (selectedTeacherIndex >= 0 && static_cast<size_t>(selectedTeacherIndex) < displayedTeachers.size()) {
        auto teacher = displayedTeachers[selectedTeacherIndex];
        
        // Draw courses title
        DrawText("Assigned Courses:", 700, 70, 30, DARKBLUE);
        
        int courseX = 700;
        int courseY = 110;
        
        // Draw courses
        const auto& courses = teacher->getCourses();
        if (courses.empty()) {
            DrawText("No courses assigned", courseX, courseY, 20, GRAY);
        } else {
            for (size_t i = 0; i < courses.size(); i++) {
                std::string courseText = courses[i]->getCode() + " - " + courses[i]->getName();
                DrawText(courseText.c_str(), courseX, courseY + static_cast<int>(i) * itemHeight, 20, DARKGRAY);
            }
        }
    }
}

ScreenState TeacherManagementScreen::processInput() {
    // Check each component for input
    for (size_t i = 0; i < components.size(); i++) {
        if (components[i]->handleInput()) {
            // If the back button was clicked
            if (i == 0) {
                return ScreenState::MAIN_MENU;
            }
        }
    }
    
    // Check for teacher selection
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        
        int listX = 400;
        int listY = 110;
        int itemHeight = 30;
        int itemWidth = 300;
        
        for (size_t i = 0; i < displayedTeachers.size(); i++) {
            if (mousePos.x >= listX && mousePos.x <= listX + itemWidth &&
                mousePos.y >= listY + static_cast<int>(i) * itemHeight && 
                mousePos.y <= listY + static_cast<int>(i + 1) * itemHeight) {
                
                selectedTeacherIndex = static_cast<int>(i);
                refreshCourseDropdown();
                break;
            }
        }
    }
    
    return ScreenState::TEACHER_MANAGEMENT;
}

void TeacherManagementScreen::refreshTeacherList() {
    displayedTeachers = scheduler->getTeachers();
}

void TeacherManagementScreen::refreshCourseDropdown() {
    std::vector<std::string> courseOptions;
    
    // Get all courses
    for (const auto& course : scheduler->getCourses()) {
        courseOptions.push_back(course->getCode() + " - " + course->getName());
    }
    
    if (courseOptions.empty()) {
        courseOptions.push_back("No courses available");
    }
    
    courseDropdown->setOptions(courseOptions);
}

void TeacherManagementScreen::addTeacher() {
    std::string id = idInput->getText();
    std::string name = nameInput->getText();
    
    if (id.empty() || name.empty()) {
        return;
    }
    
    auto teacher = std::make_shared<Teacher>(id, name);
    scheduler->addTeacher(teacher);
    
    // Clear the input fields
    idInput->clear();
    nameInput->clear();
    
    // Refresh the teacher list
    refreshTeacherList();
}

void TeacherManagementScreen::assignCourseToTeacher() {
    // Check if a teacher is selected
    if (selectedTeacherIndex < 0 || static_cast<size_t>(selectedTeacherIndex) >= displayedTeachers.size()) {
        return;
    }
    
    // Get the selected course
    std::string selectedCourseOption = courseDropdown->getSelectedOption();
    if (selectedCourseOption == "No courses available") {
        return;
    }
    
    // Extract the course code from the option string
    std::string courseCode = selectedCourseOption.substr(0, selectedCourseOption.find(" - "));
    
    // Find the course with this code
    for (const auto& course : scheduler->getCourses()) {
        if (course->getCode() == courseCode) {
            // Add the course to the teacher
            auto teacher = displayedTeachers[selectedTeacherIndex];
            teacher->addCourse(course);
            
            // Refresh the display
            refreshTeacherList();
            break;
        }
    }
}

// SectionManagementScreen implementation
SectionManagementScreen::SectionManagementScreen(std::shared_ptr<Scheduler> scheduler)
    : Screen(scheduler), selectedSectionIndex(-1) {}

void SectionManagementScreen::initialize() {
    // Create a back button
    auto backButton = std::unique_ptr<Button>(new Button(
        20, 20, 100, 40, "Back", GRAY
    ));
    backButton->setOnClick([]() {
        // Will return to main menu in processInput
    });
    components.push_back(std::move(backButton));
    
    // Section input fields
    int inputWidth = 200;
    int inputHeight = 40;
    int inputX = 150;
    int inputY = 100;
    int spacing = 60;
    
    // Section ID input
    idInput = new TextInput(inputX, inputY, inputWidth, inputHeight, "Section ID");
    components.push_back(std::unique_ptr<UIComponent>(idInput));
    
    // Course dropdown
    std::vector<std::string> courseOptions;
    for (const auto& course : scheduler->getCourses()) {
        courseOptions.push_back(course->getCode() + " - " + course->getName());
    }
    if (courseOptions.empty()) {
        courseOptions.push_back("No courses available");
    }
    courseDropdown = new Dropdown(inputX, inputY + spacing, inputWidth, inputHeight, courseOptions);
    components.push_back(std::unique_ptr<UIComponent>(courseDropdown));
    
    // Teacher dropdown
    std::vector<std::string> teacherOptions;
    for (const auto& teacher : scheduler->getTeachers()) {
        teacherOptions.push_back(teacher->getId() + " - " + teacher->getName());
    }
    if (teacherOptions.empty()) {
        teacherOptions.push_back("No teachers available");
    }
    teacherDropdown = new Dropdown(inputX, inputY + 2 * spacing, inputWidth, inputHeight, teacherOptions);
    components.push_back(std::unique_ptr<UIComponent>(teacherDropdown));
    
    // Duration input
    durationInput = new TextInput(inputX, inputY + 3 * spacing, inputWidth, inputHeight, "Duration (min)");
    components.push_back(std::unique_ptr<UIComponent>(durationInput));
    
    // Add section button
    auto addButton = std::unique_ptr<Button>(new Button(
        inputX, inputY + 4 * spacing, inputWidth, inputHeight, "Add Section", GREEN
    ));
    addButton->setOnClick([this]() {
        addSection();
    });
    components.push_back(std::move(addButton));
    
    // Refresh section list
    refreshSectionList();
    refreshDropdowns();
}

void SectionManagementScreen::update() {
    // Nothing to update continuously
}

void SectionManagementScreen::draw() {
    DrawText("Section Management", 20, 70, 30, DARKBLUE);
    
    // Draw the input field labels
    DrawText("ID:", 30, 110, 20, BLACK);
    DrawText("Course:", 30, 160, 20, BLACK);
    DrawText("Teacher:", 30, 210, 20, BLACK);
    DrawText("Duration:", 30, 260, 20, BLACK);
    
    // Draw all UI components
    for (const auto& component : components) {
        component->draw();
    }
    
    // Draw the section list
    DrawText("Sections:", 400, 70, 30, DARKBLUE);
    
    int listX = 400;
    int listY = 110;
    int itemHeight = 30;
    
    for (size_t i = 0; i < displayedSections.size(); i++) {
        Color textColor = (static_cast<int>(i) == selectedSectionIndex) ? RED : BLACK;
        auto section = displayedSections[i];
        std::string sectionText = section->getId() + " - " + 
                                  section->getCourse()->getCode() + " - " +
                                  section->getTeacher()->getName();
        
        DrawText(sectionText.c_str(), listX, listY + static_cast<int>(i) * itemHeight, 20, textColor);
    }
    
    // If a section is selected, draw its details
    if (selectedSectionIndex >= 0 && static_cast<size_t>(selectedSectionIndex) < displayedSections.size()) {
        auto section = displayedSections[selectedSectionIndex];
        auto timeSlot = section->getTimeSlot();
        
        // Draw section details title
        DrawText("Section Details:", 700, 70, 30, DARKBLUE);
        
        int detailX = 700;
        int detailY = 110;
        
        // Draw section details
        DrawText(("ID: " + section->getId()).c_str(), detailX, detailY, 20, DARKGRAY);
        DrawText(("Course: " + section->getCourse()->getCode() + " - " + section->getCourse()->getName()).c_str(), 
                 detailX, detailY + 30, 20, DARKGRAY);
        DrawText(("Teacher: " + section->getTeacher()->getName()).c_str(), 
                 detailX, detailY + 60, 20, DARKGRAY);
        
        // Display time information - either just day and duration or full time if scheduled
        std::string timeInfo = "Time: " + timeSlot->toString();
        if (!timeSlot->hasStartTime()) {
            timeInfo += " (Start time will be assigned during scheduling)";
        }
        
        DrawText(timeInfo.c_str(), detailX, detailY + 90, 20, DARKGRAY);
    }
}

ScreenState SectionManagementScreen::processInput() {
    // Check each component for input
    for (size_t i = 0; i < components.size(); i++) {
        if (components[i]->handleInput()) {
            // If the back button was clicked
            if (i == 0) {
                return ScreenState::MAIN_MENU;
            }
        }
    }
    
    // Check for section selection
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        
        int listX = 400;
        int listY = 110;
        int itemHeight = 30;
        int itemWidth = 300;
        
        for (size_t i = 0; i < displayedSections.size(); i++) {
            if (mousePos.x >= listX && mousePos.x <= listX + itemWidth &&
                mousePos.y >= listY + static_cast<int>(i) * itemHeight && 
                mousePos.y <= listY + static_cast<int>(i + 1) * itemHeight) {
                
                selectedSectionIndex = static_cast<int>(i);
                break;
            }
        }
    }
    
    return ScreenState::SECTION_MANAGEMENT;
}

void SectionManagementScreen::refreshSectionList() {
    displayedSections = scheduler->getSections();
}

void SectionManagementScreen::refreshDropdowns() {
    // Refresh course dropdown
    std::vector<std::string> courseOptions;
    for (const auto& course : scheduler->getCourses()) {
        courseOptions.push_back(course->getCode() + " - " + course->getName());
    }
    if (courseOptions.empty()) {
        courseOptions.push_back("No courses available");
    }
    courseDropdown->setOptions(courseOptions);
    
    // Refresh teacher dropdown
    std::vector<std::string> teacherOptions;
    for (const auto& teacher : scheduler->getTeachers()) {
        teacherOptions.push_back(teacher->getId() + " - " + teacher->getName());
    }
    if (teacherOptions.empty()) {
        teacherOptions.push_back("No teachers available");
    }
    teacherDropdown->setOptions(teacherOptions);
}

void SectionManagementScreen::addSection() {
    // Get input values
    std::string id = idInput->getText();
    std::string courseOption = courseDropdown->getSelectedOption();
    std::string teacherOption = teacherDropdown->getSelectedOption();
    std::string durationStr = durationInput->getText();
    
    // Validate input
    if (id.empty() || 
        courseOption == "No courses available" || 
        teacherOption == "No teachers available" ||
        durationStr.empty()) {
        return;
    }
    
    // Parse duration value
    int duration;
    try {
        duration = std::stoi(durationStr);
        
        // Validate duration value
        if (duration <= 0) {
            return;
        }
    } catch (const std::exception&) {
        return;
    }
    
    // Find the selected course
    std::string courseCode = courseOption.substr(0, courseOption.find(" - "));
    std::shared_ptr<Course> selectedCourse;
    for (const auto& course : scheduler->getCourses()) {
        if (course->getCode() == courseCode) {
            selectedCourse = course;
            break;
        }
    }
    
    if (!selectedCourse) {
        return;
    }
    
    // Find the selected teacher
    std::string teacherId = teacherOption.substr(0, teacherOption.find(" - "));
    std::shared_ptr<Teacher> selectedTeacher;
    for (const auto& teacher : scheduler->getTeachers()) {
        if (teacher->getId() == teacherId) {
            selectedTeacher = teacher;
            break;
        }
    }
    
    if (!selectedTeacher) {
        return;
    }
    
    // Create TimeSlot (only with duration - day and start time will be assigned by scheduler)
    auto timeSlot = std::make_shared<TimeSlot>(duration);
    
    // Create Section
    auto section = std::make_shared<Section>(id, selectedCourse, selectedTeacher, timeSlot);
    
    // Add Section to scheduler
    scheduler->addSection(section);
    
    // Clear input fields
    idInput->clear();
    durationInput->clear();
    
    // Refresh section list
    refreshSectionList();
}

// RequirementManagementScreen implementation
RequirementManagementScreen::RequirementManagementScreen(std::shared_ptr<Scheduler> scheduler)
    : Screen(scheduler), selectedRequirementIndex(-1) {}

void RequirementManagementScreen::initialize() {
    // Create a back button
    auto backButton = std::unique_ptr<Button>(new Button(
        20, 20, 100, 40, "Back", GRAY
    ));
    backButton->setOnClick([]() {
        // Will return to main menu in processInput
    });
    components.push_back(std::move(backButton));
    
    // Requirement input fields
    int inputWidth = 200;
    int inputHeight = 40;
    int inputX = 150;
    int inputY = 100;
    int spacing = 60;
    
    // Section dropdown
    std::vector<std::string> sectionOptions;
    for (const auto& section : scheduler->getSections()) {
        std::string course = section->getCourse()->getCode();
        std::string teacher = section->getTeacher()->getName();
        sectionOptions.push_back(section->getId() + " - " + course + " - " + teacher);
    }
    if (sectionOptions.empty()) {
        sectionOptions.push_back("No sections available");
    }
    sectionDropdown = new Dropdown(inputX, inputY, inputWidth, inputHeight, sectionOptions);
    components.push_back(std::unique_ptr<UIComponent>(sectionDropdown));
    
    // Day dropdown
    std::vector<std::string> dayOptions = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};
    dayDropdown = new Dropdown(inputX, inputY + spacing, inputWidth, inputHeight, dayOptions);
    components.push_back(std::unique_ptr<UIComponent>(dayDropdown));
    
    // Start hour input (optional)
    startHourInput = new TextInput(inputX, inputY + 2 * spacing, inputWidth / 2 - 5, inputHeight, "Hour (optional)");
    components.push_back(std::unique_ptr<UIComponent>(startHourInput));
    
    // Start minute input (optional)
    startMinuteInput = new TextInput(inputX + inputWidth / 2 + 5, inputY + 2 * spacing, inputWidth / 2 - 5, inputHeight, "Min (optional)");
    components.push_back(std::unique_ptr<UIComponent>(startMinuteInput));
    
    // Add requirement button
    auto addButton = std::unique_ptr<Button>(new Button(
        inputX, inputY + 3 * spacing, inputWidth, inputHeight, "Add Requirement", GREEN
    ));
    addButton->setOnClick([this]() {
        addRequirement();
    });
    components.push_back(std::move(addButton));
    
    // Refresh requirement list
    refreshRequirementList();
    refreshDropdowns();
}

void RequirementManagementScreen::update() {
    // Nothing to update continuously
}

void RequirementManagementScreen::draw() {
    DrawText("Requirement Management", 20, 70, 30, DARKBLUE);
    
    // Draw the input field labels
    DrawText("Section:", 30, 110, 20, BLACK);
    DrawText("Day:", 30, 170, 20, BLACK);
    DrawText("Start Time (optional):", 30, 230, 20, BLACK);
    
    // Draw all UI components
    for (const auto& component : components) {
        component->draw();
    }
    
    // Draw the requirement list
    DrawText("Requirements:", 400, 70, 30, DARKBLUE);
    
    int listX = 400;
    int listY = 110;
    int itemHeight = 30;
    
    for (size_t i = 0; i < displayedRequirements.size(); i++) {
        Color textColor = (static_cast<int>(i) == selectedRequirementIndex) ? RED : BLACK;
        std::string reqText = displayedRequirements[i]->getDescription();
        
        DrawText(reqText.c_str(), listX, listY + static_cast<int>(i) * itemHeight, 20, textColor);
    }
}

ScreenState RequirementManagementScreen::processInput() {
    // Check each component for input
    for (size_t i = 0; i < components.size(); i++) {
        if (components[i]->handleInput()) {
            // If the back button was clicked
            if (i == 0) {
                return ScreenState::MAIN_MENU;
            }
        }
    }
    
    // Check for requirement selection
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        
        int listX = 400;
        int listY = 110;
        int itemHeight = 30;
        int itemWidth = 600;
        
        for (size_t i = 0; i < displayedRequirements.size(); i++) {
            if (mousePos.x >= listX && mousePos.x <= listX + itemWidth &&
                mousePos.y >= listY + static_cast<int>(i) * itemHeight && 
                mousePos.y <= listY + static_cast<int>(i + 1) * itemHeight) {
                
                selectedRequirementIndex = static_cast<int>(i);
                break;
            }
        }
    }
    
    return ScreenState::REQUIREMENT_MANAGEMENT;
}

void RequirementManagementScreen::refreshRequirementList() {
    displayedRequirements = scheduler->getRequirements();
}

void RequirementManagementScreen::refreshDropdowns() {
    // Refresh section dropdown
    std::vector<std::string> sectionOptions;
    for (const auto& section : scheduler->getSections()) {
        std::string course = section->getCourse()->getCode();
        std::string teacher = section->getTeacher()->getName();
        sectionOptions.push_back(section->getId() + " - " + course + " - " + teacher);
    }
    if (sectionOptions.empty()) {
        sectionOptions.push_back("No sections available");
    }
    sectionDropdown->setOptions(sectionOptions);
}

void RequirementManagementScreen::addRequirement() {
    // Get selected section
    std::string sectionOption = sectionDropdown->getSelectedOption();
    if (sectionOption == "No sections available") {
        return;
    }
    
    // Extract the section ID from the option string
    std::string sectionId = sectionOption.substr(0, sectionOption.find(" - "));
    
    // Find the section with this ID
    std::shared_ptr<Section> selectedSection;
    for (const auto& section : scheduler->getSections()) {
        if (section->getId() == sectionId) {
            selectedSection = section;
            break;
        }
    }
    
    if (!selectedSection) {
        return;
    }
    
    // Get time slot parameters
    std::string dayOption = dayDropdown->getSelectedOption();
    std::string startHourStr = startHourInput->getText();
    std::string startMinuteStr = startMinuteInput->getText();
    
    // Get the duration from the section's existing timeSlot
    int duration = selectedSection->getTimeSlot()->getDurationMinutes();
    
    // Parse time values
    int startHour = -1, startMinute = -1;
    
    // Only parse start time if both fields are provided
    if (!startHourStr.empty() && !startMinuteStr.empty()) {
        try {
            startHour = std::stoi(startHourStr);
            startMinute = std::stoi(startMinuteStr);
            
            // Validate time values
            if (startHour < 0 || startHour > 23 || 
                startMinute < 0 || startMinute > 59) {
                return;
            }
        } catch (const std::exception&) {
            return;
        }
    }
    
    // Create TimeSlot
    TimeSlot::Day day;
    if (dayOption == "Monday") day = TimeSlot::MONDAY;
    else if (dayOption == "Tuesday") day = TimeSlot::TUESDAY;
    else if (dayOption == "Wednesday") day = TimeSlot::WEDNESDAY;
    else if (dayOption == "Thursday") day = TimeSlot::THURSDAY;
    else day = TimeSlot::FRIDAY;
    
    auto timeSlot = std::make_shared<TimeSlot>(duration, day, startHour, startMinute);
    
    // Create SectionTimeSlotRequirement
    auto requirement = std::make_shared<SectionTimeSlotRequirement>(selectedSection, timeSlot);
    scheduler->addRequirement(requirement);
    
    // Clear input fields
    startHourInput->clear();
    startMinuteInput->clear();
    
    // Refresh requirement list
    refreshRequirementList();
}

// ScheduleViewerScreen implementation
ScheduleViewerScreen::ScheduleViewerScreen(std::shared_ptr<Scheduler> scheduler)
    : Screen(scheduler), currentScheduleIndex(0) {}

void ScheduleViewerScreen::initialize() {
    // Create a back button
    auto backButton = std::unique_ptr<Button>(new Button(
        20, 20, 100, 40, "Back", GRAY
    ));
    backButton->setOnClick([]() {
        // Will return to main menu in processInput
    });
    components.push_back(std::move(backButton));
    
    // Create a generate button
    auto generateButton = std::unique_ptr<Button>(new Button(
        140, 20, 150, 40, "Generate", GREEN
    ));
    generateButton->setOnClick([this]() {
        generateSchedules();
    });
    components.push_back(std::move(generateButton));
    
    // Add navigation buttons for multiple schedules
    auto prevButton = std::unique_ptr<Button>(new Button(
        300, 20, 120, 40, "Previous", BLUE
    ));
    prevButton->setOnClick([this]() {
        if (!displayedSchedules.empty() && currentScheduleIndex > 0) {
            currentScheduleIndex--;
            std::cout << "Previous button clicked. Current schedule index: " 
                      << currentScheduleIndex + 1 << "/" << displayedSchedules.size() << std::endl;
        }
    });
    components.push_back(std::move(prevButton));
    
    auto nextButton = std::unique_ptr<Button>(new Button(
        430, 20, 120, 40, "Next", BLUE
    ));
    nextButton->setOnClick([this]() {
        if (!displayedSchedules.empty() && currentScheduleIndex < static_cast<int>(displayedSchedules.size()) - 1) {
            currentScheduleIndex++;
            std::cout << "Next button clicked. Current schedule index: " 
                      << currentScheduleIndex + 1 << "/" << displayedSchedules.size() << std::endl;
        }
    });
    components.push_back(std::move(nextButton));
    
    // Add a View PQ Tree button
    auto viewPQTreeButton = std::unique_ptr<Button>(new Button(
        560, 20, 150, 40, "View PQ Tree", PURPLE
    ));
    viewPQTreeButton->setOnClick([]() {
        // Will show PQ tree in processInput
    });
    components.push_back(std::move(viewPQTreeButton));
}

void ScheduleViewerScreen::update() {
    // Nothing to update continuously
}

void ScheduleViewerScreen::draw() {
    DrawText("Schedule Viewer", 20, 70, 30, DARKBLUE);
    
    // Draw all UI components
    for (const auto& component : components) {
        component->draw();
    }
    
    // Draw placeholder text or schedule
    if (displayedSchedules.empty()) {
        DrawText("No schedules generated yet. Press 'Generate' to create schedules.", 200, 300, 20, GRAY);
    } else {
        // Display current schedule index information
        DrawText(("Schedule #" + std::to_string(currentScheduleIndex + 1) + " of " + 
                 std::to_string(displayedSchedules.size())).c_str(), 560, 70, 20, BLACK);
        
        drawScheduleGrid();
    }
}

ScreenState ScheduleViewerScreen::processInput() {
    // Check each component for input
    for (size_t i = 0; i < components.size(); i++) {
        if (components[i]->handleInput()) {
            // If the back button was clicked
            if (i == 0) {
                return ScreenState::MAIN_MENU;
            }
            // If the Generate button was clicked
            else if (i == 1) {
                // This will be handled by the button's onClick callback
            }
            // If the Previous button was clicked
            else if (i == 2) {
                // The navigation is already handled in the onClick callback
                // Do not update currentScheduleIndex again here
            }
            // If the Next button was clicked
            else if (i == 3) {
                // The navigation is already handled in the onClick callback
                // Do not update currentScheduleIndex again here
            }
            // If the View PQ Tree button was clicked
            else if (i == 4 && !displayedSchedules.empty()) {
                return ScreenState::PQ_TREE_VIEWER;
            }
        }
    }
    return ScreenState::SCHEDULE_VIEWER;
}

void ScheduleViewerScreen::generateSchedules() {
    // Generate schedules using the scheduler
    scheduler->generateSchedule();
    displayedSchedules = scheduler->getAllPossibleSchedules();
    currentScheduleIndex = 0;
}

void ScheduleViewerScreen::drawScheduleGrid() {
    if (displayedSchedules.empty() || currentScheduleIndex >= static_cast<int>(displayedSchedules.size())) {
        return;
    }
    
    // Grid constants
    const int gridStartX = 100;
    const int gridStartY = 120;
    const int timeColWidth = 100;
    const int dayColWidth = 200;
    const int rowHeight = 60;
    const int numTimeSlots = 10; // 8AM to 5PM
    const int daysPerWeek = 5; // Monday to Friday
    
    // Define grid colors
    Color gridLineColor = LIGHTGRAY;
    Color gridHeaderColor = LIGHTGRAY;
    Color classBlockColor = LIME;
    Color textColor = BLACK;
    
    // Define time labels
    std::vector<std::string> timeLabels = {
        "8:00AM", "9:00AM", "10:00AM", "11:00AM", "12:00PM",
        "1:00PM", "2:00PM", "3:00PM", "4:00PM", "5:00PM"
    };
    
    // Define day labels
    std::vector<std::string> dayLabels = {
        "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"
    };
    
    // Draw grid lines and headers
    
    // Draw time column headers (left column)
    DrawRectangle(gridStartX, gridStartY, timeColWidth, rowHeight, gridHeaderColor);
    DrawRectangleLines(gridStartX, gridStartY, timeColWidth, rowHeight, gridLineColor);
    DrawText("Time", gridStartX + 10, gridStartY + rowHeight/2 - 10, 20, textColor);
    
    // Draw day column headers (top row)
    for (int i = 0; i < daysPerWeek; i++) {
        int x = gridStartX + timeColWidth + i * dayColWidth;
        DrawRectangle(x, gridStartY, dayColWidth, rowHeight, gridHeaderColor);
        DrawRectangleLines(x, gridStartY, dayColWidth, rowHeight, gridLineColor);
        DrawText(dayLabels[i].c_str(), x + 10, gridStartY + rowHeight/2 - 10, 20, textColor);
    }
    
    // Draw time rows
    for (int i = 0; i < numTimeSlots; i++) {
        int y = gridStartY + rowHeight + i * rowHeight;
        
        // Draw time label
        DrawRectangle(gridStartX, y, timeColWidth, rowHeight, RAYWHITE);
        DrawRectangleLines(gridStartX, y, timeColWidth, rowHeight, gridLineColor);
        DrawText(timeLabels[i].c_str(), gridStartX + 10, y + rowHeight/2 - 10, 20, textColor);
        
        // Draw day cells
        for (int j = 0; j < daysPerWeek; j++) {
            int x = gridStartX + timeColWidth + j * dayColWidth;
            DrawRectangle(x, y, dayColWidth, rowHeight, RAYWHITE);
            DrawRectangleLines(x, y, dayColWidth, rowHeight, gridLineColor);
        }
    }
    
    // Draw classes on the grid - Make sure we're using the current schedule index
    auto schedule = displayedSchedules[currentScheduleIndex];
    for (const auto& section : schedule->getSections()) {
        auto timeSlot = section->getTimeSlot();
        auto startTime = timeSlot->getStartHour() * 60 + timeSlot->getStartMinute();
        auto duration = timeSlot->getDurationMinutes();
        auto day = timeSlot->getDay();
        
        // Skip if outside our grid
        if (timeSlot->getStartHour() < 8 || timeSlot->getStartHour() >= 17 || day > TimeSlot::FRIDAY) {
            continue;
        }
        
        // Calculate position in grid
        int dayIndex = static_cast<int>(day);
        int startHour = timeSlot->getStartHour();
        int startMin = timeSlot->getStartMinute();
        int startRowIndex = startHour - 8; // Grid starts at 8AM
        
        // Calculate fractional positioning for better precision
        float startYOffset = startMin / 60.0f * rowHeight;
        float durationHeight = (duration / 60.0f) * rowHeight;
        
        // Ensure minimum height
        durationHeight = std::max(durationHeight, 30.0f);
        
        // Draw class block
        int classX = gridStartX + timeColWidth + dayIndex * dayColWidth;
        int classY = gridStartY + rowHeight + startRowIndex * rowHeight + startYOffset;
        
        // Draw class block
        DrawRectangle(classX + 2, classY, dayColWidth - 4, durationHeight, classBlockColor);
        
        // Format time string
        char startTimeStr[20];
        char endTimeStr[20];
        
        int endHour = (startTime + duration) / 60;
        int endMin = (startTime + duration) % 60;
        
        bool startPM = startHour >= 12;
        bool endPM = endHour >= 12;
        
        // Convert to 12-hour format
        int startHour12 = startHour > 12 ? startHour - 12 : (startHour == 0 ? 12 : startHour);
        int endHour12 = endHour > 12 ? endHour - 12 : (endHour == 0 ? 12 : endHour);
        
        snprintf(startTimeStr, sizeof(startTimeStr), "%d:%02d%s", startHour12, startMin, startPM ? "PM" : "AM");
        snprintf(endTimeStr, sizeof(endTimeStr), "%d:%02d%s", endHour12, endMin, endPM ? "PM" : "AM");
        
        // Draw text within the class block
        int textY = classY + 5;
        
        // Course code and section
        std::string courseText = section->getCourse()->getCode() + " - " + section->getId();
        DrawText(courseText.c_str(), classX + 10, textY, 18, BLACK);
        textY += 20;
        
        // Teacher name
        std::string teacherText = section->getTeacher()->getName();
        DrawText(teacherText.c_str(), classX + 10, textY, 16, BLACK);
        textY += 16;
        
        // Time text
        std::string timeText = std::string(startTimeStr) + " - " + std::string(endTimeStr);
        DrawText(timeText.c_str(), classX + 10, textY, 16, BLACK);
    }
}

// Remove the unused function since drawScheduleGrid now handles everything
void ScheduleViewerScreen::drawSelectedSchedule() {
    // This function is no longer needed as drawScheduleGrid handles the display
}

// PQTreeViewerScreen implementation
PQTreeViewerScreen::PQTreeViewerScreen(std::shared_ptr<Scheduler> scheduler)
    : Screen(scheduler), zoomLevel(1.0f), panOffset({0, 0}), dragging(false) {}

void PQTreeViewerScreen::initialize() {
    // Create a back button
    auto backButton = std::unique_ptr<Button>(new Button(
        20, 20, 100, 40, "Back", GRAY
    ));
    backButton->setOnClick([]() {
        // Will return to schedule viewer in processInput
    });
    components.push_back(std::move(backButton));
    
    // Create zoom controls
    auto zoomInButton = std::unique_ptr<Button>(new Button(
        130, 20, 40, 40, "+", BLUE
    ));
    zoomInButton->setOnClick([this]() {
        zoomLevel *= 1.2f;
    });
    components.push_back(std::move(zoomInButton));
    
    auto zoomOutButton = std::unique_ptr<Button>(new Button(
        180, 20, 40, 40, "-", BLUE
    ));
    zoomOutButton->setOnClick([this]() {
        zoomLevel /= 1.2f;
        if (zoomLevel < 0.2f) zoomLevel = 0.2f;
    });
    components.push_back(std::move(zoomOutButton));
}

void PQTreeViewerScreen::update() {
    // Handle panning with mouse drag
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        
        // Check if not over UI components
        bool overComponent = false;
        for (const auto& component : components) {
            if (component->isMouseOver()) {
                overComponent = true;
                break;
            }
        }
        
        if (!overComponent) {
            if (!dragging) {
                lastMousePos = mousePos;
                dragging = true;
            } else {
                panOffset.x += (mousePos.x - lastMousePos.x);
                panOffset.y += (mousePos.y - lastMousePos.y);
                lastMousePos = mousePos;
            }
        }
    } else {
        dragging = false;
    }
    
    // Handle zooming with mouse wheel
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        zoomLevel *= (1.0f + wheel * 0.1f);
        if (zoomLevel < 0.2f) zoomLevel = 0.2f;
        if (zoomLevel > 5.0f) zoomLevel = 5.0f;
    }
}

void PQTreeViewerScreen::draw() {
    DrawText("PQ Tree Visualization", 20, 70, 30, DARKBLUE);
    
    // Draw all UI components
    for (const auto& component : components) {
        component->draw();
    }
    
    // Get the PQ tree from the scheduler
    PQTree pqTree = scheduler->buildSchedulePQTree();
    
    if (pqTree.getRoot()) {
        // Draw the PQ tree
        drawPQTree();
    } else {
        // Draw placeholder text
        DrawText("No PQ Tree available. Generate a schedule first.", 200, 300, 20, GRAY);
    }
    
    // Draw zoom level
    DrawText(("Zoom: " + std::to_string(zoomLevel)).c_str(), 230, 30, 20, BLACK);
}

ScreenState PQTreeViewerScreen::processInput() {
    // Check each component for input
    for (size_t i = 0; i < components.size(); i++) {
        if (components[i]->handleInput()) {
            // If the back button was clicked
            if (i == 0) {
                return ScreenState::SCHEDULE_VIEWER;
            }
        }
    }
    return ScreenState::PQ_TREE_VIEWER;
}

void PQTreeViewerScreen::drawPQTree() {
    // Get the PQ tree from the scheduler
    PQTree pqTree = scheduler->buildSchedulePQTree();
    auto root = pqTree.getRoot();
    
    if (!root) return;
    
    // Set the center position for the root
    Vector2 rootPos = {
        GetScreenWidth() / 2.0f + panOffset.x,
        150.0f + panOffset.y
    };
    
    // Draw the tree starting from the root
    drawNode(root, rootPos, zoomLevel);
}

void PQTreeViewerScreen::drawNode(std::shared_ptr<PQNode> node, Vector2 position, float scale) {
    if (!node) return;
    
    // Different colors for different node types
    Color nodeColor;
    float radius = 30.0f * scale;
    
    switch (node->getType()) {
        case NodeType::P_NODE:
            nodeColor = RED;
            break;
        case NodeType::Q_NODE:
            nodeColor = BLUE;
            break;
        case NodeType::LEAF:
            nodeColor = GREEN;
            break;
    }
    
    // Draw the node
    DrawCircle(position.x, position.y, radius, nodeColor);
    DrawCircleLines(position.x, position.y, radius, BLACK);
    
    // Draw the node type or label
    std::string label;
    if (node->getType() == NodeType::LEAF) {
        label = node->getLabel();
        
        // Shorten if too long
        if (label.length() > 15) {
            label = label.substr(0, 12) + "...";
        }
    } else {
        label = (node->getType() == NodeType::P_NODE) ? "P" : "Q";
        if (!node->getLabel().empty()) {
            label += ": " + node->getLabel();
        }
    }
    
    // Draw the label
    int fontSize = static_cast<int>(14 * scale);
    fontSize = std::max(fontSize, 10); // Ensure minimum readable size
    
    // Measure text width for centering
    float textWidth = MeasureText(label.c_str(), fontSize);
    Vector2 textPos = {
        position.x - textWidth / 2,
        position.y - fontSize / 2
    };
    
    DrawText(label.c_str(), textPos.x, textPos.y, fontSize, BLACK);
    
    // Draw connections to children
    const auto& children = node->getChildren();
    if (children.empty()) return;
    
    // Calculate layout for children
    float childSpacing = 100.0f * scale;
    float totalWidth = (children.size() - 1) * childSpacing;
    float startX = position.x - totalWidth / 2;
    float childY = position.y + 80.0f * scale;
    
    // Draw each child
    for (size_t i = 0; i < children.size(); i++) {
        float childX = startX + i * childSpacing;
        Vector2 childPos = {childX, childY};
        
        // Draw line to child
        DrawLine(position.x, position.y + radius, childPos.x, childPos.y - radius * 0.9f, BLACK);
        
        // Recursively draw the child node
        drawNode(children[i], childPos, scale);
    }
}

// New method to add dummy data
void UI::addDummyData() {
    // Create courses for Math, Computer, and English
    auto mathCourse = std::make_shared<Course>("MATH101", "Mathematics", 3);
    auto compCourse = std::make_shared<Course>("COMP101", "Computer Science", 3);
    auto engCourse = std::make_shared<Course>("ENG101", "English", 3);
    
    scheduler->addCourse(mathCourse);
    scheduler->addCourse(compCourse);
    scheduler->addCourse(engCourse);
    
    // Create teachers
    auto maria = std::make_shared<Teacher>("T001", "Miss Maria");
    auto qasim = std::make_shared<Teacher>("T002", "Sir Qasim");
    auto salman = std::make_shared<Teacher>("T003", "Sir Salman");
    auto hamna = std::make_shared<Teacher>("T004", "Miss Hamna");
    auto sara = std::make_shared<Teacher>("T005", "Miss Sara");
    
    scheduler->addTeacher(maria);
    scheduler->addTeacher(qasim);
    scheduler->addTeacher(salman);
    scheduler->addTeacher(hamna);
    scheduler->addTeacher(sara);
    
    // Create sections with only duration - day and start time will be dynamically assigned
    
    // Math sections
    auto mathTimeSlot1 = std::make_shared<TimeSlot>(60); // 60 min duration
    auto mathSection1 = std::make_shared<Section>("MATH101-A", mathCourse, maria, mathTimeSlot1);
    
    auto mathTimeSlot2 = std::make_shared<TimeSlot>(60); // 60 min duration
    auto mathSection2 = std::make_shared<Section>("MATH101-B", mathCourse, qasim, mathTimeSlot2);
    
    // Computer sections
    auto compTimeSlot1 = std::make_shared<TimeSlot>(90); // 90 min duration
    auto compSection1 = std::make_shared<Section>("COMP101-A", compCourse, salman, compTimeSlot1);
    
    auto compTimeSlot2 = std::make_shared<TimeSlot>(90); // 90 min duration
    auto compSection2 = std::make_shared<Section>("COMP101-B", compCourse, maria, compTimeSlot2);
    
    // English sections
    auto engTimeSlot1 = std::make_shared<TimeSlot>(75); // 75 min duration
    auto engSection1 = std::make_shared<Section>("ENG101-A", engCourse, hamna, engTimeSlot1);
    
    auto engTimeSlot2 = std::make_shared<TimeSlot>(75); // 75 min duration
    auto engSection2 = std::make_shared<Section>("ENG101-B", engCourse, sara, engTimeSlot2);
    
    // Add all sections to the scheduler
    scheduler->addSection(mathSection1);
    scheduler->addSection(mathSection2);
    scheduler->addSection(compSection1);
    scheduler->addSection(compSection2);
    scheduler->addSection(engSection1);
    scheduler->addSection(engSection2);
    
    // Add requirements that specific sections should be at specific times
    auto compSectionReq = std::make_shared<SectionTimeSlotRequirement>(compSection1, 
        std::make_shared<TimeSlot>(compSection1->getTimeSlot()->getDurationMinutes(), TimeSlot::MONDAY, 9, 0));  // COMP101-A on Monday at 9:00
    auto mathSectionReq = std::make_shared<SectionTimeSlotRequirement>(mathSection1, 
        std::make_shared<TimeSlot>(mathSection1->getTimeSlot()->getDurationMinutes(), TimeSlot::WEDNESDAY, 13, 0));  // MATH101-A on Wednesday at 13:00
    
    scheduler->addRequirement(compSectionReq);
    scheduler->addRequirement(mathSectionReq);
} 