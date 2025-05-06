#include "ui.h"
#include <sstream>
#include <iostream>
#include <algorithm>

// Button implementation
Button::Button(float x, float y, float width, float height, const
std::string& txt, Color col)
    : rect{x, y, width, height}, text(txt), color(col), isToggle(false),
    isToggled(false) {}

bool Button::isClicked() {
    if (CheckCollisionPointRec(GetMousePosition(), rect)) {
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            if (isToggle) {
                isToggled = !isToggled;
            }
            return true;
        }
    }
    return false;
}

void Button::draw() {
    Color bgColor = color;
    Color textColor = COLOR_LIGHT_TEXT;

    if (isToggle && isToggled) {
        bgColor = COLOR_SECONDARY;
    } else if (CheckCollisionPointRec(GetMousePosition(), rect)) {
        bgColor = ColorBrightness(color, 0.2f);
    }

    DrawRectangleRec(rect, bgColor);
    DrawRectangleLinesEx(rect, 1, ColorAlpha(COLOR_BORDER, 0.5f));

    // Center text
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text.c_str(),
    FONT_SIZE, 1); float textX = rect.x + (rect.width - textSize.x) / 2;
    float textY = rect.y + (rect.height - textSize.y) / 2;

    DrawTextEx(GetFontDefault(), text.c_str(), {textX, textY}, FONT_SIZE, 1,
    textColor);
}

// InputField implementation
InputField::InputField(float x, float y, float width, float height, const
std::string& lbl, int maxLen)
    : rect{x, y, width, height}, label(lbl), text(""), isActive(false),
    maxLength(maxLen) {}

void InputField::update() {
    if (CheckCollisionPointRec(GetMousePosition(), rect)) {
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            isActive = true;
        }
    } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        isActive = false;
    }

    if (isActive) {
        int key = GetCharPressed();

        // Add character to text if within maxLength
        if (key > 0 && text.length() < maxLength) {
            text += (char)key;
        }

        // Handle backspace
        if (IsKeyPressed(KEY_BACKSPACE) && !text.empty()) {
            text.pop_back();
        }
    }
}

void InputField::draw() {
    Color bgColor = COLOR_BG;
    Color borderColor = COLOR_BORDER;

    if (isActive) {
        bgColor = COLOR_HIGHLIGHT;
        borderColor = COLOR_PRIMARY;
    } else if (CheckCollisionPointRec(GetMousePosition(), rect)) {
        bgColor = ColorBrightness(COLOR_BG, 0.8f);
    }

    // Draw field background
    DrawRectangleRec(rect, bgColor);
    DrawRectangleLinesEx(rect, 1, borderColor);

    // Draw label above field
    DrawTextEx(GetFontDefault(), label.c_str(), {rect.x, rect.y - 20},
    FONT_SIZE, 1, COLOR_TEXT);

    // Draw text
    DrawTextEx(GetFontDefault(), text.c_str(), {rect.x + 5, rect.y +
    (rect.height - FONT_SIZE) / 2}, FONT_SIZE, 1, COLOR_TEXT);

    // Draw cursor if active
    if (isActive) {
        float textWidth = MeasureTextEx(GetFontDefault(), text.c_str(),
        FONT_SIZE, 1).x; DrawRectangle(rect.x + 5 + textWidth, rect.y +
        (rect.height - FONT_SIZE) / 2, 2, FONT_SIZE, COLOR_TEXT);
    }
}

std::string InputField::getValue() const {
    return text;
}

// Dropdown implementation
Dropdown::Dropdown(float x, float y, float width, float height, const
std::string& lbl, const std::vector<std::string>& opts)
    : rect{x, y, width, height}, label(lbl), options(opts), selectedIndex(0),
    isOpen(false) {}

void Dropdown::update() {
    if (CheckCollisionPointRec(GetMousePosition(), rect)) {
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            isOpen = !isOpen;
        }
    } else if (isOpen) {
        // Check if clicked outside the dropdown options
        bool clickedOption = false;
        for (int i = 0; i < options.size(); i++) {
            Rectangle optionRect = {rect.x, rect.y + rect.height + i *
            FONT_SIZE * 1.5f, rect.width, FONT_SIZE * 1.5f}; if
            (CheckCollisionPointRec(GetMousePosition(), optionRect)) {
                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                    selectedIndex = i;
                    isOpen = false;
                    clickedOption = true;
                    break;
                }
            }
        }

        if (!clickedOption && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            isOpen = false;
        }
    }
}

void Dropdown::draw() {
    Color bgColor = COLOR_BG;
    Color borderColor = COLOR_BORDER;

    if (CheckCollisionPointRec(GetMousePosition(), rect)) {
        bgColor = ColorBrightness(COLOR_BG, 0.8f);
    }

    // Draw dropdown background
    DrawRectangleRec(rect, bgColor);
    DrawRectangleLinesEx(rect, 1, borderColor);

    // Draw label above dropdown
    DrawTextEx(GetFontDefault(), label.c_str(), {rect.x, rect.y - 20},
    FONT_SIZE, 1, COLOR_TEXT);

    // Draw selected option
    std::string displayText = selectedIndex >= 0 && selectedIndex <
    options.size()
                             ? options[selectedIndex] : "Select...";
    DrawTextEx(GetFontDefault(), displayText.c_str(), {rect.x + 5, rect.y +
    (rect.height - FONT_SIZE) / 2},
              FONT_SIZE, 1, COLOR_TEXT);

    // Draw dropdown arrow
    DrawTriangle({rect.x + rect.width - 15, rect.y + rect.height / 3},
                {rect.x + rect.width - 5, rect.y + rect.height / 3},
                {rect.x + rect.width - 10, rect.y + 2 * rect.height / 3},
                COLOR_TEXT);

    // Draw options if open
    if (isOpen) {
        for (int i = 0; i < options.size(); i++) {
            Rectangle optionRect = {rect.x, rect.y + rect.height + i *
            FONT_SIZE * 1.5f, rect.width, FONT_SIZE * 1.5f}; Color optBgColor
            = (i == selectedIndex) ? COLOR_PRIMARY : COLOR_BG; Color
            optTextColor = (i == selectedIndex) ? COLOR_LIGHT_TEXT :
            COLOR_TEXT;

            if (CheckCollisionPointRec(GetMousePosition(), optionRect)) {
                optBgColor = (i == selectedIndex) ?
                ColorBrightness(COLOR_PRIMARY, 0.2f) : COLOR_HIGHLIGHT;
            }

            DrawRectangleRec(optionRect, optBgColor);
            DrawRectangleLinesEx(optionRect, 1, COLOR_BORDER);
            DrawTextEx(GetFontDefault(), options[i].c_str(), {optionRect.x +
            5, optionRect.y + (optionRect.height - FONT_SIZE) / 2},
                      FONT_SIZE, 1, optTextColor);
        }
    }
}

int Dropdown::getSelectedIndex() const {
    return selectedIndex;
}

std::string Dropdown::getSelectedOption() const {
    return (selectedIndex >= 0 && selectedIndex < options.size()) ?
    options[selectedIndex] : "";
}

// ScheduleUI implementation
ScheduleUI::ScheduleUI()
    : scheduler(std::make_shared<ClassScheduler>()),
      currentTab(Tab::COURSES),
      isDragging(false),
      draggedSection(nullptr),
      dragOffset({0, 0}),
      addCourseButton(0, 0, 0, 0, "", BLACK),
      addTeacherButton(0, 0, 0, 0, "", BLACK),
      addSectionButton(0, 0, 0, 0, "", BLACK),
      courseDropdown(0, 0, 0, 0, "", {}),
      teacherDropdown(0, 0, 0, 0, "", {}),
      preferenceTypeDropdown(0, 0, 0, 0, "", {}),
      preferenceCourseDropdown(0, 0, 0, 0, "", {}),
      preferenceTeacherDropdown(0, 0, 0, 0, "", {}),
      addPreferenceButton(0, 0, 0, 0, "", BLACK) {}

ScheduleUI::~ScheduleUI() {
    scheduler.reset();
}

void ScheduleUI::initialize() {
    // Create tab buttons
    float tabWidth = SCREEN_WIDTH / 4.0f;
    tabButtons.push_back(Button(0, 0, tabWidth, 40, "Courses",
    COLOR_PRIMARY)); tabButtons.push_back(Button(tabWidth, 0, tabWidth, 40,
    "Teachers", COLOR_PRIMARY)); tabButtons.push_back(Button(tabWidth * 2, 0,
    tabWidth, 40, "Schedule", COLOR_PRIMARY));
    tabButtons.push_back(Button(tabWidth * 3, 0, tabWidth, 40, "Preferences",
    COLOR_PRIMARY));

    // Mark the first tab button as toggled
    tabButtons[0].isToggle = true;
    tabButtons[0].isToggled = true;
    tabButtons[1].isToggle = true;
    tabButtons[2].isToggle = true;
    tabButtons[3].isToggle = true;

    // Create action buttons
    actionButtons.push_back(Button(SCREEN_WIDTH - 200, SCREEN_HEIGHT - 50,
    180, 40, "Generate Schedule", COLOR_ACCENT));

    // Initialize course tab components
    courseInputs.push_back(InputField(PADDING, 80, 300, 40, "Course Code
    (e.g., CS101)")); courseInputs.push_back(InputField(PADDING, 150, 300,
    40, "Course Title")); courseInputs.push_back(InputField(PADDING, 220,
    300, 40, "Credit Hours")); addCourseButton = Button(PADDING, 290, 300,
    40, "Add Course", COLOR_PRIMARY);

    // Initialize teacher tab components
    teacherInputs.push_back(InputField(PADDING, 80, 300, 40, "Teacher ID"));
    teacherInputs.push_back(InputField(PADDING, 150, 300, 40, "Teacher
    Name")); addTeacherButton = Button(PADDING, 290, 300, 40, "Add Teacher",
    COLOR_PRIMARY);

    // Initialize section tab components
    sectionInputs.push_back(InputField(PADDING, 80, 300, 40, "Section ID
    (e.g., A, B, C)"));

    // Course and teacher dropdowns will be populated when needed
    courseDropdown = Dropdown(PADDING, 150, 300, 40, "Select Course", {});
    teacherDropdown = Dropdown(PADDING, 220, 300, 40, "Select Teacher", {});
    addSectionButton = Button(PADDING, 290, 300, 40, "Add Section",
    COLOR_PRIMARY);

    // Initialize preference tab components
    std::vector<std::string> prefTypes = {"Prefer Teacher", "Prefer Time
    Slot", "Avoid Teacher", "Avoid Time Slot"}; preferenceTypeDropdown =
    Dropdown(PADDING, 80, 300, 40, "Preference Type", prefTypes);
    preferenceCourseDropdown = Dropdown(PADDING, 150, 300, 40, "Select
    Course", {}); preferenceTeacherDropdown = Dropdown(PADDING, 220, 300, 40,
    "Select Teacher", {});

    // Initialize time slot selector buttons (5 days x 10 hours)
    for (int day = 0; day < 5; day++) {
        for (int hour = 8; hour <= 17; hour++) {
            float x = PADDING + day * 60;
            float y = 290 + (hour - 8) * 30;
            std::string label = TimeSlot::dayToString(day).substr(0, 3) + " "
            + std::to_string(hour); preferenceTimeButtons.push_back(Button(x,
            y, 55, 25, label, COLOR_BORDER));
            preferenceTimeButtons.back().isToggle = true;
        }
    }

    addPreferenceButton = Button(PADDING, 600, 300, 40, "Add Preference",
    COLOR_PRIMARY);

    // Initialize schedule viewport
    scheduleViewport = {DAY_HEADER_WIDTH, TIME_HEADER_HEIGHT,
                        SCREEN_WIDTH - DAY_HEADER_WIDTH,
                        SCREEN_HEIGHT - TIME_HEADER_HEIGHT};

    // Load some demo data
    loadDemoData();
}

void ScheduleUI::run() {
    // Initialize the window and graphics
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Class Scheduler with PQ-Trees");
    SetTargetFPS(60);

    initialize();

    // Main game loop
    while (!WindowShouldClose()) {
        update();

        BeginDrawing();
        ClearBackground(COLOR_BG);

        draw();

        EndDrawing();
    }

    shutdown();
}

void ScheduleUI::shutdown() {
    CloseWindow();
}

void ScheduleUI::update() {
    // Update tab buttons
    for (int i = 0; i < tabButtons.size(); i++) {
        if (tabButtons[i].isClicked()) {
            // Untoggle all other tab buttons
            for (int j = 0; j < tabButtons.size(); j++) {
                if (i != j) {
                    tabButtons[j].isToggled = false;
                }
            }

            // Set current tab
            currentTab = static_cast<Tab>(i);
        }
    }

    // Update action buttons
    if (actionButtons[0].isClicked()) {
        generateSchedule();
    }

    // Update tab-specific components
    switch (currentTab) {
        case Tab::COURSES:
            // Update course inputs
            for (auto& input : courseInputs) {
                input.update();
            }

            // Check if Add Course button is clicked
            if (addCourseButton.isClicked()) {
                addCourse();
            }
            break;

        case Tab::TEACHERS:
            // Update teacher inputs
            for (auto& input : teacherInputs) {
                input.update();
            }

            // Check if Add Teacher button is clicked
            if (addTeacherButton.isClicked()) {
                addTeacher();
            }
            break;

        case Tab::SCHEDULE:
            // Handle drag and drop of schedule items
            handleScheduleDrag();
            break;

        case Tab::PREFERENCES:
            // Update preference components
            preferenceTypeDropdown.update();
            preferenceCourseDropdown.update();
            preferenceTeacherDropdown.update();

            // Update time slot buttons only if time slot preference is
            selected if (preferenceTypeDropdown.getSelectedIndex() == 1 ||
            preferenceTypeDropdown.getSelectedIndex() == 3) {
                for (auto& button : preferenceTimeButtons) {
                    button.isClicked(); // Just to update toggle state
                }
            }

            // Check if Add Preference button is clicked
            if (addPreferenceButton.isClicked()) {
                addPreference();
            }
            break;
    }

    // Populate dropdowns with updated data if needed
    std::vector<std::string> courseOptions;
    for (const auto& course : scheduler->getCourses()) {
        courseOptions.push_back(course->code + ": " + course->title);
    }

    std::vector<std::string> teacherOptions;
    for (const auto& teacher : scheduler->getTeachers()) {
        teacherOptions.push_back(teacher->id + ": " + teacher->name);
    }

    // Update dropdowns if data changed
    if (courseDropdown.options != courseOptions) {
        courseDropdown.options = courseOptions;
    }

    if (teacherDropdown.options != teacherOptions) {
        teacherDropdown.options = teacherOptions;
    }

    if (preferenceCourseDropdown.options != courseOptions) {
        preferenceCourseDropdown.options = courseOptions;
    }

    if (preferenceTeacherDropdown.options != teacherOptions) {
        preferenceTeacherDropdown.options = teacherOptions;
    }
}

void ScheduleUI::draw() {
    // Draw tab buttons
    for (auto& button : tabButtons) {
        button.draw();
    }

    // Draw tab content
    switch (currentTab) {
        case Tab::COURSES:
            drawCoursesTab();
            break;
        case Tab::TEACHERS:
            drawTeachersTab();
            break;
        case Tab::SCHEDULE:
            drawScheduleTab();
            break;
        case Tab::PREFERENCES:
            drawPreferencesTab();
            break;
    }

    // Draw action buttons (always visible)
    for (auto& button : actionButtons) {
        button.draw();
    }
}

void ScheduleUI::drawCoursesTab() {
    // Draw heading
    DrawTextEx(GetFontDefault(), "Courses", {PADDING, 50}, HEADER_FONT_SIZE,
    1, COLOR_TEXT);

    // Draw input fields
    for (auto& input : courseInputs) {
        input.draw();
    }

    // Draw Add Course button
    addCourseButton.draw();

    // Draw list of existing courses
    DrawTextEx(GetFontDefault(), "Existing Courses:", {PADDING, 350},
    FONT_SIZE, 1, COLOR_TEXT);

    int y = 380;
    for (const auto& course : scheduler->getCourses()) {
        std::stringstream ss;
        ss << course->code << ": " << course->title << " (" <<
        course->creditHours << " credits)";

        DrawTextEx(GetFontDefault(), ss.str().c_str(), {PADDING, (float)y},
        FONT_SIZE, 1, COLOR_TEXT); y += FONT_SIZE + 5;
    }
}

void ScheduleUI::drawTeachersTab() {
    // Draw heading
    DrawTextEx(GetFontDefault(), "Teachers", {PADDING, 50}, HEADER_FONT_SIZE,
    1, COLOR_TEXT);

    // Draw input fields
    for (auto& input : teacherInputs) {
        input.draw();
    }

    // Draw Add Teacher button
    addTeacherButton.draw();

    // Draw list of existing teachers
    DrawTextEx(GetFontDefault(), "Existing Teachers:", {PADDING, 350},
    FONT_SIZE, 1, COLOR_TEXT);

    int y = 380;
    for (const auto& teacher : scheduler->getTeachers()) {
        std::stringstream ss;
        ss << teacher->id << ": " << teacher->name;

        DrawTextEx(GetFontDefault(), ss.str().c_str(), {PADDING, (float)y},
        FONT_SIZE, 1, COLOR_TEXT); y += FONT_SIZE + 5;
    }
}

void ScheduleUI::drawScheduleTab() {
    // Draw heading
    DrawTextEx(GetFontDefault(), "Class Schedule", {PADDING, 50},
    HEADER_FONT_SIZE, 1, COLOR_TEXT);

    // Draw schedule grid
    drawScheduleGrid();

    // Draw schedule items
    drawScheduleItems();
}

void ScheduleUI::drawPreferencesTab() {
    // Draw heading
    DrawTextEx(GetFontDefault(), "Preferences", {PADDING, 50},
    HEADER_FONT_SIZE, 1, COLOR_TEXT);

    // Draw preference components
    preferenceTypeDropdown.draw();
    preferenceCourseDropdown.draw();

    // Show teacher dropdown or time slots based on preference type
    if (preferenceTypeDropdown.getSelectedIndex() == 0 ||
    preferenceTypeDropdown.getSelectedIndex() == 2) {
        // Teacher preference
        preferenceTeacherDropdown.draw();
    } else if (preferenceTypeDropdown.getSelectedIndex() == 1 ||
    preferenceTypeDropdown.getSelectedIndex() == 3) {
        // Time slot preference
        DrawTextEx(GetFontDefault(), "Select Time Slot:", {PADDING, 290 -
        20}, FONT_SIZE, 1, COLOR_TEXT);

        for (auto& button : preferenceTimeButtons) {
            button.draw();
        }
    }

    // Draw Add Preference button
    addPreferenceButton.draw();
}

void ScheduleUI::drawScheduleGrid() {
    // Draw the time headers (8:00 - 17:00)
    for (int hour = 8; hour <= 17; hour++) {
        std::string timeLabel = std::to_string(hour) + ":00";
        float y = TIME_HEADER_HEIGHT + (hour - 8) * CELL_HEIGHT;

        DrawRectangle(0, y, DAY_HEADER_WIDTH, CELL_HEIGHT, COLOR_HIGHLIGHT);
        DrawRectangleLines(0, y, DAY_HEADER_WIDTH, CELL_HEIGHT,
        COLOR_BORDER);

        // Center text in cell
        Vector2 textSize = MeasureTextEx(GetFontDefault(), timeLabel.c_str(),
        FONT_SIZE, 1); float textX = (DAY_HEADER_WIDTH - textSize.x) / 2;
        float textY = y + (CELL_HEIGHT - textSize.y) / 2;

        DrawTextEx(GetFontDefault(), timeLabel.c_str(), {textX, textY},
        FONT_SIZE, 1, COLOR_TEXT);
    }

    // Draw the day headers (Monday - Friday)
    const char* days[] = {"Monday", "Tuesday", "Wednesday", "Thursday",
    "Friday"}; float dayWidth = (SCREEN_WIDTH - DAY_HEADER_WIDTH) / 5;

    for (int day = 0; day < 5; day++) {
        float x = DAY_HEADER_WIDTH + day * dayWidth;

        DrawRectangle(x, 0, dayWidth, TIME_HEADER_HEIGHT, COLOR_HIGHLIGHT);
        DrawRectangleLines(x, 0, dayWidth, TIME_HEADER_HEIGHT, COLOR_BORDER);

        // Center text in cell
        Vector2 textSize = MeasureTextEx(GetFontDefault(), days[day],
        FONT_SIZE, 1); float textX = x + (dayWidth - textSize.x) / 2; float
        textY = (TIME_HEADER_HEIGHT - textSize.y) / 2;

        DrawTextEx(GetFontDefault(), days[day], {textX, textY}, FONT_SIZE, 1,
        COLOR_TEXT);
    }

    // Draw the grid cells
    for (int day = 0; day < 5; day++) {
        for (int hour = 8; hour <= 17; hour++) {
            float x = DAY_HEADER_WIDTH + day * dayWidth;
            float y = TIME_HEADER_HEIGHT + (hour - 8) * CELL_HEIGHT;

            DrawRectangleLines(x, y, dayWidth, CELL_HEIGHT, COLOR_BORDER);
        }
    }
}

void ScheduleUI::drawScheduleItems() {
    float dayWidth = (SCREEN_WIDTH - DAY_HEADER_WIDTH) / 5;

    // Draw sections on the schedule
    for (const auto& section : scheduler->getSections()) {
        if (!section->teacher || section->timeSlots.empty()) continue;

        for (const auto& slot : section->timeSlots) {
            float x = DAY_HEADER_WIDTH + slot.day * dayWidth;
            float y = TIME_HEADER_HEIGHT + (slot.hour - 8) * CELL_HEIGHT;
            float height = slot.duration * CELL_HEIGHT;

            // Draw section block
            DrawRectangle(x + 1, y + 1, dayWidth - 2, height - 2,
            COLOR_PRIMARY);

            // Draw section text
            std::stringstream ss;
            ss << section->course->code << " (" << section->id << ")\n";
            ss << section->teacher->name << "\n";
            ss << slot.toString();

            DrawTextEx(GetFontDefault(), ss.str().c_str(), {x + 5, y + 5},
            FONT_SIZE, 1, COLOR_LIGHT_TEXT);
        }
    }
}

void ScheduleUI::handleScheduleDrag() {
    float dayWidth = (SCREEN_WIDTH - DAY_HEADER_WIDTH) / 5;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Check if clicked on a section
        Vector2 mousePos = GetMousePosition();

        for (const auto& section : scheduler->getSections()) {
            if (!section->teacher || section->timeSlots.empty()) continue;

            for (const auto& slot : section->timeSlots) {
                float x = DAY_HEADER_WIDTH + slot.day * dayWidth;
                float y = TIME_HEADER_HEIGHT + (slot.hour - 8) * CELL_HEIGHT;
                float height = slot.duration * CELL_HEIGHT;

                Rectangle slotRect = {x, y, dayWidth, height};

                if (CheckCollisionPointRec(mousePos, slotRect)) {
                    isDragging = true;
                    draggedSection = section;
                    dragOffset = {mousePos.x - x, mousePos.y - y};
                    break;
                }
            }

            if (isDragging) break;
        }
    } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && isDragging) {
        // Drop the section at new position
        Vector2 mousePos = GetMousePosition();

        // Calculate the day and hour from mouse position
        int day = (mousePos.x - DAY_HEADER_WIDTH) / dayWidth;
        int hour = (mousePos.y - TIME_HEADER_HEIGHT) / CELL_HEIGHT + 8;

        // Clamp values to valid range
        day = (day < 0) ? 0 : (day > 4) ? 4 : day;
        hour = (hour < 8) ? 8 : (hour > 17) ? 17 : hour;

        // Update the section's time slot
        if (draggedSection && !draggedSection->timeSlots.empty()) {
            TimeSlot newSlot = draggedSection->timeSlots[0];
            newSlot.day = day;
            newSlot.hour = hour;

            // Check if this slot is valid (teacher is available, no
            conflicts) bool isValid = true;

            // TODO: Add validation logic

            if (isValid) {
                draggedSection->timeSlots[0] = newSlot;
            }
        }

        // Reset drag state
        isDragging = false;
        draggedSection = nullptr;
    }
}

void ScheduleUI::addCourse() {
    if (courseInputs.size() >= 3) {
        std::string code = courseInputs[0].getValue();
        std::string title = courseInputs[1].getValue();
        int credits = 3; // Default

        try {
            credits = std::stoi(courseInputs[2].getValue());
        } catch (...) {
            // Use default value if parsing fails
        }

        if (!code.empty() && !title.empty()) {
            auto course = std::make_shared<Course>(code, title, credits);
            scheduler->addCourse(course);

            // Clear inputs
            for (auto& input : courseInputs) {
                input.text.clear();
            }
        }
    }
}

void ScheduleUI::addTeacher() {
    if (teacherInputs.size() >= 2) {
        std::string id = teacherInputs[0].getValue();
        std::string name = teacherInputs[1].getValue();

        if (!id.empty() && !name.empty()) {
            auto teacher = std::make_shared<Teacher>(id, name);

            // Add default available time slots (all day, all week)
            for (int day = 0; day < 5; day++) {
                for (int hour = 8; hour <= 16; hour++) {
                    TimeSlot slot;
                    slot.day = day;
                    slot.hour = hour;
                    slot.duration = 1;
                    teacher->addAvailableTimeSlot(slot);
                }
            }

            scheduler->addTeacher(teacher);

            // Clear inputs
            for (auto& input : teacherInputs) {
                input.text.clear();
            }
        }
    }
}

void ScheduleUI::addSection() {
    std::string sectionId = sectionInputs[0].getValue();
    int courseIndex = courseDropdown.getSelectedIndex();
    int teacherIndex = teacherDropdown.getSelectedIndex();

    if (!sectionId.empty() && courseIndex >= 0 && courseIndex <
    scheduler->getCourses().size() &&
        teacherIndex >= 0 && teacherIndex < scheduler->getTeachers().size())
        {

        auto course = scheduler->getCourses()[courseIndex];
        auto teacher = scheduler->getTeachers()[teacherIndex];

        // Assign teacher to course if not already assigned
        if (std::find(course->assignedTeachers.begin(),
        course->assignedTeachers.end(), teacher) ==
        course->assignedTeachers.end()) {
            course->assignTeacher(teacher);
        }

        // Create section
        auto section = std::make_shared<Section>(sectionId, course);
        section->assignTeacher(teacher);

        scheduler->addSection(section);

        // Clear inputs
        sectionInputs[0].text.clear();
    }
}

void ScheduleUI::addPreference() {
    int prefType = preferenceTypeDropdown.getSelectedIndex();
    int courseIndex = preferenceCourseDropdown.getSelectedIndex();

    if (prefType >= 0 && courseIndex >= 0 && courseIndex <
    scheduler->getCourses().size()) {
        StudentPreference pref;
        pref.courseCode = scheduler->getCourses()[courseIndex]->code;
        pref.weight = 1.0f; // Default weight

        switch (prefType) {
            case 0: // Prefer Teacher
            case 2: // Avoid Teacher
                {
                    int teacherIndex =
                    preferenceTeacherDropdown.getSelectedIndex(); if
                    (teacherIndex >= 0 && teacherIndex <
                    scheduler->getTeachers().size()) {
                        pref.type = (prefType == 0) ?
                        StudentPreference::PREFER_TEACHER :
                        StudentPreference::AVOID_TEACHER; pref.teacherId =
                        scheduler->getTeachers()[teacherIndex]->id;
                        scheduler->addPreference(pref);
                    }
                }
                break;

            case 1: // Prefer Time Slot
            case 3: // Avoid Time Slot
                {
                    // Find the selected time slot button
                    for (int i = 0; i < preferenceTimeButtons.size(); i++) {
                        if (preferenceTimeButtons[i].isToggled) {
                            int day = i / 10;  // 10 time slots per day
                            int hour = (i % 10) + 8;

                            pref.type = (prefType == 1) ?
                            StudentPreference::PREFER_TIME_SLOT :
                            StudentPreference::AVOID_TIME_SLOT;
                            pref.timeSlot.day = day;
                            pref.timeSlot.hour = hour;
                            pref.timeSlot.duration = 1;

                            scheduler->addPreference(pref);

                            // Reset button state
                            preferenceTimeButtons[i].isToggled = false;
                            break;
                        }
                    }
                }
                break;
        }
    }
}

void ScheduleUI::generateSchedule() {
    bool success = scheduler->generateSchedule();

    if (success) {
        // Switch to schedule tab to show the result
        currentTab = Tab::SCHEDULE;
        tabButtons[0].isToggled = false;
        tabButtons[1].isToggled = false;
        tabButtons[2].isToggled = true;
        tabButtons[3].isToggled = false;
    }
}

void ScheduleUI::loadDemoData() {
    // Create some courses
    auto cs101 = std::make_shared<Course>("CS101", "Introduction to
    Programming", 3); auto cs201 = std::make_shared<Course>("CS201", "Data
    Structures", 4); auto math101 = std::make_shared<Course>("MATH101",
    "Calculus I", 3);

    scheduler->addCourse(cs101);
    scheduler->addCourse(cs201);
    scheduler->addCourse(math101);

    // Create some teachers
    auto smith = std::make_shared<Teacher>("T1", "John Smith");
    auto jones = std::make_shared<Teacher>("T2", "Sarah Jones");
    auto wilson = std::make_shared<Teacher>("T3", "Robert Wilson");

    // Add available time slots for teachers
    for (int day = 0; day < 5; day++) {
        for (int hour = 8; hour <= 16; hour++) {
            TimeSlot slot;
            slot.day = day;
            slot.hour = hour;
            slot.duration = 1;

            smith->addAvailableTimeSlot(slot);
            jones->addAvailableTimeSlot(slot);
            wilson->addAvailableTimeSlot(slot);
        }
    }

    scheduler->addTeacher(smith);
    scheduler->addTeacher(jones);
    scheduler->addTeacher(wilson);

    // Assign teachers to courses
    cs101->assignTeacher(smith);
    cs101->assignTeacher(jones);
    cs201->assignTeacher(jones);
    math101->assignTeacher(wilson);

    // Create some sections
    auto cs101A = std::make_shared<Section>("A", cs101);
    auto cs101B = std::make_shared<Section>("B", cs101);
    auto cs201A = std::make_shared<Section>("A", cs201);
    auto math101A = std::make_shared<Section>("A", math101);

    scheduler->addSection(cs101A);
    scheduler->addSection(cs101B);
    scheduler->addSection(cs201A);
    scheduler->addSection(math101A);

    // Add some preferences
    StudentPreference pref1;
    pref1.type = StudentPreference::PREFER_TEACHER;
    pref1.courseCode = "CS101";
    pref1.teacherId = "T1";
    pref1.weight = 0.8f;

    StudentPreference pref2;
    pref2.type = StudentPreference::AVOID_TIME_SLOT;
    pref2.courseCode = "MATH101";
    pref2.timeSlot.day = 0;  // Monday
    pref2.timeSlot.hour = 8; // 8:00 AM
    pref2.timeSlot.duration = 1;
    pref2.weight = 0.9f;

    scheduler->addPreference(pref1);
    scheduler->addPreference(pref2);

    // Generate an initial schedule
    scheduler->generateSchedule();
}