//
// Created by esakella on 7/8/15.
//

#include "ASTImportSource.h"

ASTImportSource::ASTImportSource() { }
ASTImportSource::~ASTImportSource() { }

bool ASTImportSource::FindExternalVisibleDeclsByName(
  const clang::DeclContext *DC, clang::DeclarationName Name){

  cout << "ASTImportSource::FindExternalVisibleDeclsByName" <<endl;
/*  clang::DeclContext::decl_range decl_range = DC->decls();
 //clang::Decl::Kind kind1;
  //clang::Decl::Kind kind_embed;
  for (const auto *I : decl_range ) {
    if (const auto *IFD = llvm::dyn_cast<clang::Decl>(I)) {
      if (IFD->getKind() == clang::Decl::Namespace){
         //IFD->dump();
        const auto *IFFD = llvm::dyn_cast<clang::NamedDecl>(IFD);
        cout<< IFFD->getQualifiedNameAsString ()<<endl;
        //do the same for the embedded namespace
        //recursively
        this->FindExternalVisibleDeclsByName(llvm::dyn_cast<clang::DeclContext>(IFD), Name);

      }else if(IFD->getKind() == clang::Decl::Var){
        const auto *IFFD = llvm::dyn_cast<clang::NamedDecl>(IFD);
        cout<< IFFD->getQualifiedNameAsString()<<endl;
        if(IFFD->getQualifiedNameAsString()=="mynamespace::a"){
          cout << "Found" <<endl;
          return true;
        }
      }
    }

  }*/
  return false;
}

