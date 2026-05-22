#include <iostream>
#include <string>
#include <cassert>
#include <cmath>

struct Transformer;
struct Number;
struct BinaryOperation;
struct FunctionCall;
struct Variable;

struct Expression {
    virtual ~Expression() {}
    virtual double evaluate() const = 0;
    virtual Expression* transform(Transformer* tr) const = 0;
};

struct Transformer {
    virtual ~Transformer() {}
    virtual Expression* transformNumber(Number const*) = 0;
    virtual Expression* transformBinaryOperation(BinaryOperation const*) = 0;
    virtual Expression* transformFunctionCall(FunctionCall const*) = 0;
    virtual Expression* transformVariable(Variable const*) = 0;
};

struct Number : Expression {
    Number(double value) : value_(value) {}
    double value() const { return value_; }
    double evaluate() const override { return value_; }
    Expression* transform(Transformer* tr) const override {
        return tr->transformNumber(this);
    }
private:
    double value_;
};

struct BinaryOperation : Expression {
    enum { PLUS = '+', MINUS = '-', DIV = '/', MUL = '*' };
    BinaryOperation(Expression const* left, int op, Expression const* right)
        : left_(left), right_(right), op_(op) {
        assert(left_ && right_);
    }
    ~BinaryOperation() {
        delete left_;
        delete right_;
    }
    double evaluate() const override {
        double l = left_->evaluate();
        double r = right_->evaluate();
        switch (op_) {
        case PLUS: return l + r;
        case MINUS: return l - r;
        case DIV: return l / r;
        case MUL: return l * r;
        default: return 0.0;
        }
    }
    Expression* transform(Transformer* tr) const override {
        return tr->transformBinaryOperation(this);
    }
    Expression const* left() const { return left_; }
    Expression const* right() const { return right_; }
    int operation() const { return op_; }
private:
    Expression const* left_;
    Expression const* right_;
    int op_;
};

struct FunctionCall : Expression {
    FunctionCall(std::string const& name, Expression const* arg)
        : name_(name), arg_(arg) {
        assert(arg_);
        assert(name_ == "sqrt" || name_ == "abs");
    }
    ~FunctionCall() {
        delete arg_;
    }
    double evaluate() const override {
        double a = arg_->evaluate();
        if (name_ == "sqrt") return sqrt(a);
        else return fabs(a);
    }
    Expression* transform(Transformer* tr) const override {
        return tr->transformFunctionCall(this);
    }
    std::string const& name() const { return name_; }
    Expression const* arg() const { return arg_; }
private:
    std::string const name_;
    Expression const* arg_;
};

struct Variable : Expression {
    Variable(std::string const& name) : name_(name) {}
    std::string const& name() const { return name_; }
    double evaluate() const override { return 0.0; }
    Expression* transform(Transformer* tr) const override {
        return tr->transformVariable(this);
    }
private:
    std::string const name_;
};

struct CopySyntaxTree : Transformer {
    Expression* transformNumber(Number const* number) override {
        return new Number(number->value());
    }
    Expression* transformBinaryOperation(BinaryOperation const* binop) override {
        Expression* newLeft = binop->left()->transform(this);
        Expression* newRight = binop->right()->transform(this);
        return new BinaryOperation(newLeft, binop->operation(), newRight);
    }
    Expression* transformFunctionCall(FunctionCall const* fcall) override {
        Expression* newArg = fcall->arg()->transform(this);
        return new FunctionCall(fcall->name(), newArg);
    }
    Expression* transformVariable(Variable const* var) override {
        return new Variable(var->name());
    }
};

int main(){
    Number* n32 = new Number(32.0);
    Number* n16 = new Number(16.0);
    BinaryOperation* minus = new BinaryOperation(n32, BinaryOperation::MINUS, n16);
    FunctionCall* callSqrt = new FunctionCall("sqrt", minus);
    Variable* var = new Variable("var");
    BinaryOperation* mult = new BinaryOperation(var, BinaryOperation::MUL, callSqrt);
    FunctionCall* callAbs = new FunctionCall("abs", mult);

    CopySyntaxTree cst;
    Expression* newExpr = callAbs->transform(&cst);

    std::cout << "Original expression result: " << callAbs->evaluate() << std::endl;
    std::cout << "Copied expression result: " << newExpr->evaluate() << std::endl;

    delete newExpr;
    delete callAbs;
}