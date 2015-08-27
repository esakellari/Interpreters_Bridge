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
  return false; }

bool ASTImportSource::FindExternalVisibleDeclsByName(
  const clang::DeclContext *DC, clang::DeclarationName Name){
  assert(DC->hasExternalVisibleStorage() &&
           "DeclContext has no visible decls in storage");

  const clang::TranslationUnitDecl *tu_original  =
    m_first_Interp->getCI()->getASTContext().getTranslationUnitDecl();

  const clang::TranslationUnitDecl *tu_temporary  =
    m_second_Interp->getCI()->getASTContext().getTranslationUnitDecl();

  static clang::DeclContext * tunitDeclContext_Orig =
  clang::TranslationUnitDecl::castToDeclContext(tu_original);

  static clang::DeclContext * tunitDeclContext_Temp =
  clang::TranslationUnitDecl::castToDeclContext(tu_temporary);

  /*clang will call FindExternalVisibleDeclsByName with an
  IdentifierInfo valid for the second interpreter. Get the
  IdentifierInfo's StringRef representation; you ask the first
  interpreter's ASTContext.Ident*/
  llvm::StringRef name(Name.getAsString());
  //construct again the Declaration name  from the
  //Identifier Info we got from the first interpreter.

  clang::IdentifierTable &identsTemp =
    m_second_Interp->getCI()->getASTContext().Idents;
  clang::IdentifierInfo &IITemp = identsTemp.get(name);
  clang::DeclarationName declNameTemp(&IITemp);

  clang::IdentifierTable &identsOrig =
    m_first_Interp->getCI()->getASTContext().Idents;
  //Identifier info about the context we are looking for
  //It will also be used for the map of Decl Contexts.
  clang::IdentifierInfo &IIOrig = identsOrig.get(name);
  clang::DeclarationName declNameOrig(&IIOrig);

  cout << "About to search in the map of the second interpreter "
            "for the Decl Context : " << declNameOrig.getAsString()
  << endl;
  if (m_declName_map.find(declNameTemp) != m_declName_map.end()){
    cout << "Found " << declNameTemp.getAsString() << " in the map." << endl;
    return true;
  }

  cout << "About to search in first interpreter for: "
       << declNameOrig.getAsString() << endl;
  clang::DeclContext::lookup_result lookup_result =
    tunitDeclContext_Orig->lookup(declNameOrig);

  if(lookup_result.data()){
    clang::Decl *declResult = *lookup_result.data();
    //clang::NamedDecl *namedDeclResult = *lookup_result.data();
    cout<< "Did lookup and found in Interpreter 1: " <<
    (*lookup_result.data())->getNameAsString() << endl;

    clang::ASTContext &from_ASTContext = tu_original->getASTContext();
    clang::ASTContext &to_ASTContext = tu_temporary->getASTContext();

    const clang::FileSystemOptions systemOptions;
    clang::FileManager fm(systemOptions, nullptr);
    clang::ASTImporter importer(to_ASTContext, fm, from_ASTContext, fm,false);

    clang::Decl* importedDecl = importer.Import(declResult);

    // importer.Import((*lookup_result.data())->getDeclName());
    /***************************************************/
    //std::vector<clang::NamedDecl*> declVector;
   // declVector.push_back((clang::NamedDecl*)importedDecl);
    // declVector.push_back(dynamic_cast<clang::NamedDecl*>(importedDecl));
    //llvm::ArrayRef<clang::NamedDecl*> FoundDecls(declVector);
    // clang::DeclContext::lookup_result setExternalResult =
    //SetExternalVisibleDeclsForName(DC, Name, FoundDecls);
    /***************************************************/

    //put the declaration context I found from the first interpreter
    //in my map to have it for the future.
    //Map<NamedDecl* /*second*/,NamedDecl* /*first*/>
    m_Declarations_map[importedDecl]= declResult;
    m_declName_map[declNameTemp] = declNameOrig;

    return true;
  }
  cout<< "Did not find " << declNameOrig.getAsString()
  << " in first interpreter"
  << endl;
  return false;
}