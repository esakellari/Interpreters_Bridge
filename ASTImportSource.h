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
//#include "cling/Interpreter/InterpreterCallbacks.h"
#include "cling/Interpreter/Value.h"
#include "cling/Interpreter/Interpreter.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Lex/Preprocessor.h"

#include "clang/Parse/RAIIObjectsForParser.h"
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

using  namespace std;

class ASTImportSource : /*public clang::ExternalASTSource,*/ public clang::ExternalSemaSource{

  private:
  cling::Interpreter *m_first_Interp;
  cling::Interpreter *m_second_Interp;
  clang::Sema *m_Sema;
  std::map<clang::NamedDecl*, clang::NamedDecl*> m_Decl_map;
  int aa;
  public:
    ASTImportSource(cling::Interpreter* interpreter_first,
                    cling::Interpreter* interpreter_second)
      {
        m_first_Interp = interpreter_first;
        m_second_Interp = interpreter_second;
      }
    ~ASTImportSource();
    bool
    FindExternalVisibleDeclsByName(const clang::DeclContext *DC, clang::DeclarationName Name) override;
    void InitializeSema(clang::Sema& S) override;

    bool LookupUnqualified(clang::LookupResult &R, clang::Scope *S) override;
    cling::Interpreter* getInterpreter() { return m_first_Interp; }
};

#endif //LLVM_ASTIMPORTSOURCE_H
