#ifndef CLING_ASTIMPORTSOURCE_H
#define CLING_ASTIMPORTSOURCE_H

#include <string>
#include <iostream>
#include <map>

#include "cling/Interpreter/Interpreter.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Sema/Sema.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTImporter.h"

class ASTImportSource : public clang::ExternalASTSource {

  private:
    cling::Interpreter *m_first_Interp;
    cling::Interpreter *m_second_Interp;
    const clang::TranslationUnitDecl *m_translationUnitI1;
    const clang::TranslationUnitDecl *m_translationUnitI2;
    clang::DeclContext * m_TUDeclContextI1;
    clang::Sema *m_Sema;
    std::map<const clang::DeclContext*, clang::DeclContext*> m_DeclContexts_map;
    std::map<std::string, clang::DeclarationName> m_DeclName_map;
    std::map<std::string, std::pair<clang::DeclContext*, clang::DeclContext*>>
      m_DeclContextsNames_map;
    std::map<std::string, std::pair<clang::Decl*, clang::Decl*>> m_Decls_map;

  public:
    ASTImportSource(cling::Interpreter* interpreter_first,
                    cling::Interpreter* interpreter_second);
    ~ASTImportSource();
    bool
    FindExternalVisibleDeclsByName(const clang::DeclContext *DC, clang::DeclarationName Name);
    void InitializeSema(clang::Sema& S);
    void ForgetSema();

    bool Import(clang::DeclContext::lookup_result lookup_result,
                clang::ASTContext& from_ASTContext,
                clang::ASTContext& to_ASTContext,
                const clang::DeclContext *DC,
                clang::DeclarationName &Name,
                clang::DeclarationName &declNameI1);

    //bool LookupUnqualified(clang::LookupResult &R, clang::Scope *S);
    cling::Interpreter* getInterpreter() { return m_first_Interp; }

};

#endif //CLING_ASTIMPORTSOURCE_H
