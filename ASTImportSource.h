//
// Created by esakella on 7/8/15.
//
#ifndef LLVM_ASTIMPORTSOURCE_H
#define LLVM_ASTIMPORTSOURCE_H

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <map>
#include <chrono>
//#include "cling/Interpreter/InterpreterCallbacks.h"
#include "cling/Interpreter/Value.h"
#include "cling/Interpreter/Interpreter.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Lex/Preprocessor.h"

//#include "clang/Parse/RAIIObjectsForParser.h"
#include "clang/Sema/CodeCompleteConsumer.h"
#include "clang/Sema/CodeCompleteOptions.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/YAMLTraits.h"
#include "llvm/Support/Allocator.h"
#include "cling/Interpreter/Transaction.h"
#include "clang/Sema/Sema.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclarationName.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTImporter.h"

class ASTImportSource : public clang::ExternalASTSource/*, public clang::ExternalSemaSource*/{

  private:
    cling::Interpreter *m_first_Interp;
    cling::Interpreter *m_second_Interp;
    const clang::TranslationUnitDecl *m_translationUnitI1;
    const clang::TranslationUnitDecl *m_translationUnitI2;
    clang::DeclContext * m_TUDeclContextI1;
    clang::Sema *m_Sema;
    std::map<const clang::DeclContext*, clang::DeclContext*> m_DeclContexts_map;
    std::map<clang::Decl*, clang::Decl*> m_Decls_map;
    std::map<clang::DeclarationName, clang::DeclarationName> m_DeclName_map;

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
                clang::DeclarationName Name);
    bool LookupUnqualified(clang::LookupResult &R, clang::Scope *S);
    cling::Interpreter* getInterpreter() { return m_first_Interp; }

    bool GetCompleteDecl(clang::ASTContext *ast, clang::Decl *decl);
};

#endif //LLVM_ASTIMPORTSOURCE_H
