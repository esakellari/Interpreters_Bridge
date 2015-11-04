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

  const clang::TranslationUnitDecl *translationUnitI1  =
    m_first_Interp->getCI()->getASTContext().getTranslationUnitDecl();

  const clang::TranslationUnitDecl *translationUnitI2 =
    m_second_Interp->getCI()->getASTContext().getTranslationUnitDecl();

  static clang::DeclContext * declContextI1 =
  clang::TranslationUnitDecl::castToDeclContext(translationUnitI1);

  llvm::StringRef name(Name.getAsString());

  /* clang will call FindExternalVisibleDeclsByName with an
     IdentifierInfo valid for the second interpreter. Get the
     IdentifierInfo's StringRef representation; you ask the first
     interpreter's ASTContext.Ident. */

  /* Construct again the Declaration name  from the
     Identifier Info we got from the first interpreter. */
  clang::IdentifierTable &identsI2 =
    m_second_Interp->getCI()->getASTContext().Idents;
  clang::IdentifierInfo &IdentifierInfoI2 = identsI2.get(name);
  clang::DeclarationName declNameI2(&IdentifierInfoI2);

  /* Identifier info about the context we are looking for.
     It will also be used for the map of Decl Contexts we keep in I2.*/
  clang::IdentifierTable &identsI1 =
    m_first_Interp->getCI()->getASTContext().Idents;
  clang::IdentifierInfo &IdentifierInfoI1 = identsI1.get(name);
  clang::DeclarationName declNameI1(&IdentifierInfoI1);

  std::cout << "About to search in the map of the second interpreter "
               "for the Name : "
            << declNameI1.getAsString()
            << std::endl;

  if (m_declName_map.find(declNameI2) != m_declName_map.end()){
    std::cout << "Found "
              << declNameI2.getAsString()
              << " in the map."
              << std::endl;
    return true;
  }

  std::cout << "About to search in first interpreter for:"
            << declNameI1.getAsString()
            << std::endl;

  clang::DeclContext::lookup_result lookup_result =
    declContextI1->lookup(declNameI1);

  if(lookup_result.data()){
    clang::Decl *declFrom = llvm::cast<clang::Decl>(*lookup_result.data());

    std::cout << "Did lookup and found in Interpreter 1: "
              << (*lookup_result.data())->getNameAsString()
              << std::endl;

    //llvm::SmallVector<clang::NamedDecl*, 4> FoundDecls;
    clang::ASTContext &from_ASTContext = translationUnitI1->getASTContext();
    clang::ASTContext &to_ASTContext = translationUnitI2->getASTContext();

    const clang::FileSystemOptions systemOptions;
    clang::FileManager fm(systemOptions, nullptr);
    clang::ASTImporter importer(to_ASTContext, fm, from_ASTContext, fm,  false);

    clang::Decl* importedDecl = importer.Import(declFrom);
    if(!importedDecl)
      std::cerr << "Error: Could not import "
                << declNameI1.getAsString()
                << std::endl;
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

  std::cout << "Did not find "
            << declNameI1.getAsString()
            << " in first interpreter"
            << std::endl;

  return false;
}