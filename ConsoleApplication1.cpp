#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <cmath>

struct Transformer;
struct Number;
struct BinaryOperation;
struct FunctionCall;
struct Variable;
struct AquariumObject;
struct Barbus;
struct Carp;
struct Catfish;
struct BigSnail;
struct SmallSnail;
struct Waterweed;
struct Fish;
struct sinal;
struct Water;

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
    ~FunctionCall() { delete arg_; }
    double evaluate() const override {
        double a = arg_->evaluate();
        if (name_ == "sqrt") return std::sqrt(a);
        else return std::fabs(a);
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

struct FoldConstants : Transformer {
    Expression* transformNumber(Number const* number) override {
        return new Number(number->value());
    }
    Expression* transformBinaryOperation(BinaryOperation const* binop) override {
        Expression* left = binop->left()->transform(this);
        Expression* right = binop->right()->transform(this);
        Number* leftNum = dynamic_cast<Number*>(left);
        Number* rightNum = dynamic_cast<Number*>(right);
        if (leftNum && rightNum) {
            double res;
            switch (binop->operation()) {
            case BinaryOperation::PLUS: res = leftNum->value() + rightNum->value(); break;
            case BinaryOperation::MINUS: res = leftNum->value() - rightNum->value(); break;
            case BinaryOperation::MUL: res = leftNum->value() * rightNum->value(); break;
            case BinaryOperation::DIV: res = leftNum->value() / rightNum->value(); break;
            default: res = 0.0;
            }
            delete left;
            delete right;
            return new Number(res);
        }
        else {
            return new BinaryOperation(left, binop->operation(), right);
        }
    }
    Expression* transformFunctionCall(FunctionCall const* fcall) override {
        Expression* arg = fcall->arg()->transform(this);
        Number* argNum = dynamic_cast<Number*>(arg);
        if (argNum) {
            double val;
            if (fcall->name() == "sqrt") val = std::sqrt(argNum->value());
            else val = std::fabs(argNum->value());
            delete arg;
            return new Number(val);
        }
        else {
            return new FunctionCall(fcall->name(), arg);
        }
    }
    Expression* transformVariable(Variable const* var) override {
        return new Variable(var->name());
    }
};


struct AquariumVisitor {
    virtual ~AquariumVisitor() {}
    virtual void visit(Barbus* obj) = 0;
    virtual void visit(Carp* obj) = 0;
    virtual void visit(Catfish* obj) = 0;
    virtual void visit(BigSnail* obj) = 0;
    virtual void visit(SmallSnail* obj) = 0;
    virtual void visit(Waterweed* obj) = 0;
};

struct AquariumObject {
    virtual ~AquariumObject() {}
    virtual void accept(AquariumVisitor& visitor) = 0;

    virtual std::string name() const = 0;
};
struct Fish : AquariumObject {
    virtual ~Fish() {}
     void accept(AquariumVisitor& visitor) = 0;
     std::string name() const = 0;
};
struct Barbus : Fish {
    void accept(AquariumVisitor& visitor) override { visitor.visit(this); }
    std::string name() const override { return "Barbus"; }
};
struct Carp : Fish {
    void accept(AquariumVisitor& visitor) override { visitor.visit(this); }
    std::string name() const override { return "Carp"; }
};
struct Catfish : Fish {
    void accept(AquariumVisitor& visitor) override { visitor.visit(this); }
    std::string name() const override { return "Catfish"; }
};

struct sinal : AquariumObject {
    virtual ~sinal() {}
    void accept(AquariumVisitor& visitor) = 0;
     std::string name() const = 0;
};
struct BigSnail : sinal {
    void accept(AquariumVisitor& visitor) override { visitor.visit(this); }
    std::string name() const override { return "Big Snail"; }
};
struct SmallSnail : sinal {
    
    void accept(AquariumVisitor& visitor) override { visitor.visit(this); }
    std::string name() const override { return "Small Snail"; }
};

struct Waterweed : AquariumObject {
    virtual ~Waterweed() {}
    void accept(AquariumVisitor& visitor) = 0;
     std::string name() const = 0;
};

struct Water : Waterweed {
    void accept(AquariumVisitor& visitor) override { visitor.visit(this); }
    std::string name() const override { return "Waterweed"; }
};

class Aquarium {
    std::vector<AquariumObject*> inhabitants;
public:
    ~Aquarium() {
        for (auto obj : inhabitants) delete obj;
    }
    void add(AquariumObject* obj) {
        inhabitants.push_back(obj);
    }
    void accept(AquariumVisitor& visitor) {
        for (auto obj : inhabitants) obj->accept(visitor);
    }
};

struct CountVisitor : AquariumVisitor {
    int barbus = 0, carp = 0, catfish = 0, bigSnail = 0, smallSnail = 0, waterweed = 0;
    void visit(Barbus*) override { ++barbus; }
    void visit(Carp*) override { ++carp; }
    void visit(Catfish*) override { ++catfish; }
    void visit(BigSnail*) override { ++bigSnail; }
    void visit(SmallSnail*) override { ++smallSnail; }
    void visit(Waterweed*) override { ++waterweed; }
    void print() const {
        std::cout << "Aquarium composition:\n"
            << "Barbus: " << barbus << "\n"
            << "Carp: " << carp << "\n"
            << "Catfish: " << catfish << "\n"
            << "Big Snail: " << bigSnail << "\n"
            << "Small Snail: " << smallSnail << "\n"
            << "Waterweed: " << waterweed << "\n"
            << "Total: " << (barbus + carp + catfish + bigSnail + smallSnail + waterweed) << "\n";
    }
};

struct NamePrinter : AquariumVisitor {
    void visit(Barbus* obj) override { std::cout << obj->name() << "\n"; }
    void visit(Carp* obj) override { std::cout << obj->name() << "\n"; }
    void visit(Catfish* obj) override { std::cout << obj->name() << "\n"; }
    void visit(BigSnail* obj) override { std::cout << obj->name() << "\n"; }
    void visit(SmallSnail* obj) override { std::cout << obj->name() << "\n"; }
    void visit(Waterweed* obj) override { std::cout << obj->name() << "\n"; }
};

int main() {
    setlocale(LC_ALL, "Russian");
    std::cout << "Задание 1:\n";
    {
        Number* n32 = new Number(32.0);
        Number* n16 = new Number(16.0);
        BinaryOperation* minus = new BinaryOperation(n32, BinaryOperation::MINUS, n16);
        FunctionCall* callSqrt = new FunctionCall("sqrt", minus);
        Variable* var = new Variable("var");
        BinaryOperation* mult = new BinaryOperation(var, BinaryOperation::MUL, callSqrt);
        FunctionCall* callAbs = new FunctionCall("abs", mult);

        CopySyntaxTree cst;
        Expression* newExpr = callAbs->transform(&cst);

        std::cout << "Original result: " << callAbs->evaluate() << std::endl;
        std::cout << "Copied result: " << newExpr->evaluate() << std::endl;

        delete newExpr;
        delete callAbs;
    }

    std::cout << "\nЗадание 2:\n";
    {
        Number* n32 = new Number(32.0);
        Number* n16 = new Number(16.0);
        BinaryOperation* minus = new BinaryOperation(n32, BinaryOperation::MINUS, n16);
        FunctionCall* callSqrt = new FunctionCall("sqrt", minus);
        Variable* var = new Variable("var");
        BinaryOperation* mult = new BinaryOperation(var, BinaryOperation::MUL, callSqrt);
        FunctionCall* callAbs = new FunctionCall("abs", mult);

        FoldConstants fc;
        Expression* folded = callAbs->transform(&fc);

        std::cout << "Original: " << callAbs->evaluate() << std::endl;
        std::cout << "Folded:   " << folded->evaluate() << std::endl;

        delete folded;
        delete callAbs;
    }

    std::cout << "\n Задание 3 (Вариант 16):\n";
    {
        Aquarium aquarium;
        aquarium.add(new Barbus());
        aquarium.add(new Barbus());
        aquarium.add(new Carp());
        aquarium.add(new Catfish());
        aquarium.add(new BigSnail());
        aquarium.add(new SmallSnail());
        aquarium.add(new SmallSnail());
        aquarium.add(new Water());

        CountVisitor counter;
        aquarium.accept(counter);
        counter.print();

        std::cout << "\nList of inhabitants:\n";
        NamePrinter printer;
        aquarium.accept(printer);
    }

    return 0;
}