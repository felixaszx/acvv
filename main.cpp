#include "app.hpp"

int main(int argc, char** argv)
{
    App app;
    try
    {
        app.run();
    }
    catch (std::exception e)
    {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}