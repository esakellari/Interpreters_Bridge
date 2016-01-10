#include "ASTImportSource.h"

using namespace cling;
using namespace cling::utils;
using namespace clang;

int main(int argc, char** argv) {

  /*-----------------------I1 Creation----------------------------------*/
  Interpreter I1(argc, argv, LLVMRESDIR);
  /*--------------------------------------------------------------------*/

  {
    /*-----------------------I2 Creation----------------------------*/
    Interpreter I2(I1, argc, argv, LLVMRESDIR);

    //TranslationUnitDecl *translationUnitDeclI2 =
      //I2.getCI()->getASTContext().getTranslationUnitDecl();

    Interpreter *I1_p = &I1;
    Interpreter *I2_p = &I2;

    //1. Create an external source from the information from the
    //first interpreter
    ASTImportSource *myExternalSource = new ASTImportSource(I1_p, I2_p);

   // ASTContext& tuI2ASTContext = I2.getCI()->getASTContext();

    llvm::IntrusiveRefCntPtr <ExternalASTSource>
      astContextExternalSource(myExternalSource);

    I2.getCI()->getASTContext().ExternalSource.resetWithoutRelease();
    I2.getCI()->getASTContext().setExternalSource(astContextExternalSource);
    /* Inform the Translation Unit Decl of I2 that it has to
     * search somewhere else to find the declarations. */
    I2.getCI()->getASTContext().getTranslationUnitDecl()->setHasExternalVisibleStorage();

    /*--------------------------------------------------------------*/

    /*---------------------------TESTS-------------------------*/
    I1.declare("#include <iostream>");
    I1.declare("#include <vector>");
    I1.declare("#include <map>");
/*
    I1.declare("template<typename X, typename Y> void func(X x, Y y) {};");
    I1.declare("template<typename X> void func(X x, int y);");
    I1.declare("int x;");
    I2.execute("x=2;");
    I2.execute("func(2,2);");

    I1.declare("template <class T>\n"
                 "class mypair {\n"
                 "    T values [2];\n"
                 "  public:\n"
                 "    mypair (T first, T second)\n"
                 "    {\n"
                 "      values[0]=first; values[1]=second;\n"
                 "    }\n"
                 "};");

    I2.declare("mypair<int> *myobject;");

    I1.declare("std::vector<int> myvec;");

    I2.declare("int x;");
    I1.declare("extern \"C\"{"
                 "void foo(){"
                 "std::cout << \"Interp1::foo \" << std::endl;"
                 "static int i = 0;"
                 "std::cout << ++i << std::endl;"
                 "}"
                 "}");

    I1.execute("foo();");
    I2.execute("foo();");
    I1.execute("foo();");*/

   // I1.declare("template<typename X, typename Y> void func(X x, Y y) "
   //              "{ std::cout << \"Template function func\" << std::endl; };");
   // I1.execute("func(2,3);");
   // I2.execute("func(2,2);");
    I1.declare("void hello(){ std::cout << \"hello(void)\" << std::endl; }");


    I1.execute("hello()");
    I2.execute("hello()");
    I1.declare("void hello(int i){ std::cout << \"hello(int)\" << std::endl; }");
    I2.execute("hello()");
    I1.execute("hello(8999)");
    //I1.execute("hello(899009)");

    I2.execute("hello(8);");

    I1.declare("void I1func(int i){ std::cout << \"I1::func, i = \" << i << std::endl;}");
    I1.execute("I1func(1);");
    I2.execute("I1func(2);");

    I1.echo("4433");

    I2.echo("3");

    //I2.declare("std::map<int,int> m_map;");
    //I2.declare("std::vector<int> myvec2;");
  } // I2 destructed here
  return 0;
}