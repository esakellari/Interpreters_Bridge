//
// Created by esakella on 7/8/15.
//

#include "ASTImportSource.h"

ASTImportSource::~ASTImportSource() { }

void ASTImportSource::InitializeSema(clang::Sema& S) {
  m_Sema = &S;
}

void ASTImportSource::ForgetSema() {
  m_Sema = nullptr;
}

bool ASTImportSource::GetCompleteDecl(clang::ASTContext *ast, clang::Decl *decl) {

  clang::ExternalASTSource *ast_source = ast->getExternalSource();

  if(!ast_source)
    return false;

  if (clang::TagDecl *tag_decl = llvm::dyn_cast<clang::TagDecl>(decl)) {
    if (tag_decl->isCompleteDefinition())
      return true;

    if (!tag_decl->hasExternalLexicalStorage())
      return false;

    ast_source->CompleteType(tag_decl);

    return !tag_decl->getTypeForDecl()->isIncompleteType();
  }
  return false;
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

  //static clang::DeclContext * tunitDeclContext_Temp =
  //clang::TranslationUnitDecl::castToDeclContext(tu_temporary);

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

  std::cout << "About to search in the map of the second interpreter "
            "for the Decl Context : " << declNameOrig.getAsString()
  << std::endl;

  if (m_declName_map.find(declNameTemp) != m_declName_map.end()){
    std::cout << "Found " << declNameTemp.getAsString() << " in the map." << std::endl;
    return true;
  }

  //If you are about to search for the namespace "cling", don't
  //if(Name.getAsString() == "cling")
    //return true;

  std::cout << "About to search in first interpreter for: "
       << declNameOrig.getAsString() << std::endl;
  clang::DeclContext::lookup_result lookup_result =
    tunitDeclContext_Orig->lookup(declNameOrig);

  if(lookup_result.data()){
    //clang::Decl *declFrom = *lookup_result.data();
    clang::Decl *declFrom = llvm::cast<clang::Decl>(*lookup_result.data());
    std::cout<< "Did lookup and found in Interpreter 1: " <<
    (*lookup_result.data())->getNameAsString() << std::endl;

    // TO DELETE
    /*if(Name.getAsString() == "foo"){
      llvm::StringRef fooFun("foo");
      void* address = m_first_Interp->getAddressOfGlobal(fooFun);
      m_second_Interp->addSymbolPublic("foo", address);
    }*/
    //llvm::SmallVector<clang::NamedDecl*, 4> FoundDecls;

    clang::ASTContext &from_ASTContext = tu_original->getASTContext();
    clang::ASTContext &to_ASTContext = tu_temporary->getASTContext();

    const clang::FileSystemOptions systemOptions;
    clang::FileManager fm(systemOptions, nullptr);
    clang::ASTImporter importer(to_ASTContext, fm, from_ASTContext, fm,  false);

    clang::Decl* importedDecl = importer.Import(declFrom);
    if(!importedDecl)
      std::cerr << "Error: Could not import " << declNameOrig.getAsString() << std::endl;
    else {
      clang::ASTContext *astContextP = &from_ASTContext;
      GetCompleteDecl(astContextP, declFrom);
      //clang::Decl* imported= importer.Imported(declFrom,importedDecl);
      importer.ImportDefinition(declFrom);

      std::vector<clang::NamedDecl*> declVector;
      //clang::NamedDecl *namedDeclResult = *lookup_result.data();
      declVector.push_back((clang::NamedDecl*)importedDecl);
      llvm::ArrayRef<clang::NamedDecl*> FoundDecls(declVector);
      SetExternalVisibleDeclsForName(DC, Name, FoundDecls);
    }

    /***************************************************/
    // importer.Import((*lookup_result.data())->getDeclName());
    //std::vector<clang::NamedDecl*> declVector;
   //
    // declVector.push_back(dynamic_cast<clang::NamedDecl*>(importedDecl));
    //
    // clang::DeclContext::lookup_result setExternalResult =
    /***************************************************/

    //Put the declaration context I found from the first interpreter
    //in the map to have it for the future.
    //Map<NamedDecl* /*second*/,NamedDecl* /*first*/>
    //m_Declarations_map[importedDecl]= declFrom;
    // m_declName_map[declNameTemp] = declNameOrig;

    return true;
  }

  std::cout<< "Did not find " << declNameOrig.getAsString()
  << " in first interpreter"
  << std::endl;
  return false;
}