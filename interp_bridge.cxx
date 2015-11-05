#include "ASTImportSource.h"

int main(int argc, char** argv) {

  cling::Interpreter I1(argc, argv, LLVMRESDIR);
  cling::IncrementalExecutor* I1IncrExecutor = I1.getIncrementalExecutor();
  {
    cling::Interpreter I2(argc, argv, LLVMRESDIR);
    I2.setExternalIncrementalExecutor(I1IncrExecutor);

    clang::TranslationUnitDecl *translationUnitDeclI1 =
      I1.getCI()->getASTContext().getTranslationUnitDecl();

    clang::TranslationUnitDecl *translationUnitDeclI2 =
      I2.getCI()->getASTContext().getTranslationUnitDecl();

    std::map < clang::TranslationUnitDecl * , clang::TranslationUnitDecl * > DC_map;

    DC_map[translationUnitDeclI1] = translationUnitDeclI2;

    cling::Interpreter *I1_p = &I1;
    cling::Interpreter *I2_p = &I2;

    //1. Create an external source from the information from the
    //first interpreter
    ASTImportSource *myExternalSource = new ASTImportSource(I1_p, I2_p);

    //2. Set as an External AST source the AST source I created from the first interpreter
    //for the second interpreter.

    clang::Sema &SemaRef = I2.getSema();
    // clang::ASTReader* Reader = I2.getCI()->getModuleManager().get();
    //clang::ExternalSemaSource* externalSemaSrc = SemaRef.getExternalSource();
    //if (!externalSemaSrc || externalSemaSrc == Reader) {
    myExternalSource->InitializeSema(SemaRef);
    I2.getSema().addExternalSource(myExternalSource);

    llvm::IntrusiveRefCntPtr <clang::ExternalASTSource>
      astContextExternalSource(SemaRef.getExternalSource());
    clang::ASTContext &Ctx = SemaRef.getASTContext();
    Ctx.ExternalSource.resetWithoutRelease();
    Ctx.setExternalSource(astContextExternalSource);
    //}

    //3. And finally inform the second interpreter that we have to search
    //in external sources for the semantic information.

    I1.declare("int x=9;");

    I2.declare("#include <iostream>");
    I1.declare("#include \"I1.h\"");
    I2.declare(R"code(#include "I2.h" )code");
    translationUnitDeclI2->setHasExternalVisibleStorage();

    clang::CompilerInstance *ci = I1.getCI();
    clang::DiagnosticsEngine &diags = ci->getSema().getDiagnostics();
    diags.Reset();

    /*1*/I1.execute("foo_namespace::foo()");
    /*2*/I2.execute("foo_namespace::foo()");
    /*3*/I1.execute("foo_namespace::foo()");

    I1.execute("foofoo()");
    I2.execute("foofoo()");

    I2.execute("bar()");
    I1.execute("bar()"); /* ERROR: unresolved symbol // defined in I2 */
    //I2.~Interpreter();
  } // I2 destructed here
  I1.execute("foo_namespace::foo()"); /* something I2 was calling, but is only defined in I1 */
  I1.execute("bar()"); /* ERROR: unresolved symbol // defined in I2 */
  I1.execute("foofoo()"); /* a function defined both in I1 and I2*/

  return 0;
}