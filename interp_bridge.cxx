#include "ASTImportSource.h"

namespace clang {

  ///\brief Cleanup Parser state after a failed lookup.
  ///
  /// After a failed lookup we need to discard the remaining unparsed input,
  /// restore the original state of the incremental parsing flag, clear any
  /// pending diagnostics, restore the suppress diagnostics flag, and restore
  /// the spell checking language options.
  ///
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
  cling::Interpreter interp_second(argc, argv, LLVMRESDIR);

  interp_first.declare(R"code(#include "header_interpOne.h")code");

  clang::TranslationUnitDecl *global_DC_interp1  =
    interp_first.getCI()->getASTContext().getTranslationUnitDecl();

  clang::TranslationUnitDecl *global_DC_interp2  =
    interp_second.getCI()->getASTContext().getTranslationUnitDecl();

  std::map<clang::TranslationUnitDecl*, clang::TranslationUnitDecl*> DC_map;
  DC_map.insert(std::pair<clang::TranslationUnitDecl*,
    clang::TranslationUnitDecl*>
                  (global_DC_interp1,global_DC_interp2));

  //clang::Decl *decl = llvm::dyn_cast<clang::Decl>(global_DC_interp1);
 // decl->dump();

 // ASTImportSource *astImportSource = new ASTImportSource();
  //clang::DeclarationName *declarationName3 = new clang::DeclarationName();//empty for now
  //bool found  = astImportSource->FindExternalVisibleDeclsByName(global_DC_interp1, *declarationName3);

 /* const char *Str = "non_existent_name";
  llvm::StringRef name(Str);
  clang::IdentifierTable &identifierTable1 = interp_first.getCI()->getASTContext().Idents;
  clang::IdentifierInfo &IIOrig1 = identifierTable1.get(name);
    clang::DeclarationName declarationName1(&IIOrig1);
    llvm::StringRef identName1 = IIOrig1.getName();
    cout<<  declarationName1.getAsString() << endl;
    clang::DeclContext::lookup_result res1 = global_DC_interp1->lookup(declarationName1);*/

  const char *Str = "mynamespace";
  llvm::StringRef name2(Str);

  //set my external source to the
  ASTImportSource *myExternalSource = new ASTImportSource();
  interp_second.getSema().addExternalSource(myExternalSource);
  global_DC_interp2->setHasExternalVisibleStorage();

  llvm::IntrusiveRefCntPtr<clang::ExternalASTSource> astContextExternalSource(myExternalSource);//interp_second.getSema().getExternalSource());
  interp_second.getCI()->getASTContext().setExternalSource(astContextExternalSource);

  interp_second.declare(R"code(#include "header_interpTwo.h" )code");
  // llvm::StringRef identName1 = IIOrig1.getName();
  // cout<<  declarationName2.getAsString() << endl;

  bool hasexternal = global_DC_interp2->hasExternalVisibleStorage();
  clang::IdentifierTable &identifierTable2 = interp_second.getCI()->getASTContext().Idents;
  //unsigned int size = identifierTable1.size();
  clang::IdentifierInfo &IIOrig2 = identifierTable2.get(name2);
  //llvm::StringRef identName2 = IIOrig2.getName();
  clang::DeclarationName declarationName2(&IIOrig2);
  //std::string asstring2 = declarationName2.getAsString();
  static clang::DeclContext *translationUnitDeclContext =  	global_DC_interp2->castToDeclContext (global_DC_interp2);
  //clang::DeclContext::lookup_result res2 = translationUnitDeclContext->lookup(declarationName2);
  //clang::DeclContext::lookup_result res2 =  global_DC_interp2->lookup(declarationName2);
/*
  clang::DeclContext *declContext = global_DC_interp2->getEnclosingNamespaceContext();
  bool isNamespace  = declContext->isNamespace();
  clang::DeclContext *declContext_enclosing_t = declContext->getEnclosingNamespaceContext();

  clang::Decl::Kind  kind = declContext_enclosing_t->getDeclKind();
  clang::DeclarationName declarationName;
*/

  return 0;
}