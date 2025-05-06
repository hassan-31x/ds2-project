// #include "ui.h"
// #include <iostream>

// int main()
// {
//     try
//     {
//         // Initialize and launch the scheduling interface
//         ScheduleUI userInterface;
//         userInterface.run();

//         // Exit successfully if everything completes
//         return 0;
//     }
//     // Handle known error conditions
//     catch (const std::exception &error)
//     {
//         std::cerr << "Application error: " << error.what() << std::endl;
//         return 1;
//     }
//     // Catch any unexpected exceptions
//     catch (...)
//     {
//         std::cerr << "Unexpected error occurred" << std::endl;
//         return 1;
//     }
// }