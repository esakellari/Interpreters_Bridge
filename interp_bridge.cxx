#include "ASTImportSource.h"
#include <ctime>

int main(int argc, char** argv) {

  cling::Interpreter interp_first(argc, argv, LLVMRESDIR);
  strcat(*argv, "-fsyntax-only");
  cling::Interpreter interp_second(argc, argv, LLVMRESDIR);

  clang::TranslationUnitDecl *global_DC_interp1  =
    interp_first.getCI()->getASTContext().getTranslationUnitDecl();

  clang::TranslationUnitDecl *global_DC_interp2  =
    interp_second.getCI()->getASTContext().getTranslationUnitDecl();

  std::map<clang::TranslationUnitDecl*, clang::TranslationUnitDecl*> DC_map;
  /*DC_map.insert(std::pair<clang::TranslationUnitDecl*,
    clang::TranslationUnitDecl*>
                  (global_DC_interp1,global_DC_interp2));*/
  DC_map[global_DC_interp1] = global_DC_interp2;

  /****************************************************************************************
  const char *Str = "mynamespace";
  llvm::StringRef name(Str);
  clang::IdentifierTable &identifierTable = interp_second.getCI()->getASTContext().Idents;
  clang::IdentifierInfo &IIOrig = identifierTable.get(name);
  clang::DeclarationName declarationName(&IIOrig);
  llvm::StringRef identName = IIOrig.getName();
 // cout << identName.str() << endl;
  ***************************************************************************************/

  cling::Interpreter *first_interp_p = &interp_first;
  cling::Interpreter *second_interp_p = &interp_second;

  //1. Create an external source from the information from the
  //first interpreter
  ASTImportSource *myExternalSource = new ASTImportSource(first_interp_p, second_interp_p);

  //2. Set as an External AST source the AST source I created from the first interpreter
  //for the second interpreter.

  clang::Sema& SemaRef = interp_second.getSema();
  // clang::ASTReader* Reader = interp_second.getCI()->getModuleManager().get();
  //clang::ExternalSemaSource* externalSemaSrc = SemaRef.getExternalSource();
  //if (!externalSemaSrc || externalSemaSrc == Reader) {
    myExternalSource->InitializeSema(SemaRef);
    interp_second.getSema().addExternalSource(myExternalSource);

    llvm::IntrusiveRefCntPtr <clang::ExternalASTSource>
      astContextExternalSource(SemaRef.getExternalSource());
    clang::ASTContext &Ctx = SemaRef.getASTContext();
    Ctx.ExternalSource.resetWithoutRelease();
    Ctx.setExternalSource(astContextExternalSource);
  //}

  //3. And finally inform the second interpreter that we have to search
  //in external sources for the semantic information.
  global_DC_interp2->setHasExternalVisibleStorage();

  //interp_second.declare(R"code(#include "header_interpTwo.h" )code");
  interp_first.declare(R"code(#include "header_interpOne.h")code");
  interp_first.echo("t+p");

  llvm::StringRef fooFun("foo");
  void* address = interp_first.getAddressOfGlobal(fooFun);
  interp_second.addSymbolPublic("foo", address);

  interp_first.execute("foo()");
  interp_second.execute("foo()");
  interp_first.execute("foo()");
  //cling::Interpreter::CompilationResult resu1 = interp_second.echo(" foo() ");
  //cling::Interpreter::CompilationResult resu = interp_first.echo("foo()");

  /*interp_second.declare(R"code(
   int main(int, char*[]) {
    //t = 0;
    //p = 0;
    //B::t = 100;
    std::cout << "in main" << std::endl;
    return 0;
   }
   )code");
*/
  //interp_first.echo("&t");
  //interp_second.echo("&t");

  delete myExternalSource;
  return 0;
}