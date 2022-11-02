#include <iostream>
#include <algorithm>
#include <gtk/gtk.h>
#include <cctype>
#include <math.h>
#include <vector>
#include <boost/algorithm/string_regex.hpp>
#define RESOLUTION 0.01
#define XMIN (-1.0)
#define XMAX (20.0)
#define XSCALE (XMAX - XMIN)
#define YMIN (-1.0)
#define YMAX (10.0)
#define YSCALE (YMAX - YMIN)
#define XGAP (50.0)
#define YGAP (50.0)
using namespace std;
using namespace boost;
extern "C" char _binary_res_window_glade_start;
extern "C" char _binary_res_window_glade_size;
bool isfunction(string expr, uint8_t* operands)
{
    *operands = (expr == "max" || expr == "min") ? 2 : 1;
    return expr == "max" || expr == "min" || expr == "sin" || expr == "cos" || expr == "tan"
        || expr == "atan" || expr == "acos" || expr == "asin" || expr == "log" || expr == "ln"
        || expr == "sqrt";
}
struct Token
{
    enum Type
    {
        LeftParentheses,
        RightParentheses,
        Expression,
        Operator,
        Function
    };
    uint8_t precedence, operands;
    bool left_align;
    Type type;
    string expr;
    Token(double d)
    {
        expr = to_string(d);
        type = Expression;
    }
    Token(string expr)
    {
        this->expr = expr;
        if (expr == ")")
        {
            type = RightParentheses;
        }
        else if (expr == "(")
        {
            type = LeftParentheses;
        }
        else if (expr == "+" || expr == "-" || expr == "*" || expr == "/" || expr == "^")
        {
            type = Operator;
            if (expr == "^")
            {
                precedence = 4;
                left_align = true;
            }
            if (expr == "*" || expr == "/")
            {
                precedence = 3;
                left_align = true;
            }
            if (expr == "+" || expr == "-")
            {
                precedence = 2;
                left_align = true;
            }
        }
        else if (isfunction(expr, &operands))
        {
            type = Function;
        }
        else
        {
            type = Expression;
        }
    }
    double value(double x)
    {
        if (expr == "e") return exp(1);
        if (expr.find('x') == -1) return stod(expr);
        if (expr == "x") return x;
        return stod(expr.substr(0, expr.find('x'))) * x;
    }
    double f(double x)
    {
        if (expr == "sin")
        {
            return sin(x);
        }
        if (expr == "cos")
        {
            return cos(x);
        }
        if (expr == "tan")
        {
            return tan(x);
        }
        if (expr == "asin")
        {
            return asin(x);
        }
        if (expr == "acos")
        {
            return acos(x);
        }
        if (expr == "atan")
        {
            return atan(x);
        }
        if (expr == "log")
        {
            return log10(x);
        }
        if (expr == "ln")
        {
            return log(x);
        }
        if (expr == "sqrt")
        {
            if (x < 0) throw x;
            return sqrt(x);
        }
        return -1;
    }
    double f(double x, double y)
    {
        if (expr == "+")
        {
            return x + y;
        }
        if (expr == "-")
        {
            return x - y;
        }
        if (expr == "*")
        {
            return x * y;
        }
        if (expr == "/")
        {
            return x / y;
        }
        if (expr == "^")
        {
            return pow(x, y);
        }
        if (expr == "max")
        {
            return max(x, y);
        }
        if (expr == "min")
        {
            return min(x, y);
        }
        return -1;
    }
    friend ostream& operator<<(ostream& out, Token token)
    {
        out << token.expr;
        return out;
    }
};
vector<Token> expressions_standard, expressions_rpn, op_stack;
double f(double x)
{
    vector<Token> exprs = expressions_rpn;
    for (size_t i = 0; i < exprs.size(); i++)
    {
        if (exprs[i].type == Token::Operator)
        {
            exprs[i - 2] = exprs[i].f(exprs[i - 2].value(x), exprs[i - 1].value(x));
            exprs.erase(exprs.begin() + i - 1, exprs.begin() + i + 1);
            i -= 2;
        }
        else if (exprs[i].type == Token::Function)
        {
            if (exprs[i].operands == 1)
            {
                exprs[i - 1] = exprs[i].f(exprs[i - 1].value(x));
                exprs.erase(exprs.begin() + i);
                i--;
            }
            else
            {
                exprs[i - 2] = exprs[i].f(exprs[i - 2].value(x), exprs[i - 2].value(x));
                exprs.erase(exprs.begin() + i - 1, exprs.begin() + i + 1);
                i -= 2;
            }
        }
    }
    return exprs[0].value(x);
}
void draw(GtkWidget* widget, cairo_t* cairo, void* data)
{
    cairo_set_line_width(cairo, 1);
    for (double i = XMIN; i < XMAX; i++)
    {
        cairo_set_source_rgba(cairo, 0, 0, 0, 0.25);
        cairo_move_to(cairo, i * XGAP, 0);
        cairo_line_to(cairo, i * XGAP, 500);
        cairo_stroke(cairo);
    }
    for (double i = YMIN; i < YMAX; i++)
    {
        cairo_set_source_rgba(cairo, 0, 0, 0, 0.25);
        cairo_move_to(cairo, 0, i * YGAP);
        cairo_line_to(cairo, 1000, i * YGAP);
        cairo_stroke(cairo);
    }
    if (!(XMIN < 0 && XMAX > 0)) goto skip1;
    cairo_set_line_width(cairo, 3);
    cairo_set_source_rgba(cairo, 0, 0, 0, 1);
    cairo_move_to(cairo, -(XMIN) * XGAP, 0);
    cairo_line_to(cairo, -(XMIN) * XGAP, 500);
    cairo_stroke(cairo);
skip1:
    if (!(YMIN < 0 && YMAX > 0)) goto skip2;
    cairo_set_line_width(cairo, 3);
    cairo_set_source_rgba(cairo, 0, 0, 0, 1);
    cairo_move_to(cairo, 0, 500 + (YMIN) * YGAP);
    cairo_line_to(cairo, 1000, 500 + (YMIN) * YGAP);
    cairo_stroke(cairo);
skip2:
    cairo_set_line_width(cairo, 1);
    for (double i = XMIN; i < XMAX; i += RESOLUTION)
    {
        try
        {
            double y1 = f(i), y2 = f(i + RESOLUTION);
            if (y1 >= YMAX || y1 < YMIN || y1 >= YMAX || y1 < YMIN) continue;
            cairo_set_source_rgb(cairo, 0, 0, 1);
            cairo_move_to(cairo, (i - XMIN) * XGAP, 500 - (y1 - YMIN) * YGAP);
            cairo_line_to(cairo, (i + RESOLUTION - XMIN) * XGAP, 500 - (y2 - YMIN) * YGAP);
            cairo_stroke(cairo);
        }
        catch (double err)
        {
        }
    }
}
regex r("(\\d+)|([a-z]+)|([\\*\\-\\+\\/\\^\\(\\)])");
int main(int argc, char const *argv[])
{
    gtk_init(&argc, (char***)&argv);
    cout << "Please enter an equation in f(x) form: ";
    string eq;
    getline(cin, eq);
    smatch matches;
    auto start = eq.begin();
    auto end = eq.end();
    match_results<string::iterator> what;
    while (regex_search(start, end, what, r))
    {
        expressions_standard.push_back(Token(string(what[0].first, what[0].second)));
        start = what[0].second;
    }
    for (size_t j = 0; j < expressions_standard.size(); j++)
    {
        auto token = expressions_standard[j];
        if (token.type == Token::Operator)
        {
            for (int i = op_stack.size() - 1; i >= 0; i--)
            {
                if (op_stack[i].type != Token::LeftParentheses 
                    && (op_stack[i].precedence > token.precedence || 
                    (op_stack[i].left_align == true && op_stack[i].precedence == token.precedence)))
                {
                    expressions_rpn.push_back(op_stack[i]);
                    op_stack.pop_back();
                }
                else break;
            }
            op_stack.push_back(token);
        }
        else if (j != 0)
        {
            auto lastToken = expressions_standard[j - 1];
            if (token.type == Token::Expression)
            {
                if (lastToken.type == Token::Expression)
                {
                    auto newToken = Token("*");
                    for (int i = op_stack.size() - 1; i >= 0; i--)
                    {
                        if (op_stack[i].type != Token::LeftParentheses 
                            && (op_stack[i].precedence > newToken.precedence || 
                            (op_stack[i].left_align == true 
                            && op_stack[i].precedence == newToken.precedence)))
                        {
                            expressions_rpn.push_back(op_stack[i]);
                            op_stack.pop_back();
                        }
                        else break;
                    }
                    op_stack.push_back(newToken);
                }
            }
            if (token.type == Token::LeftParentheses)
            {
                if (lastToken.type == Token::Expression
                    || lastToken.type == Token::RightParentheses)
                {
                    auto newToken = Token("*");
                    for (int i = op_stack.size() - 1; i >= 0; i--)
                    {
                        if (op_stack[i].type != Token::LeftParentheses 
                            && (op_stack[i].precedence > newToken.precedence || 
                            (op_stack[i].left_align == true 
                            && op_stack[i].precedence == newToken.precedence)))
                        {
                            expressions_rpn.push_back(op_stack[i]);
                            op_stack.pop_back();
                        }
                        else break;
                    }
                    op_stack.push_back(newToken);
                }
            }
        }
        if (token.type == Token::Expression)
        {
            expressions_rpn.push_back(token);
        }
        if (token.type == Token::LeftParentheses)
        {
            op_stack.push_back(token);
        }
        if (token.type == Token::RightParentheses)
        {
            for (int i = op_stack.size() - 1; i >= 0; i--)
            {
                if (op_stack[i].type != Token::LeftParentheses)
                {
                    expressions_rpn.push_back(op_stack[i]);
                    op_stack.pop_back();
                }
                else
                {
                    op_stack.pop_back();
                    break;
                }
            }
            if (op_stack.size() > 0 && op_stack[op_stack.size() - 1].type == Token::Function)
            {
                expressions_rpn.push_back(op_stack[op_stack.size() - 1]);
                op_stack.pop_back();
            }
        }
        if (token.type == Token::Function)
        {
            op_stack.push_back(token);
        }
    }
    for (int i = op_stack.size() - 1; i >= 0; i--)
    {
        expressions_rpn.push_back(op_stack[i]);
        op_stack.pop_back();
    }
    GtkBuilder* builder = gtk_builder_new();
    gtk_builder_add_from_string(builder, &_binary_res_window_glade_start, 
        (size_t)&_binary_res_window_glade_size, NULL);
    GtkWidget* window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    GtkWidget* canvas = GTK_WIDGET(gtk_builder_get_object(builder, "canvas"));
    g_signal_connect(GTK_WINDOW(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(GTK_DRAWING_AREA(canvas), "draw", G_CALLBACK(draw), NULL);
    gtk_widget_show_all(window);
    gtk_widget_queue_draw(canvas);
    gtk_main();
    return 0;
}