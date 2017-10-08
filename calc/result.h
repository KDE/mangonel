#ifndef ABAKUS_RESULT_H
#define ABAKUS_RESULT_H
/*
 * result.h - part of abakus
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

#include "node.h"

/**
 * A conceptual result from an expression parse.  Used to determine if the
 * parse succeeded or failed.  If it succeeded it will have a node value you
 * can query as the answer.
 */
class Result
{
public:
    typedef enum { Error, Null, Value } Type;

    /**
     * Default constructor, which constructs a "failed" Result.
     */
    Result(const QString &message = "");

    /**
     * Node constructor, which constructs a "succeeded" result.
     */
    Result(NodePtr node);

    /**
     * Constructor, constructs a "null" result.  This means that the
     * operation was successful, but did not result in a normal value.
     */
    Result(Type type);

    bool failed() const { return m_type == Error; }

    Type type() const { return m_type; }

    QString message() const { return m_message; }

    const NodePtr result() const { return m_node; }
    NodePtr result() { return m_node; }

    static Result *lastResult() { return m_lastResult; }
    static void setLastResult(const Result &result)
    {
        *m_lastResult = result;
    }

private:
    NodePtr m_node;
    Type m_type;
    QString m_message;
    static Result *m_lastResult;
};

#endif /* ABAKUS_RESULT_H */

// vim: set et ts=8 sw=4:
