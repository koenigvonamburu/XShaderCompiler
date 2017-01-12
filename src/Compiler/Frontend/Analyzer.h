/*
 * Analyzer.h
 * 
 * This file is part of the XShaderCompiler project (Copyright (c) 2014-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef XSC_ANALYZER_H
#define XSC_ANALYZER_H


#include <Xsc/Xsc.h>
#include "ReportHandler.h"
#include "Visitor.h"
#include "Variant.h"
#include "Token.h"
#include "SymbolTable.h"
#include "AST.h"
#include <string>
#include <stack>


namespace Xsc
{


struct StructTypeDenoter;

// Context analyzer base class.
class Analyzer : protected Visitor
{
    
    public:
        
        Analyzer(Log* log = nullptr);

        bool DecorateAST(
            Program& program,
            const ShaderInput& inputDesc,
            const ShaderOutput& outputDesc
        );

    protected:
        
        using OnOverrideProc = ASTSymbolTable::OnOverrideProc;

        virtual void DecorateASTPrimary(
            Program& program,
            const ShaderInput& inputDesc,
            const ShaderOutput& outputDesc
        ) = 0;

        /* ----- Report and error handling ----- */

        void SubmitReport(bool isError, const std::string& msg, const AST* ast = nullptr, const HLSLErr errorCode = HLSLErr::Unknown);
        
        void Error(const std::string& msg, const AST* ast = nullptr, const HLSLErr errorCode = HLSLErr::Unknown);
        void ErrorUndeclaredIdent(const std::string& ident, const AST* ast = nullptr);
        void ErrorUndeclaredIdent(const std::string& ident, const std::string& contextName, const AST* ast = nullptr);
        void ErrorInternal(const std::string& msg, const AST* ast = nullptr);

        void Warning(const std::string& msg, const AST* ast = nullptr);
        void WarningOnNullStmnt(const StmntPtr& ast, const std::string& stmntTypeName);

        // Returns the report handler.
        inline ReportHandler& GetReportHandler()
        {
            return reportHandler_;
        }

        /* ----- Symbol table functions ----- */

        void OpenScope();
        void CloseScope();

        void Register(const std::string& ident, AST* ast);
        
        AST* Fetch(const std::string& ident, const AST* ast = nullptr);
        AST* Fetch(const VarIdentPtr& ident);

        AST* FetchType(const std::string& ident, const AST* ast = nullptr);

        VarDecl* FetchVarDecl(const std::string& ident, const AST* ast = nullptr);

        FunctionDecl* FetchFunctionDecl(const std::string& ident, const std::vector<ExprPtr>& args, const AST* ast = nullptr);
        FunctionDecl* FetchFunctionDecl(const std::string& ident, const AST* ast = nullptr);

        VarDecl* FetchFromStructDecl(const StructTypeDenoter& structTypeDenoter, const std::string& ident, const AST* ast = nullptr);

        StructDecl* FetchStructDeclFromIdent(const std::string& ident, const AST* ast = nullptr);
        StructDecl* FetchStructDeclFromTypeDenoter(const TypeDenoter& typeDenoter);

        /* ----- Function declaration tracker ----- */

        void PushFunctionDeclLevel(FunctionDecl* ast);
        void PopFunctionDeclLevel();

        // Returns true if the analyzer is currently inside a function declaration.
        bool InsideFunctionDecl() const;

        // Returns true if the analyzer is currently inside the main entry point.
        bool InsideEntryPoint() const;

        // Returns the active (inner most) function declaration or null if the analyzer is currently not inside a function declaration.
        FunctionDecl* ActiveFunctionDecl() const;

        /* ----- Structure declaration tracker ----- */

        void PushStructDecl(StructDecl* ast);
        void PopStructDecl();

        // Returns true if the analyzer is currently inside a structure declaration.
        bool InsideStructDecl() const;

        // Returns the stack (or rather the list) of all current, nested structure declarations.
        inline const std::vector<StructDecl*>& GetStructDeclStack() const
        {
            return structDeclStack_;
        }

        /* ----- Function call tracker ----- */

        void PushFunctionCall(FunctionCall* ast);
        void PopFunctionCall();

        // Returns the active (inner most) function call or null if the analyzer is currently not inside a function call.
        FunctionCall* ActiveFunctionCall() const;

        /* ----- Analyzer functions ----- */

        void AnalyzeTypeDenoter(TypeDenoterPtr& typeDenoter, const AST* ast);
        void AnalyzeBufferTypeDenoter(BufferTypeDenoter& bufferTypeDen, const AST* ast);
        void AnalyzeStructTypeDenoter(StructTypeDenoter& structTypeDen, const AST* ast);
        void AnalyzeAliasTypeDenoter(TypeDenoterPtr& typeDenoter, const AST* ast);

        TypeDenoterPtr GetTypeDenoterFrom(TypedAST* ast);

        void ValidateTypeCast(const TypeDenoter& sourceTypeDen, const TypeDenoter& destTypeDen, const std::string& contextDesc, const AST* ast = nullptr);
        void ValidateTypeCastFrom(TypedAST* sourceAST, TypedAST* destAST, const std::string& contextDesc);

        /* ----- Const-expression evaluation ----- */

        // Evaluates the specified constant expression.
        Variant EvaluateConstExpr(Expr& expr);

        // Evaluates the specified constant variable access expression or throws the expression if it's not constant.
        Variant EvaluateConstVarAccessdExpr(VarAccessExpr& expr);

        // Evaluates the specified constant integer expression.
        int EvaluateConstExprInt(Expr& expr);

        // Evaluates the specified constant floating-point expression.
        float EvaluateConstExprFloat(Expr& expr);

    private:

        /* === Members === */

        ReportHandler               reportHandler_;
        SourceCode*                 sourceCode_                 = nullptr;

        ASTSymbolOverloadTable      symTable_;

        // Current level of function declarations. Actually only 0 or 1 (but can be more if inner functions are supported).
        std::size_t                 funcDeclLevelOfEntryPoint_  = ~0;

        // Function declaration stack.
        std::stack<FunctionDecl*>   funcDeclStack_;

        // Structure stack to collect all members with system value semantic (SV_...), and detect all nested structures.
        std::vector<StructDecl*>    structDeclStack_;

        // Function call stack to join arguments with its function call.
        std::stack<FunctionCall*>   funcCallStack_;

};


} // /namespace Xsc


#endif



// ================================================================================