//
// Created by esakella on 7/8/15.
//
#include "ASTImportSource.h"

ASTImportSource::ASTImportSource(cling::Interpreter* interpreter_first,
                                 cling::Interpreter* interpreter_second):
  m_first_Interp(interpreter_first), m_second_Interp(interpreter_second) {

  m_translationUnitI1 = m_first_Interp->getCI()->getASTContext().getTranslationUnitDecl();
  m_translationUnitI2 = m_second_Interp->getCI()->getASTContext().getTranslationUnitDecl();

  m_TUDeclContextI1 = clang::TranslationUnitDecl::castToDeclContext(m_translationUnitI1);
}

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

bool ASTImportSource::Import(clang::DeclContext::lookup_result lookup_result,
                             clang::ASTContext& from_ASTContext,
                             clang::ASTContext& to_ASTContext,
                             const clang::DeclContext *DC,
                             clang::DeclarationName Name){

  if(lookup_result.empty()) {
    std::cout << "Did not find "
    << Name.getAsString()
    << " in first interpreter"
    << std::endl;
    return false;
  }

  //Prepare to import the Decl(Context)  we found in the
  //second interpreter
  const clang::FileSystemOptions systemOptions;
  clang::FileManager fm(systemOptions, nullptr);

  /*----------------------------ASTImporter---------------------------------*/
  clang::ASTImporter importer(to_ASTContext, fm, from_ASTContext, fm,
                                  /*MinimalImport : ON*/ true);
  /*-----------------------------------------------------------------------*/

  //If this Name we are looking for is for example a Namespace,
  // then it is a Decl Context.
  if ((*lookup_result.data())->getKind() == clang::Decl::Namespace) {

    clang::DeclContext* declContextFrom =
      llvm::cast<clang::DeclContext>(*lookup_result.data());

    clang::DeclContext* importedDeclContext =
      importer.ImportContext(declContextFrom);

    if(!importedDeclContext) {
      std::cerr << "Error: Could not import Decl Context: "
      << Name.getAsString()
      << std::endl;
    } else {
      std::cout << "Successfully imported the Decl Context: "
      << Name.getAsString()
      << std::endl;

      //And also put the declaration context I found from the first Interpreter
      //in the map of the second Interpreter to have it for the future.
      m_DeclContexts_map[importedDeclContext]= declContextFrom;
      m_DeclContextsNames_map[Name.getAsString()] =
        std::make_pair(importedDeclContext,declContextFrom);
      importedDeclContext->setHasExternalVisibleStorage(true);

        //clang::ASTContext *astContextP = &from_ASTContext;
        //GetCompleteDecl(astContextP, declContextFrom);

      std::vector<clang::NamedDecl*> declVector;
      declVector.push_back((clang::NamedDecl*)importedDeclContext);
      llvm::ArrayRef<clang::NamedDecl*> FoundDecls(declVector);
      //SetExternalVisibleDeclsForName(DC, Name, FoundDecls)
    }
  } else { //If this name is just a Decl.

    //Check if we have more than one results, this means that it may be
    //an overloaded function.
    if (lookup_result.size() > 1) {
      clang::DeclContext::lookup_iterator I;
      clang::DeclContext::lookup_iterator E;
      for (I = lookup_result.begin(), E = lookup_result.end(); I != E; ++I) {
        clang::NamedDecl *D = *I;
        clang::Decl *declFrom = llvm::cast<clang::Decl>(D);
        clang::Decl *importedDecl = importer.Import(declFrom);

        //TODO: overloaded template functions check

        if(!importedDecl) {
          std::cerr << "Error: Could not import the Decl :"
          << Name.getAsString()
          << std::endl;
        } else {
          std::cout << "Successfully imported the Decl : "
          << Name.getAsString()
          << std::endl;

          //Import this decl in the map we own.
          m_Decls_map[Name.getAsString()] = std::make_pair(importedDecl, declFrom);
          clang::ASTContext *astContextP = &from_ASTContext;
          //GetCompleteDecl(astContextP, declFrom);

          std::vector<clang::NamedDecl*> declVector;
          declVector.push_back((clang::NamedDecl*)importedDecl);
          llvm::ArrayRef<clang::NamedDecl*> FoundDecls(declVector);
          SetExternalVisibleDeclsForName(DC, Name, FoundDecls);
        }
      }
    } else {
      clang::Decl *declFrom = llvm::cast<clang::Decl>(*lookup_result.data());
      clang::Decl *importedDecl = nullptr;
      if (!(declFrom->isFunctionOrFunctionTemplate() && declFrom->isTemplateDecl())) {
        //Don't do the import if we have a Function Template
        importedDecl = importer.Import(declFrom);
      }

      if (!importedDecl) {
        std::cerr << "Error: Could not import the Decl :"
        << Name.getAsString()
        << std::endl;
      } else {
        std::cout << "Successfully imported the Decl : "
        << Name.getAsString()
        << std::endl;

        //Import this decl in the map we own.
        m_Decls_map[Name.getAsString()] = std::make_pair(importedDecl, declFrom);
        clang::ASTContext *astContextP = &from_ASTContext;
        //GetCompleteDecl(astContextP, declFrom);

        std::vector < clang::NamedDecl * > declVector;
        //clang::NamedDecl *namedDeclResult = *lookup_result.data();
        declVector.push_back((clang::NamedDecl *) importedDecl);
        llvm::ArrayRef < clang::NamedDecl * > FoundDecls(declVector);

        SetExternalVisibleDeclsForName(DC, Name, FoundDecls);
      }
    }
  }
    //clang::Decl* imported= importer.Imported(declFrom,importedDecl);
    //importer.ImportDefinition(declFrom);

    /***************************************************/
    // importer.Import((*lookup_result.data())->getDeclName());
    //std::vector<clang::NamedDecl*> declVector;
    //
    // declVector.push_back(dynamic_cast<clang::NamedDecl*>(importedDecl));
    //
    // clang::DeclContext::lookup_result setExternalResult =
    /***************************************************/
  return true;
}

bool ASTImportSource::FindExternalVisibleDeclsByName(const clang::DeclContext *DC,
                                                      clang::DeclarationName Name) {

  assert(DC->hasExternalVisibleStorage() &&
           "DeclContext has no visible decls in storage");

  llvm::StringRef name(Name.getAsString());

  std::cout << "About to search for the name: "
  << Name.getAsString()
  << std::endl;

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

  if (m_DeclContextsNames_map.find(Name.getAsString()) != m_DeclContextsNames_map.end()) {
    std::cout << "We have already imported this Decl (Context) before!"
    << std::endl;
    return true;
  } else if (m_Decls_map.find(Name.getAsString()) != m_Decls_map.end()) {
    std::cout << "We have already imported this Decl before!"
    << std::endl;
    return true;
  }

  clang::DeclContext::lookup_result lookup_result;
  //If we are not looking for this Name in the Translation Unit
  //but instead inside a namespace
  if (!DC->isTranslationUnit()){

    //Search in the map of the stored Decl Contexts for this
    //namespace
    if (m_DeclContexts_map.find(DC) != m_DeclContexts_map.end()){
      //If DC was found before and is already in the map,
      //then do the lookup using the stored pointer
      clang::DeclContext* originalDeclContext = m_DeclContexts_map.find(DC)->second;

      clang::Decl *fromDeclContext = clang::Decl::castFromDeclContext(originalDeclContext);
      clang::ASTContext &from_ASTContext = fromDeclContext->getASTContext();

      clang::Decl *toDeclContext = clang::Decl::castFromDeclContext(DC);
      clang::ASTContext &to_ASTContext = toDeclContext->getASTContext();

      lookup_result = originalDeclContext->lookup(declNameI1);
      //Do the import
      if (Import(lookup_result, from_ASTContext, to_ASTContext, DC, Name)) {
        //And also put the Decl I found from the first Interpreter
        //in the map of the second Interpreter to have it for the future.
        m_DeclName_map[Name.getAsString()] = declNameI1;
        return true;
      }
    }
  } else { // Otherwise search in the Translation Unit
    std::cout << "About to search in the Translation Unit of I1 for:"
    << declNameI1.getAsString()
    << std::endl;

    clang::ASTContext &from_ASTContext = m_translationUnitI1->getASTContext();
    clang::ASTContext &to_ASTContext = m_translationUnitI2->getASTContext();

    lookup_result = m_TUDeclContextI1->lookup(declNameI1);
    //Do the import
    if (Import(lookup_result, from_ASTContext, to_ASTContext, DC, Name)) {
      //And also put the Decl I found from the first Interpreter
      //in the map of the second Interpreter to have it for the future.
      m_DeclName_map[Name.getAsString()] = declNameI1;
      return true;
    }
  }

  return false;
}
