#include "oclint/AbstractASTVisitorRule.h"
#include "oclint/RuleSet.h"
#include "clang/Lex/Lexer.h"
    
using namespace std;
using namespace clang;
using namespace oclint;

class SameArgumentForFuncCallRule : public AbstractASTVisitorRule<SameArgumentForFuncCallRule>
{
public:
    virtual const string name() const override
    {
        return "same argument for func call";
    }

    virtual int priority() const override
    {
        return 3;
    }

    virtual const string category() const override
    {
        return "custom";
    }

#ifdef DOCGEN
    virtual const std::string since() const override
    {
        return "0.12";
    }

    virtual const std::string description() const override
    {
        return ""; // TODO: fill in the description of the rule.
    }

    virtual const std::string example() const override
    {
        return R"rst(
.. code-block:: cpp

    void example()
    {
        // TODO: modify the example for this rule.
    }
        )rst";
    }

    /* fill in the file name only when it does not match the rule identifier
    virtual const std::string fileName() const override
    {
        return "";
    }
    */

    /* add each threshold's key, description, and its default value to the map.
    virtual const std::map<std::string, std::string> thresholds() const override
    {
        std::map<std::string, std::string> thresholdMapping;
        return thresholdMapping;
    }
    */

    /* add additional document for the rule, like references, notes, etc.
    virtual const std::string additionalDocument() const override
    {
        return "";
    }
    */

    /* uncomment this method when the rule is enabled to be suppressed.
    virtual bool enableSuppress() const override
    {
        return true;
    }
    */
#endif

    virtual void setUp() override {
        sm = &_carrier->getSourceManager();
    }
    virtual void tearDown() override {}

    bool isLiteral(Expr* expr){
        if(isa<ImplicitCastExpr>(expr)){
            ImplicitCastExpr* ice = dyn_cast_or_null<ImplicitCastExpr>(expr);
            expr = ice->getSubExpr();
        }
        return isa<IntegerLiteral>(expr) || isa<CharacterLiteral>(expr) || isa<FloatingLiteral>(expr) || isa<StringLiteral>(expr);
    }
    /* Visit CallExpr */
    bool VisitCallExpr(CallExpr *callExpr)
    {
        unsigned num = callExpr->getNumArgs();
        FunctionDecl* fd = callExpr->getDirectCallee();
        if(num<2 || !fd)return true;

        string funName = fd->getNameInfo().getAsString();    
        // 重造+/-/*/符号
        if(funName.find("operator")!=string::npos)return true;

        Expr* arg1 = callExpr->getArg(0);
        Expr* arg2 = callExpr->getArg(1);

        if(!isLiteral(arg1) && !isLiteral(arg2)){
            string arg1Str = expr2str(arg1);
            string arg2Str = expr2str(arg2);
        
            if(arg1Str.size() && arg1Str==arg2Str){
                string funcName = callExpr->getDirectCallee()->getNameInfo().getAsString();
                string message = "The first argument of '"+funcName+"' function is equal to the second argument.";
                addViolation(callExpr, this, message);
                
            }
            
        }
        return true;
    }
      
private:
    std::string expr2str(Expr* expr) {
        // (T, U) => "T,,"
        string text = clang::Lexer::getSourceText(
            CharSourceRange::getTokenRange(expr->getSourceRange()), *sm, LangOptions(), 0);
        if (text.size()>0 && text.at(text.size()-1) == ',')
            return clang::Lexer::getSourceText(CharSourceRange::getCharRange(expr->getSourceRange()), *sm, LangOptions(), 0);
        return text;
    }
private:
    SourceManager *sm;
     
};

static RuleSet rules(new SameArgumentForFuncCallRule());
