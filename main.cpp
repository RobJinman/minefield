/*
 * Author: Rob Jinman <admin@robjinman.com>
 * Date: 2013
 */

#include <iostream>
#include <dodge/Exception.hpp>
#include "Application.hpp"


//===========================================
// programExit
//===========================================
void programExit() {
   exit(0);
}

//===========================================
// main
//===========================================
int main(int argc, char** argv) {
   Application app;

   try {
      app.onExit(Functor<void, TYPELIST_0()>(programExit));
      app.launch(argc, argv);
   }
   catch (Dodge::Exception& e) {
      std::cerr << "An error occured; " << e.what() << "\n";
   }
   catch (...) {
      std::cerr << "An unknown error occured\n";
   }

   return EXIT_FAILURE;
}
