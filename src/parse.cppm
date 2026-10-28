module;
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <stdexcept>
#include <cctype>
#include <cmath>

export module cx.parse;
import cx.core;

export namespace cx {

struct Node;
using NodePtr = std::unique_ptr<Node>;

struct Const { double v; };
struct Var {};
struct Neg { NodePtr child; };
struct BinOp { char op; NodePtr lhs, rhs; };
struct Func { std::string name; NodePtr arg; };

struct Node {
    std::variant<Const, Var, Neg, BinOp, Func> inner;
};

inline Complex<> eval(const Node& n, Complex<> z) {
    return std::visit([&](auto&& x) -> Complex<> {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, Const>) return {x.v};
        else if constexpr (std::is_same_v<T, Var>) return z;
        else if constexpr (std::is_same_v<T, Neg>) return -eval(*x.child, z);
        else if constexpr (std::is_same_v<T, BinOp>) {
            auto l = eval(*x.lhs, z);
            auto r = eval(*x.rhs, z);
            switch (x.op) {
                case '+': return l + r;
                case '-': return l - r;
                case '*': return l * r;
                case '/': return l / r;
                case '^': return l.pow(r);
            }
            return Complex<>{};
        }
        else if constexpr (std::is_same_v<T, Func>) {
            auto a = eval(*x.arg, z);
            if (x.name == "exp") return a.exp();
            if (x.name == "log") return a.log();
            if (x.name == "sin") return a.sin();
            if (x.name == "cos") return a.cos();
            if (x.name == "sqrt") return a.pow(Complex<>{0.5, 0.0});
            throw std::runtime_error("unknown function: " + x.name);
        }
        return Complex<>{};
    }, n.inner);
}

class Parser {
    std::string_view src_;
    std::size_t pos_ = 0;

    void skip() {
        while (pos_ < src_.size() && std::isspace((unsigned char)src_[pos_])) ++pos_;
    }
    char peek() { skip(); return pos_ < src_.size() ? src_[pos_] : '\0'; }
    char advance() { skip(); return pos_ < src_.size() ? src_[pos_++] : '\0'; }

    static int prec_of(char c) {
        switch (c) {
            case '+': case '-': return 1;
            case '*': case '/': return 2;
            case '^': return 3;
            default: return 0;
        }
    }

    NodePtr parse_expr(int min_prec) {
        auto lhs = parse_atom();
        while (true) {
            char op = peek();
            int prec = prec_of(op);
            if (prec == 0 || prec < min_prec) break;
            advance();
            int next_min = (op == '^') ? prec : prec + 1;
            auto rhs = parse_expr(next_min);
            lhs = std::make_unique<Node>(Node{BinOp{op, std::move(lhs), std::move(rhs)}});
        }
        return lhs;
    }

    NodePtr parse_atom() {
        char c = peek();
        if (c == '(') {
            advance();
            auto e = parse_expr(0);
            if (advance() != ')') throw std::runtime_error("expected ')'");
            return e;
        }
        if (c == '-') {
            advance();
            return std::make_unique<Node>(Node{Neg{parse_atom()}});
        }
        if (std::isdigit((unsigned char)c) || c == '.') return parse_number();
        if (std::isalpha((unsigned char)c))             return parse_ident();
        throw std::runtime_error("unexpected char in expression");
    }

    NodePtr parse_number() {
        std::size_t start = pos_;
        while (pos_ < src_.size() &&
               (std::isdigit((unsigned char)src_[pos_]) || src_[pos_] == '.')) ++pos_;
        double v = std::stod(std::string(src_.substr(start, pos_ - start)));
        return std::make_unique<Node>(Node{Const{v}});
    }

    NodePtr parse_ident() {
        std::size_t start = pos_;
        while (pos_ < src_.size() && std::isalpha((unsigned char)src_[pos_])) ++pos_;
        std::string name(src_.substr(start, pos_ - start));
        if (name == "z") return std::make_unique<Node>(Node{Var{}});
        if (peek() == '(') {
            advance();
            auto arg = parse_expr(0);
            if (advance() != ')') throw std::runtime_error("expected ')'");
            return std::make_unique<Node>(Node{Func{name, std::move(arg)}});
        }
        throw std::runtime_error("unknown identifier: " + name);
    }

public:
    explicit Parser(std::string_view s) : src_(s) {}

    NodePtr parse() {
        auto e = parse_expr(0);
        skip();
        if (pos_ != src_.size()) throw std::runtime_error("trailing input");
        return e;
    }
};

inline NodePtr parse(std::string_view s) { return Parser(s).parse(); }

} // namespace cx
