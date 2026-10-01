#include <cstddef>
#include <iomanip>
#include <ios>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>
#include <thread>


#include "error_handler.h"
#include "settings.h"


Setting::Setting(ErrorHandler& error_handler, int argc, char *argv[]) {
    #ifdef GUI
    gui = true;
    #endif // GUI

    std::vector<std::string> arguments(argv, argv+argc);

    // get root path
    std::string temp_root_path = arguments[0];
    std::size_t executable_place = temp_root_path.find_last_of("/\\");
    temp_root_path = temp_root_path.substr(0, executable_place);
    temp_root_path.append("/../../");
    rootPath = temp_root_path;

    for (std::string argument : arguments | std::views::drop(1)) {
        if (argument.find("--help") == 0) {
            std::stringstream help_description;
            const int align = 24;
            help_description << "help:" << std::endl << std::left;
            help_description << std::setw(Setting::kIndent) << "" << std::setw(align) << "--help" << "shows this message" << std::endl;
            help_description << std::setw(Setting::kIndent) << "" << std::setw(align) << "--gui" << "graphical user interface [true/false]" << std::endl;
            help_description << std::setw(Setting::kIndent) << "" << std::setw(align) << "--rootPath" << "path to puppet-cube-v2/" << std::endl;
            help_description << std::setw(Setting::kIndent) << "" << std::setw(align) << "--errorLevel" << "amount of output [criticalError/error/info/all/extra/memory]" << std::endl;
            help_description << std::endl;
            help_description << std::setw(Setting::kIndent) << "" << "Example: ./build/bin/PuppetCubeV2 --gui=false --rootPath=./ --errorLevel=extra --threads=1"<< std::endl;
            error_handler.Handle(ErrorHandler::Level::kInfo, "setting.cpp", help_description.str());
        }

        else if (argument.find("--gui=") == 0) {
            argument = argument.erase(0, std::string("--gui=").size());
            if (argument == "true") {
                gui = true;
            }
            else if (argument == "false") {
                gui = false;
            }
            else {
                error_handler.Handle(ErrorHandler::Level::kWarning, "settings.cpp", "gui argument not found. Should be true/false");
            }
        }

        else if (argument.find("--rootPath=") == 0) {
            rootPath = argument.erase(0, std::string("--rootPath=").size());
        }

        else if (argument.find("--errorLevel=") == 0) {
            std::string error_level = argument.erase(0, std::string("--errorLevel=").size());
            if (error_level == "criticalError") {
                error_handler.SetErrorLevel(ErrorHandler::Level::kCriticalError);
            }
            else if (error_level == "error") {
                error_handler.SetErrorLevel(ErrorHandler::Level::kError);
            }
            else if (error_level == "warning") {
                error_handler.SetErrorLevel(ErrorHandler::Level::kWarning);
            }
            else if (error_level == "info") {
                error_handler.SetErrorLevel(ErrorHandler::Level::kInfo);
            }
            else if (error_level == "all") {
                error_handler.SetErrorLevel(ErrorHandler::Level::kAll);
            }
            else if (error_level == "extra") {
                error_handler.SetErrorLevel(ErrorHandler::Level::kExtra);
            }
            else if (error_level == "memory") {
                error_handler.SetErrorLevel(ErrorHandler::Level::kMemory);
            }
            else {
                error_handler.Handle(ErrorHandler::Level::kWarning, "settings.cpp", "error level type " + error_level + " not found");
            }
        }

        else {
            error_handler.Handle(ErrorHandler::Level::kInfo, "settings.cpp", "could not find a setting for: " + argument);
        }
    }
}
