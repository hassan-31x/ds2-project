# Class Scheduling System with PQ-Trees

This project implements a class scheduling system using PQ-Trees for constraint-based scheduling optimization. It features a Raylib-based graphical user interface that allows users to manage courses, teachers, sections, and student preferences.

## Features

- **PQ-Tree Implementation**: Efficient data structure for maintaining ordering constraints
- **Class Scheduling Logic**: Algorithm to generate optimized schedules
- **Constraint Management**: Support for various scheduling constraints
- **Student Preferences**: Ability to specify preferred/avoided teachers and time slots
- **Interactive UI**: Graphical interface for managing all aspects of scheduling
- **Drag-and-Drop Schedule Editing**: Intuitive schedule manipulation

## Dependencies

- [Raylib](https://www.raylib.com/): Simple and easy-to-use library for game development
- C++11 or later

## Building the Project

### Using Make

```bash
make
```

### Manual Compilation

```bash
g++ -o scheduler src/*.cpp -Iinclude -Lraylib -lraylib -std=c++11
```

## Usage

Run the compiled executable:

```bash
./scheduler
```

### Interface

The application has four main tabs:

1. **Courses**: Add and manage courses
2. **Teachers**: Add and manage teachers
3. **Schedule**: View and modify the generated schedule
4. **Preferences**: Add student preferences for courses, teachers, and time slots

### Quick Start

1. Add courses using the Courses tab
2. Add teachers using the Teachers tab
3. Create sections by assigning courses to teachers
4. Add student preferences if desired
5. Click "Generate Schedule" to create an optimized schedule
6. View and modify the schedule in the Schedule tab

## PQ-Tree Details

PQ-Trees are used to represent the constraints on the ordering of time slots in the schedule. They allow for efficient constraint checking and enforcement:

- **P-Nodes**: Children can be reordered in any way (representing flexible time slots)
- **Q-Nodes**: Children can only be reversed (representing consecutive time slots)
- **Leaf Nodes**: Represent individual time slots

The scheduling algorithm uses PQ-Trees to ensure that all constraints are satisfied.

## Class Scheduling Logic

The scheduling algorithm takes into account:

- Teacher availability
- Course credit hours
- Section requirements
- Student preferences
- Scheduling constraints (e.g., consecutive slots for multi-hour courses)

## Future Enhancements

- More sophisticated PQ-Tree reduction algorithm
- Advanced constraint types
- Room assignment functionality
- Student enrollment management
- Schedule exporting (PDF, CSV)
- Calendar integration

## License

MIT

## Authors

- Your Name 