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

    clang::Sema *m_Sema;
    /* A map for the DeclContext-to-DeclContext correspondence
     * with DeclContexts pointers. */
    std::map<const clang::DeclContext*, clang::DeclContext*> m_DeclContexts_map;
    /* A map for all the imported Decls (Contexts). */
    std::map<std::string, clang::DeclarationName> m_DeclName_map;
    /* A map for the Names connected to the DeclContexts imported
    std::map<std::string, std::pair<clang::DeclContext*, clang::DeclContext*>>
      m_DeclContextsNames_map;
    */
    /* A map for the Names connected to the Decls imported
    std::map<std::string, std::pair<clang::Decl*, clang::Decl*>> m_Decls_map;
     */

  public:
    ASTImportSource(cling::Interpreter* interpreter_first,
                    cling::Interpreter* interpreter_second);
    ~ASTImportSource(){};
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

    void ImportDeclContext(clang::DeclContext *declContextFrom,
                           clang::ASTImporter &importer,
                           clang::DeclarationName &Name,
                           clang::DeclarationName &declNameI1,
                           const clang::DeclContext *DC);

    void ImportDecl(clang::Decl *declFrom,
                    clang::ASTImporter &importer,
                    clang::DeclarationName &Name,
                    clang::DeclarationName &declNameI1,
                    const clang::DeclContext *DC);

    //bool LookupUnqualified(clang::LookupResult &R, clang::Scope *S);
    cling::Interpreter* getInterpreter() { return m_first_Interp; }
};
#endif //CLING_ASTIMPORTSOURCE_H
