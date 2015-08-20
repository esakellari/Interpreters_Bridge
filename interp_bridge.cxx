#include "ASTImportSource.h"
#include <ctime>

namespace clang {

  ///\brief Cleanup Parser state after a failed lookup.
  ///
  /// After a failed lookup we need to discard the remaining unparsed input,
  /// restore the original state of the incremental parsing flag, clear any
  /// pending diagnostics, restore the suppress diagnostics flag, and restore
  /// the spell checking language options.
  ///

  class a{



  };
  class ParserStateRAII {
    private:
    Parser* P;
    Preprocessor& PP;
    bool ResetIncrementalProcessing;
    bool OldSuppressAllDiagnostics;
    bool OldSpellChecking;
    DestroyTemplateIdAnnotationsRAIIObj CleanupTemplateIds;
    SourceLocation OldPrevTokLocation;
    unsigned short OldParenCount, OldBracketCount, OldBraceCount;
    unsigned OldTemplateParameterDepth;


    public:
    ParserStateRAII(Parser& p)
      : P(&p), PP(p.getPreprocessor()),
        ResetIncrementalProcessing(p.getPreprocessor()
                                     .isIncrementalProcessingEnabled()),
        OldSuppressAllDiagnostics(p.getPreprocessor().getDiagnostics()
                                    .getSuppressAllDiagnostics()),
        OldSpellChecking(p.getPreprocessor().getLangOpts().SpellChecking),
        CleanupTemplateIds(p), OldPrevTokLocation(p.PrevTokLocation),
        OldParenCount(p.ParenCount), OldBracketCount(p.BracketCount),
        OldBraceCount(p.BraceCount),
        OldTemplateParameterDepth(p.TemplateParameterDepth)
    {
    }

    ~ParserStateRAII()
    {
      //
      // Advance the parser to the end of the file, and pop the include stack.
      //
      // Note: Consuming the EOF token will pop the include stack.
      //
      P->SkipUntil(tok::eof);
      PP.enableIncrementalProcessing(ResetIncrementalProcessing);
      // Doesn't reset the diagnostic mappings
      P->getActions().getDiagnostics().Reset(/*soft=*/true);
      PP.getDiagnostics().setSuppressAllDiagnostics(OldSuppressAllDiagnostics);
      const_cast<LangOptions&>(PP.getLangOpts()).SpellChecking =
        OldSpellChecking;

      P->PrevTokLocation = OldPrevTokLocation;
      P->ParenCount = OldParenCount;
      P->BracketCount = OldBracketCount;
      P->BraceCount = OldBraceCount;
      P->TemplateParameterDepth = OldTemplateParameterDepth;
    }
  };
}

int main(int argc, char** argv) {

  cling::Interpreter interp_first(argc, argv, LLVMRESDIR);
  strcat(*argv, "-fsyntax-only");
  cling::Interpreter interp_second(argc, argv, LLVMRESDIR);

  interp_first.declare(R"code(#include "header_interpOne.h")code");

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
    // If the ExternalSemaSource is the PCH reader we still need to insert
    // our listener.
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

  interp_second.declare(R"code(#include "header_interpTwo.h" )code");

  return 0;
}