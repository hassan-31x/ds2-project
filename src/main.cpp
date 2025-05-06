#include <iostream>
#include <memory>
#include "UI.hpp"

int main() {
    try {
        // Create and initialize the UI
        UI ui;
        ui.initialize();
        
        // Run the main loop
        ui.run();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
}
