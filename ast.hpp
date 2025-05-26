#ifndef AST_HPP
#define AST_HPP

#include <variant>
#include <string>
#include <memory>

struct IntVarStmt {
    std::string m_name{};
    BinOpExpr m_value{};
};

struct Stmt {
    std::variant<IntVarStmt> m_stmt{};
};

struct Expr;

struct BinOpExpr {
    std::shared_ptr<Expr> m_lhs{};
    std::shared_ptr<Expr> m_rhs{};
    Token_Type m_op{};
};

struct Expr {
    std::variant<BinOpExpr> m_expr{};
};

struct Program {
    std::vector<Stmt> m_body{};
};

#endif
