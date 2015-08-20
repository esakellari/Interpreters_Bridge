//
// Created by esakella on 7/8/15.
//

#include "ASTImportSource.h"

ASTImportSource::~ASTImportSource() { }

void ASTImportSource::InitializeSema(clang::Sema& S) {
  m_Sema = &S;
}

bool ASTImportSource::LookupUnqualified(clang::LookupResult &R,
                                        clang::Scope *S) {
  //cout << "ASTImportSource::LookupUnqualified(LookupResult &R, Scope *S)" <<endl;
  //cout << aa << endl;
  return false; }

bool ASTImportSource::FindExternalVisibleDeclsByName(
  const clang::DeclContext *DC, clang::DeclarationName Name){
  assert(DC->hasExternalVisibleStorage() &&
           "DeclContext has no visible decls in storage");

  //cout << "ASTImportSource::FindExternalVisibleDeclsByName" <<endl;
  //cout << Name.getAsString() << endl;
  const clang::TranslationUnitDecl *global_DC_interp1  =
    m_first_Interp->getCI()->getASTContext().getTranslationUnitDecl();

  const clang::TranslationUnitDecl *global_DC_interp2  =
    m_second_Interp->getCI()->getASTContext().getTranslationUnitDecl();

  static clang::DeclContext * translation_unit_decl_context_one =
  clang::TranslationUnitDecl::castToDeclContext(global_DC_interp1);

  static clang::DeclContext * translation_unit_decl_context_two =
    clang::TranslationUnitDecl::castToDeclContext(global_DC_interp2);

  /*clang will call FindExternalVisibleDeclsByName with an
  IdentifierInfo valid for the second interpreter. Get the
  IdentifierInfo's StringRef representation; you ask the first
  interpreter's ASTContext.Ident*/
  llvm::StringRef name(Name.getAsString());
  //construct again the Declaration name  from the
  //Identifier Info we got from the first interpreter.

  clang::IdentifierTable &identifierTable =
    m_first_Interp->getCI()->getASTContext().Idents;
  //Identifier info about the context we are looking for
  //It will also be used for the map of Decl Contexts.
  clang::IdentifierInfo &IIOrig = identifierTable.get(name);
  clang::DeclarationName declarationName(&IIOrig);
  //llvm::StringRef identName = IIOrig.getName();

  cout << "About to search in the map of the second interpreter "
            "for the Decl Context : " << declarationName.getAsString()
  << endl;

  cout << "About to search in first interpreter for: "
       << declarationName.getAsString() << endl;
  clang::DeclContext::lookup_result lookup_result =
    translation_unit_decl_context_one->lookup(declarationName);

  if(lookup_result.data()){
   // clang::Decl *DeclResult = *lookup_result.data();
    cout<< "Did lookup and found in Interpreter 1: " <<
    (*lookup_result.data())->getNameAsString() << endl;

    std::vector<clang::NamedDecl* > namedDeclVector;
   // namedDeclVector.push_back((clang::NamedDecl*)namedDeclResult);
    namedDeclVector.push_back(*lookup_result.data());
    llvm::ArrayRef<clang::NamedDecl*> FoundDecls(namedDeclVector);

    /***************************************************/
    clang::DeclContext::lookup_result setExternalResult =
      SetExternalVisibleDeclsForName(DC, Name, FoundDecls);
    /***************************************************/

    //put the declaration context I found from the first interpreter
    //in my map to have it for the future.
    //Map<NamedDecl* /*second*/,NamedDecl* /*first*/>
    //clang::DeclContext *DeclContextSecond = (*setExternalResult.data())->getDeclContext();
    //clang::DeclContext *DeclContextFirst =  (*lookup_result.data())->getDeclContext();
    m_Decl_map[*setExternalResult.data()]= *lookup_result.data();

    clang::DeclContext::lookup_result setExternalResult1 =
      SetExternalVisibleDeclsForName(DC, Name, FoundDecls);
    return !FoundDecls.empty();
  }

  return false;
}

