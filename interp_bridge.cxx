#include "ASTImportSource.h"

int main(int argc, char** argv) {

  cling::Interpreter interp_first(argc, argv, LLVMRESDIR);
  cling::IncrementalExecutor* I1IncrExecutor = interp_first.getIncrementalExecutor();

  cling::Interpreter interp_second(argc, argv, LLVMRESDIR);
  interp_second.setExternalIncrementalExecutor(I1IncrExecutor);

  clang::TranslationUnitDecl *global_DC_interp1  =
    interp_first.getCI()->getASTContext().getTranslationUnitDecl();

  clang::TranslationUnitDecl *global_DC_interp2  =
    interp_second.getCI()->getASTContext().getTranslationUnitDecl();

  std::map<clang::TranslationUnitDecl*, clang::TranslationUnitDecl*> DC_map;

  DC_map[global_DC_interp1] = global_DC_interp2;

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


  interp_second.declare(R"code(#include "header_interpTwo.h" )code");
  global_DC_interp2->setHasExternalVisibleStorage();
  interp_first.declare(R"code(#include "header_interpOne.h")code");

  clang::CompilerInstance* ci = interp_first.getCI();
  clang::DiagnosticsEngine &diags = ci->getSema().getDiagnostics();
  diags.Reset();

  /*1*/interp_first.execute("foo_namespace::foo()");
  /*2*/interp_second.execute("foo_namespace::foo()");
  /*3*/interp_first.execute("foo_namespace::foo()");

  interp_first.execute("foofoo()");
  interp_second.execute("foofoo()");

  //dete myExternalSource;
  return 0;
}