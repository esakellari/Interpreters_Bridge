#include "ASTImportSource.h"
#include <iostream>
using namespace clang;

.L ASTImportSource.cpp
.L /home/esakella/cling/src/tools/clang/lib/AST/ASTImporter.cpp
.L /home/esakella/cling/src/tools/clang/lib/AST/ExternalASTSource.cpp

const char * const argV = "cling";

cling::Interpreter *ChildInterp;
ChildInterp = new cling::Interpreter(*gCling, 1, &argV);

cling::utils::ASTImportSource *myExternalSource = new cling::utils::ASTImportSource(gCling, ChildInterp);

llvm::IntrusiveRefCntPtr <ExternalASTSource> astContextExternalSource(myExternalSource);
ChildInterp->getCI()->getASTContext().ExternalSource.resetWithoutRelease();
ChildInterp->getCI()->getASTContext().setExternalSource(astContextExternalSource);
ChildInterp->getCI()->getASTContext().getTranslationUnitDecl()->setHasExternalVisibleStorage();

//declare something in the parent interpreter
gCling->declare("void hello(){ std::cout << \"hello(void)\" << std::endl; }");

//then execute it from the child interpreter
ChildInterp->execute("hello()");

//check if function overloading works
gCling->declare("void hello(int i){ std::cout << \"hello(int)\" << std::endl; }");

ChildInterp->execute("hello(1)");


