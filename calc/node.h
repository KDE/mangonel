#ifndef ABAKUS_NODE_H
#define ABAKUS_NODE_H
/*
 * node.h - part of abakus
 * Copyright (C) 2004, 2005 Michael Pyne <michael.pyne@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <qstring.h>
#include <memory>

#include "numerictypes.h"

class Node;
struct Function;

//template <class T> class SharedPtr;
typedef std::shared_ptr<Node> NodePtr;

/**
 * A class that operates on a Node.  Called recursively on a node and all
 * of its children.
 */
class NodeFunctor
{
    public:
    virtual void operator()(const Node *node) = 0;
    virtual ~NodeFunctor() { }
};

class Node
{
    public:
    virtual ~Node() { }
    virtual Abakus::number_t value() const = 0;

    // Deletes a node only if it isn't a function, since those are
    // typically read-only.
    virtual void deleteNode(Node *node);

    // Calls functor() on all subchildren and this.
    virtual void applyMap(NodeFunctor &fn) const = 0;

    // Returns an infix representation of the expression.
    virtual QString infixString() const = 0;

    // Returns the derivative of the node.
    virtual Abakus::number_t derivative() const = 0;
};

class BaseFunction : public Node
{
    public:
    BaseFunction(const char *name);

    virtual QString name() const { return m_name; }
    virtual const Function *function() const;

    private:
    QString m_name;
};

class UnaryFunction : public BaseFunction
{
    public:
    UnaryFunction(const char *name, Node *operand);
    virtual ~UnaryFunction();

    virtual Node *operand() const { return m_node; }
    virtual void setOperand(Node *operand);

    virtual void applyMap(NodeFunctor &fn) const;
    virtual QString infixString() const;

    private:
    Node *m_node;
};

// Calculates the approximate derivative of its operand.
class DerivativeFunction : public Node
{
    public:
    DerivativeFunction(Node *operand, Node *where) : 
        m_operand(operand), m_where(where) { }
    ~DerivativeFunction();

    virtual Abakus::number_t value() const;

    virtual void applyMap(NodeFunctor &fn) const;

    // Returns an infix representation of the expression.
    virtual QString infixString() const;

    virtual Abakus::number_t derivative() const;

    private:
    Node *m_operand;
    Node *m_where;
};

class UserDefinedFunction : public UnaryFunction
{
    public:
    UserDefinedFunction(const char *name, Node *operand) : UnaryFunction(name, operand) { };

    virtual Abakus::number_t value() const { return operand()->value(); }
    virtual Abakus::number_t derivative() const { return operand()->derivative(); }
};

class BuiltinFunction : public UnaryFunction
{
    public:
    BuiltinFunction(const char *name, Node *operand);

    virtual Abakus::number_t value() const;
    virtual Abakus::number_t derivative() const;
};

class UnaryOperator : public Node
{
    public:
    typedef enum { Negation } Type;

    UnaryOperator(Type type, Node *operand);
    virtual ~UnaryOperator();

    virtual void applyMap(NodeFunctor &fn) const;
    virtual QString infixString() const;

    virtual Abakus::number_t value() const;
    virtual Abakus::number_t derivative() const;

    virtual Type type() const { return m_type; }
    virtual Node *operand() const { return m_node; }

    private:
    Type m_type;
    Node *m_node;
};

class BinaryOperator : public Node
{
    public:
    typedef enum { Addition, Subtraction, Multiplication, Division, Exponentiation } Type;

    BinaryOperator(Type type, Node *left, Node *right);
    virtual ~BinaryOperator();

    virtual void applyMap(NodeFunctor &fn) const;
    virtual QString infixString() const;

    virtual Abakus::number_t value() const;
    virtual Abakus::number_t derivative() const;

    virtual Type type() const       { return m_type; }
    virtual Node *leftNode() const  { return m_left; }
    virtual Node *rightNode() const { return m_right; }

    private:
    bool isSimpleNode(Node *node) const;

    Type m_type;
    Node *m_left, *m_right;
};

class Identifier : public Node
{
    public:
    Identifier(const char *name);

    virtual void applyMap(NodeFunctor &fn) const;
    virtual QString infixString() const { return name(); }

    virtual Abakus::number_t value() const;
    virtual Abakus::number_t derivative() const
    {
        if(name() == "x")
            return "1";
        else
            return "0";
    }

    virtual QString name() const { return m_name; }

    private:
    QString m_name;
};

class NumericValue : public Node
{
    public:
    NumericValue(const Abakus::number_t value) : m_value(value) { }

    virtual void applyMap(NodeFunctor &fn) const { fn(this); }
    virtual QString infixString() const;

    virtual Abakus::number_t value() const { return m_value; }
    virtual Abakus::number_t derivative() const { return "0"; }

    private:
    Abakus::number_t m_value;
};

#endif

// vim: set et sw=4 ts=8:
